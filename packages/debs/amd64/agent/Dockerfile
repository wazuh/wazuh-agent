FROM debian:10

ENV DEBIAN_FRONTEND=noninteractive

# Installing necessary packages
RUN apt-get update && apt-get install -y build-essential \
            curl wget git gawk devscripts sudo libssl-dev \
            unzip zip tar texinfo pkg-config clang-tidy uuid \
            libmpfr-dev libgmp3-dev libmpc-dev autopoint libtool \
            zlib1g-dev libgcrypt20-dev libmagic-dev libpopt-dev \
            libmagic-dev libsqlite3-dev  gettext \
            ninja-build libsystemd-dev libaudit-dev selinux-basics \
            libarchive-dev debhelper && rm -rf /var/lib/apt/lists/*

RUN echo "deb http://ftp.de.debian.org/debian bullseye main" >>/etc/apt/sources.list && \
    apt-get update && apt-get install -y g++-10 && rm -rf /var/lib/apt/lists/*

RUN wget https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.gz && \
    tar xzf gcc-13.2.0.tar.gz  && cd gcc-13.2.0/ && \
    ./contrib/download_prerequisites && \
    ./configure --prefix=/usr/local/gcc-13.2.0 --enable-languages=c,c++ \
        --disable-multilib --disable-libsanitizer && \
    make -j$(nproc) && make install -j$(nproc) && \
    ln -fs /usr/local/gcc-13.2.0/bin/g++ /usr/bin/c++ && \
    ln -fs /usr/local/gcc-13.2.0/bin/gcc /usr/bin/cc && cd .. && rm -rf gcc-*

ENV PKG_CONFIG_PATH "/usr/local/lib/pkgconfig:${PKG_CONFIG_PATH}"

RUN curl -OL https://github.com/Kitware/CMake/releases/download/v3.30.3/cmake-3.30.3.tar.gz && \
    tar -zxf cmake-3.*.tar.gz && cd cmake-3.* && ./bootstrap && \
    make -j$(nproc) && make install && ln -s /usr/local/bin/cmake /usr/bin/cmake && \
    cd / && rm -rf cmake-*

RUN git config --global --add safe.directory /wazuh-local-src

RUN git clone --branch test/128-140-packages --single-branch --recurse-submodules https://github.com/wazuh/wazuh-agent.git && \
    cd wazuh-agent/src && mkdir build && cd build && cmake .. && cd / && rm -rf wazuh-*
