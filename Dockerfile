FROM ubuntu:26.10

ENV DEBIAN_FRONTEND=noninteractive
ENV WORKSPACE=/workspace
WORKDIR ${WORKSPACE}

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    clang \
    clangd \
    cmake \
    lld \
    llvm \
    llvm-dev \
    llvm-runtime \
    mingw-w64 \
    ninja-build \
    python3 \
    python3-minimal \
    && rm -rf /var/lib/apt/lists/*

COPY build.py /usr/local/bin/build
RUN chmod +x /usr/local/bin/build

RUN ln -s /usr/bin/llvm-windres-18 /usr/local/bin/llvm-windres

ENTRYPOINT ["/usr/local/bin/build"]
