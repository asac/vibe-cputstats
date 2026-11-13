# cpustats

Small Pantavisor container that logs CPU statistics to stdout at a fixed interval:
total and per-core utilisation (from `/proc/stat`), load average, SoC temperature and
current CPU frequency.

```
2026-11-13T21:34:45Z cpu=5.9% cpu0=2.8% cpu1=7.4% cpu2=4.0% cpu3=9.2% load=0.54/0.35/0.14 temp=66.1C freq=1500MHz
```

The interval is set with `CPUSTATS_INTERVAL` (seconds, default 5).

## Build

### Local, with the Pantavisor SDK

```sh
. /opt/pv-sdk/environment-setup-armv8a-poky-linux-musl
make export        # writes cpustats.pvrexport.tgz, signed with the SDK's developer key
```

Upload the export to Pantahub and merge it into a device revision.

### On Pantahub, from this repository

Pantahub's builder (`start_build`, mode `git`) builds the `Dockerfile` at a given ref for
the requested platform and packs the image into a pvr export.
