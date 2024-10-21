FROM centos:7

# Install all the necessary tools to build the packages
RUN rm /etc/yum.repos.d/* && echo "exactarch=1" >> /etc/yum.conf
COPY CentOS-Base.repo /etc/yum.repos.d/CentOS-Base.repo
RUN rpm --import /etc/pki/rpm-gpg/RPM-GPG-KEY-CentOS-7
RUN yum clean all && yum update -y

RUN yum install -y gcc make wget git \
    openssh-clients sudo gnupg \
    automake autoconf libtool policycoreutils-python \
    yum-utils epel-release redhat-rpm-config rpm-devel \
    autopoint gettext nspr nspr-devel \
    nss nss-devel magic magic-devel \
    db4 db4-devel zlib zlib-devel rpm-build bison \
    sharutils bzip2-devel xz-devel lzo-devel \
    e2fsprogs-devel libacl-devel libattr-devel \
    openssl-devel libxml2-devel kexec-tools elfutils \
    libarchive-devel elfutils-libelf-devel \
    patchelf elfutils-devel libgcrypt-devel \
    file-devel gettext-devel curl-devel systemd-devel

RUN yum-builddep python34 -y
RUN curl --silent --location https://rpm.nodesource.com/setup_8.x | bash -
RUN yum install -y nodejs

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

RUN curl -O http://packages.wazuh.com/utils/autoconf/autoconf-2.69.tar.gz && \
    gunzip autoconf-2.69.tar.gz && tar xvf autoconf-2.69.tar && \
    cd autoconf-2.69 && ./configure && make -j$(nproc) && \
    make install && cd / && rm -rf autoconf-*

RUN git clone https://github.com/sqlite/sqlite.git --branch version-3.46.1 --single-branch && \
    cd sqlite && ./configure && make -j$(nproc) && \
    make install -j$(nproc) && ln -fs /usr/local/bin/sqlite3 /usr/bin/sqlite3 && \
    cd / && rm -rf sqlit*

RUN curl -sO https://lua.org/ftp/lua-5.4.7.tar.gz && tar -xzvf lua-5.4.7.tar.gz && \
    cd lua-5.4.7 && make -j$(nproc) linux CFLAGS+="-fPIC" LDFLAGS+="-fPIC" && make install && \
    ln -fs /usr/local/bin/lua /usr/bin/lua && cd / && rm -rf lua*

RUN echo "# Package Information for pkg-config" > /usr/local/lib/pkgconfig/lua.pc && \
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

RUN git clone https://github.com/rpm-software-management/popt.git --branch popt-1.19-release --single-branch && \
    cd popt && ./autogen.sh && ./configure && make -j$(nproc) && make install -j$(nproc) && \
    cd / && rm -rf popt*

RUN git clone https://github.com/rpm-software-management/rpm.git --branch rpm-4.18.2-release --single-branch && \
    cd rpm && ./autogen.sh && make -j$(nproc) && make install && cd / && rm -rf rpm*

RUN mkdir -p /usr/local/var/lib/rpm && \
    cp /var/lib/rpm/Packages /usr/local/var/lib/rpm/Packages && \
    /usr/local/bin/rpm --rebuilddb && rm -rf /root/rpmbuild
