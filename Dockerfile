FROM debian:latest
RUN apt-get update && apt-get install --assume-yes --no-install-recommends automake libtool libssh-dev tcl-dev
COPY . /usr/src/app
WORKDIR /usr/src/app
