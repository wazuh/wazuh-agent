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

RUN echo "deb http://ftp.de.debian.org/debian bullseye main" >>/etc/apt/sources.list && \
    apt-get update && apt-get install -y g++-10 && rm -rf /var/lib/apt/lists/*

RUN curl -O http://ftp.gnu.org/gnu/gcc/gcc-14.1.0/gcc-14.1.0.tar.gz && \
        tar -xf gcc-14.1.0.tar.gz && cd gcc-14.1.0 && ./configure -v --build=x86_64-linux-gnu \
        --host=x86_64-linux-gnu --target=x86_64-linux-gnu --prefix=/usr/local/gcc-14.1.0 \
        --enable-checking=release --enable-languages=c,c++ --disable-multilib --program-suffix=-14.1.0 && \
        make -j$(nproc) && make install && cd / && rm -rf gcc-14*
RUN update-alternatives --install /usr/bin/gcc gcc /usr/local/gcc-14.1.0/bin/gcc-14.1.0 14
RUN update-alternatives --install /usr/bin/g++ g++ /usr/local/gcc-14.1.0/bin/g++-14.1.0 14

RUN apt-get update && apt-get install -y \
        ninja-build \
        libsystemd-dev \
        libaudit-dev \
        selinux-basics  \
        && rm -rf /var/lib/apt/lists/*
