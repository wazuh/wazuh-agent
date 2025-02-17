FROM arm64v8/debian:10

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y build-essential \
    curl wget git devscripts sudo libssl-dev unzip zip \
    tar pkg-config autopoint libtool gettext ninja-build \
    libsystemd-dev selinux-basics debhelper nuget \
    && rm -rf /var/lib/apt/lists/*

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

ENV CPLUS_INCLUDE_PATH "/usr/local/gcc-13.2.0/include/c++/13.2.0/"
ENV LD_LIBRARY_PATH "/usr/local/gcc-13.2.0/lib64/"
ENV PATH "/usr/local/gcc-13.2.0/bin:${PATH}"
ENV PKG_CONFIG_PATH "/usr/local/lib/pkgconfig:${PKG_CONFIG_PATH}"

RUN curl -OL https://github.com/Kitware/CMake/releases/download/v3.30.3/cmake-3.30.3.tar.gz && \
    tar -zxf cmake-3.*.tar.gz && cd cmake-3.* && ./bootstrap && \
    make -j$(nproc) && make install && ln -s /usr/local/bin/cmake /usr/bin/cmake && \
    cd / && rm -rf cmake-*

RUN git config --global --add safe.directory /wazuh-local-src

RUN git clone https://github.com/mono/mono.git && cd mono && \
    ./autogen.sh --prefix=/usr/local && make -j $(nproc) && make install -j $(nproc) && \
    cd .. && rm -rf mono && update-ca-certificates && cert-sync /etc/ssl/certs/ca-certificates.crt
