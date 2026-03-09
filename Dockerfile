FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    make \
    libjemalloc-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY *.cpp *.hpp CMakeLists.txt ./

RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

# ---

FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libjemalloc2 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/build/redis-clone .

EXPOSE 8080

CMD ["./redis-clone"]
