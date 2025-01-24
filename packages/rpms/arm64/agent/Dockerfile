FROM arm64v8/centos:7

# Install all the necessary tools to build the packages
RUN sed -i 's/mirrorlist/#mirrorlist/g' /etc/yum.repos.d/CentOS-*
RUN sed -i 's|#baseurl=http://mirror.centos.org|baseurl=http://vault.centos.org|g' /etc/yum.repos.d/CentOS-*

RUN yum install -y gcc make wget git openssh-clients sudo gnupg \
    libdb-devel libtool policycoreutils-python epel-release \
    redhat-rpm-config rpm-devel autopoint nspr-devel nss-devel \
    db4-devel rpm-build bison sharutils lzo-devel e2fsprogs-devel \
    libacl-devel libattr-devel openssl-devel kexec-tools \
    libarchive-devel elfutils-libelf-devel patchelf elfutils-devel \
    libgcrypt-devel file-devel gettext-devel curl-devel systemd-devel

RUN yum-builddep python34 -y

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
    tar -zxf cmake-3.30.3.tar.gz && cd cmake-3.30.3 && \
    ./bootstrap --no-system-curl CC=/usr/local/gcc-13.2.0/bin/gcc \
        CXX=/usr/local/gcc-13.2.0/bin/g++ && \
    make -j$(nproc) && make install && cd / && rm -rf cmake-*

RUN git clone https://github.com/git/git.git --branch v2.30.2 --single-branch && \
    cd git && make prefix=/usr/local && make install prefix=/usr/local && \
    ln -fs /usr/local/bin/git /usr/bin/git && cd .. && rm -rf git*

RUN git clone https://github.com/Perl/perl5.git --branch v5.34.0 --single-branch && \
    cd perl5 && ./Configure -des -Dcc='gcc' -Dusethreads && \
    make -j$(nproc) && make install && ln -fs /usr/local/bin/perl /bin/perl && \
    cd / && rm -rf perl*

RUN curl -sO https://lua.org/ftp/lua-5.4.7.tar.gz && tar -xzvf lua-5.4.7.tar.gz && \
    cd lua-5.4.7 && make -j$(nproc) linux CFLAGS+="-fPIC" LDFLAGS+="-fPIC" && make install && \
    ln -fs /usr/local/bin/lua /usr/bin/lua && cd / && rm -rf lua*

RUN mkdir -p /usr/local/lib/pkgconfig && \
    echo "# Package Information for pkg-config" > /usr/local/lib/pkgconfig/lua.pc && \
    echo "prefix=/usr/local" >> /usr/local/lib/pkgconfig/lua.pc && \
    echo "exec_prefix=\${prefix}" >> /usr/local/lib/pkgconfig/lua.pc && \
    echo "libdir=\${exec_prefix}/lib" >> /usr/local/lib/pkgconfig/lua.pc && \
    echo "includedir=\${prefix}/include" >> /usr/local/lib/pkgconfig/lua.pc && \
    echo "" >> /usr/local/lib/pkgconfig/lua.pc && \
    echo "Name: Lua" >> /usr/local/lib/pkgconfig/lua.pc && \
    echo "Description: Lua" >> /usr/local/lib/pkgconfig/lua.pc && \
    echo "Version: 5.4.7" >> /usr/local/lib/pkgconfig/lua.pc && \
    echo "Libs: -L\${libdir} -llua -lm" >> /usr/local/lib/pkgconfig/lua.pc && \
    echo "Cflags: -I\${includedir}" >> /usr/local/lib/pkgconfig/lua.pc

RUN git clone https://github.com/rpm-software-management/rpm.git --branch rpm-4.15.1-release --single-branch && \
    cd rpm && ./autogen.sh && make -j$(nproc) && make install && cd / && rm -rf rpm*

RUN mkdir -p /usr/local/var/lib/rpm && \
    cp /var/lib/rpm/Packages /usr/local/var/lib/rpm/Packages && \
    /usr/local/bin/rpm --rebuilddb && rm -rf /root/rpmbuild

<<<<<<< HEAD
RUN git clone https://github.com/mono/mono.git && cd mono && ./autogen.sh --prefix=/usr/local && \
    make -j $(nproc) && make install -j $(nproc) && cd .. && rm -rf mono && cert-sync /etc/ssl/certs/ca-bundle.crt

RUN yum install ninja-build -y
=======
RUN rpmkeys --import "http://keyserver.ubuntu.com/pks/lookup?op=get&search=0x3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF" && \
    su -c 'curl https://download.mono-project.com/repo/centos7-stable.repo | tee /etc/yum.repos.d/mono-centos7-stable.repo' && \
    yum install mono-devel -y

# Needs to be built later, version might be lower than vcpkg requires
RUN yum install -y ninja-build
>>>>>>> deac4c5a3 (feat: remove no longer need packages from Dockerfiles)
