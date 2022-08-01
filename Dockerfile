# Build Stage
FROM --platform=linux/amd64 ubuntu:20.04 as builder

## Install build dependencies.
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y cmake clang git build-essential re2c libcpputest-dev libubsan1

## Add source code to the build stage.
ADD . /timelib
WORKDIR /timelib

## Build
RUN make fuzzer -j$(nproc) BUILD_FUZZ=1

## Package Stage
FROM --platform=linux/amd64 ubuntu:20.04
COPY --from=builder /timelib/timelib-fuzz /

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y libubsan1

## Set up fuzzing!
ENTRYPOINT []
CMD /timelib-fuzz -close_fd_mask=2
