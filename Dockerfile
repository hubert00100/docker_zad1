# syntax=docker/dockerfile:1
FROM alpine:3.19 AS builder

RUN apk add --no-cache build-base git openssh-client

RUN mkdir -p -m 0700 ~/.ssh && ssh-keyscan github.com >> ~/.ssh/known_hosts

RUN adduser -D -u 10001 appuser
WORKDIR /app

RUN --mount=type=ssh git clone git@github.com:hubert00100/docker_Zad1.git .

RUN --mount=type=cache,target=/root/.cache \
    gcc -Os -s -o my_app main.c -static

FROM scratch
LABEL org.opencontainers.image.authors="Hubert Luszczew"

COPY --from=builder /etc/passwd /etc/passwd 
COPY --from=builder /app/my_app /my_app 

USER appuser 
EXPOSE 8080 
ENTRYPOINT ["/my_app"] 