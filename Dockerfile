# Built by Pantahub (start_build, mode git) per target architecture.
FROM alpine:3.20 AS build
RUN apk add --no-cache build-base
WORKDIR /src
COPY main.c .
RUN gcc -O2 -Wall -Wextra -o cpustats main.c && strip cpustats

FROM alpine:3.20
COPY --from=build /src/cpustats /usr/bin/cpustats
ENV CPUSTATS_INTERVAL=5
ENTRYPOINT ["/usr/bin/cpustats"]
