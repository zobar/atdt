FROM debian:latest
RUN apt-get update && apt-get install --assume-yes --no-install-recommends critcl libssh-dev
WORKDIR /usr/src/app
