FROM debian:latest
RUN apt-get update && apt-get install --assume-yes --no-install-recommends automake gdb libtool libssh-dev tcl-dev
WORKDIR /usr/src/app
