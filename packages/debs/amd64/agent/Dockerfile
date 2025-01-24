FROM debian:10

ENV DEBIAN_FRONTEND=noninteractive

# Installing necessary packages
RUN apt-get update && apt-get install -y build-essential \
    curl wget git gawk devscripts sudo libssl-dev unzip \
    zip tar pkg-config uuid autopoint libtool gettext \
    ninja-build libsystemd-dev selinux-basics \
    debhelper && rm -rf /var/lib/apt/lists/*

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
    make -j$(nproc) && make install && ln -fs /usr/local/bin/cmake /usr/bin/cmake && \
    cd / && rm -rf cmake-*

RUN git config --global --add safe.directory /wazuh-local-src

RUN apt update && apt install dirmngr gnupg apt-transport-https ca-certificates -y && \
    apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF && \
    sh -c 'echo "deb https://download.mono-project.com/repo/debian stable-buster main" > /etc/apt/sources.list.d/mono-official-stable.list' && \
    apt update && apt install mono-devel pip -y && \
    pip install jinja2

RUN curl -o /usr/local/bin/nuget https://dist.nuget.org/win-x86-commandline/v6.10.2/nuget.exe
