FROM debian:latest
RUN apt-get update &&\
 apt-get install --assume-yes --no-install-recommends\
 automake ca-certificates cmake curl g++ gnulib libssl1.0.0-dbg libssl-dev\
 libtool make valgrind xz-utils
WORKDIR /usr/src
RUN curl -Lso - https://red.libssh.org/attachments/download/195/libssh-0.7.3.tar.xz | xz -d | tar xf - &&\
 curl -Lso - http://prdownloads.sourceforge.net/tcl/tcl8.6.5-src.tar.gz | tar xzf - &&\
 curl -Lso - http://prdownloads.sourceforge.net/tcllib/tcllib-1.18.tar.gz | tar xzf -
WORKDIR /tmp/build/libssh-0.7.3
RUN cmake -D CMAKE_BUILD_TYPE=Debug -D CMAKE_INSTALL_PREFIX=/usr -D LIB_INSTALL_DIR=/usr/lib/x86_64-linux-gnu /usr/src/libssh-0.7.3 &&\
 make &&\
 make install
WORKDIR /tmp/build/tcl8.6.5
RUN CFLAGS=-DTCL_MEM_DEBUG /usr/src/tcl8.6.5/unix/configure --enable-64bit --enable-symbols --libdir=/usr/lib/x86_64-linux-gnu --prefix=/usr &&\
 make &&\
 make install
WORKDIR /tmp/build/tcllib-1.18
RUN /usr/src/tcllib-1.18/configure --libdir=/usr/lib/x86_64-linux-gnu --prefix=/usr &&\
 make &&\
 make install
COPY . /usr/src/app
WORKDIR /usr/src/app
