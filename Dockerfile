FROM debian:latest
RUN apt-get update && apt-get install --assume-yes --no-install-recommends ca-certificates curl
RUN apt-get install --assume-yes --no-install-recommends g++ libarchive-dev libcurl4-openssl-dev libexpat1-dev libjsoncpp-dev liblzma-dev make zlib1g-dev &&\
 mkdir -p /usr/src/cmake &&\
 curl --location --output - https://cmake.org/files/v3.6/cmake-3.6.1.tar.gz | tar --directory=/usr/src/cmake --extract --file=- --gzip --strip-components=1 &&\
 mkdir -p /tmp/build/cmake &&\
 cd /tmp/build/cmake &&\
 /usr/src/cmake/bootstrap --prefix=/usr --system-libs &&\
 make &&\
 make install
RUN apt-get install --assume-yes --no-install-recommends libssl-dev &&\
 mkdir -p /usr/src/libssh &&\
 curl --location --output - https://git.libssh.org/projects/libssh.git/snapshot/libssh-0.7.3.tar.gz | tar --directory=/usr/src/libssh --extract --file=- --gzip --strip-components=1 &&\
 mkdir -p /tmp/build/libssh &&\
 cd /tmp/build/libssh &&\
 cmake -D CMAKE_BUILD_TYPE=Debug -D CMAKE_INSTALL_PREFIX=/usr -D LIB_INSTALL_DIR=/usr/lib/x86_64-linux-gnu /usr/src/libssh &&\
 make &&\
 make install
RUN mkdir -p /usr/src/tcl &&\
 curl --location --output - http://prdownloads.sourceforge.net/tcl/tcl8.6.5-src.tar.gz | tar --directory=/usr/src/tcl --extract --file=- --gzip --strip-components=1 &&\
 mkdir -p /tmp/build/tcl &&\
 cd /tmp/build/tcl &&\
 CFLAGS=-DTCL_MEM_DEBUG /usr/src/tcl/unix/configure --enable-64bit --enable-symbols --libdir=/usr/lib/x86_64-linux-gnu --prefix=/usr &&\
 make &&\
 make install
RUN mkdir -p /usr/src/tcllib &&\
 curl --location --output - http://prdownloads.sourceforge.net/tcllib/tcllib-1.18.tar.gz | tar --directory=/usr/src/tcllib --extract --file=- --gzip --strip-components=1 &&\
 mkdir -p /tmp/build/tcllib &&\
 cd /tmp/build/tcllib &&\
 /usr/src/tcllib/configure --libdir=/usr/lib/x86_64-linux-gnu --prefix=/usr &&\
 make &&\
 make install
COPY . /usr/src/atdt
WORKDIR /usr/src/atdt
RUN apt-get install --assume-yes --no-install-recommends gnulib libssl1.0.0-dbg valgrind &&\
 gnulib-tool --update &&\
 autoreconf --install &&\
 ./configure --libdir=/usr/lib/x86_64-linux-gnu --prefix=/usr
