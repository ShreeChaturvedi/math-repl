FROM ubuntu:24.04 AS build

RUN apt-get update \
    && apt-get install -y --no-install-recommends cmake g++ make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

RUN cmake -S . -B build -DREPL_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --config Release

FROM ubuntu:24.04

RUN useradd -m repl
COPY --from=build /src/build/src/repl /usr/local/bin/repl
USER repl

ENTRYPOINT ["repl"]
