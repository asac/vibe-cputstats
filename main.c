// cpustats: periodically log CPU utilisation, load, frequency and temperature to stdout
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAXCPU 64

struct cpu { unsigned long long total, idle; };

// reads aggregate (index 0) and per-core (1..n) counters; returns number of cores
static int read_stat(struct cpu *c)
{
	FILE *f = fopen("/proc/stat", "r");
	char line[512];
	int n = -1;

	if (!f)
		return -1;
	while (fgets(line, sizeof(line), f) && n < MAXCPU) {
		unsigned long long v[8] = { 0 };
		char name[16];

		if (strncmp(line, "cpu", 3))
			break;
		if (sscanf(line, "%15s %llu %llu %llu %llu %llu %llu %llu %llu", name,
			   &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7]) < 5)
			continue;
		n++;
		c[n].total = 0;
		for (int i = 0; i < 8; i++)
			c[n].total += v[i];
		c[n].idle = v[3] + v[4]; // idle + iowait
	}
	fclose(f);
	return n;
}

static double pct(const struct cpu *a, const struct cpu *b)
{
	unsigned long long dt = b->total - a->total, di = b->idle - a->idle;
	return dt ? 100.0 * (double)(dt - di) / (double)dt : 0.0;
}

static long read_long(const char *path)
{
	FILE *f = fopen(path, "r");
	long v = -1;

	if (!f)
		return -1;
	if (fscanf(f, "%ld", &v) != 1)
		v = -1;
	fclose(f);
	return v;
}

int main(void)
{
	struct cpu prev[MAXCPU + 1], cur[MAXCPU + 1];
	const char *e = getenv("CPUSTATS_INTERVAL");
	int interval = e ? atoi(e) : 5;
	int n;

	if (interval < 1)
		interval = 5;
	setvbuf(stdout, NULL, _IOLBF, 0);

	n = read_stat(prev);
	if (n < 0) {
		fprintf(stderr, "cpustats: cannot read /proc/stat\n");
		return 1;
	}
	printf("cpustats: started, %d cores, interval %ds\n", n, interval);

	for (;;) {
		char ts[32], load[64] = "?";
		time_t now;
		long temp, freq;
		FILE *f;

		sleep(interval);
		n = read_stat(cur);
		if (n < 0)
			continue;

		now = time(NULL);
		strftime(ts, sizeof(ts), "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));

		if ((f = fopen("/proc/loadavg", "r"))) {
			double l1, l5, l15;
			if (fscanf(f, "%lf %lf %lf", &l1, &l5, &l15) == 3)
				snprintf(load, sizeof(load), "%.2f/%.2f/%.2f", l1, l5, l15);
			fclose(f);
		}
		temp = read_long("/sys/class/thermal/thermal_zone0/temp");
		freq = read_long("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");

		printf("%s cpu=%.1f%%", ts, pct(&prev[0], &cur[0]));
		for (int i = 1; i <= n; i++)
			printf(" cpu%d=%.1f%%", i - 1, pct(&prev[i], &cur[i]));
		printf(" load=%s", load);
		if (temp >= 0)
			printf(" temp=%.1fC", temp / 1000.0);
		if (freq >= 0)
			printf(" freq=%ldMHz", freq / 1000);
		printf("\n");

		memcpy(prev, cur, sizeof(prev));
	}
}
