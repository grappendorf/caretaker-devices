FROM alpine:3.3

RUN apk upgrade --update
RUN apk add --no-cache hiredis libstdc++ libevent
RUN mkdir /lib64 && ln -s /lib/libc.musl-x86_64.so.1 /lib64/ld-linux-x86-64.so.2
ADD sim /usr/bin

ENTRYPOINT ["sim"]
