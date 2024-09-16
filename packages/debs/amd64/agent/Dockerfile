FROM debian:10

ENV DEBIAN_FRONTEND=noninteractive

# Installing necessary packages
RUN apt-get update && apt-get install -y build-essential \
            curl \
            git \
            gawk \
            devscripts \
            sudo \
            libssl-dev \
            unzip \
            zip \
            tar \
            texinfo \
            pkg-config \
            clang-tidy \
            uuid \
            libmpfr-dev \
            libgmp3-dev \
            libmpc-dev \
            && rm -rf /var/lib/apt/lists/*


RUN curl -OL https://github.com/Kitware/CMake/releases/download/v3.30.3/cmake-3.30.3.tar.gz && \
    tar -zxf cmake-3.*.tar.gz && cd cmake-3.* && ./bootstrap && \
    make -j$(nproc) && make install && ln -s /usr/local/bin/cmake /usr/bin/cmake && \
    cd / && rm -rf cmake-*
