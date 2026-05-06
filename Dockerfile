FROM alpine:3.19 AS builder
RUN apk add --no-cache build-base
RUN adduser -D -u 10001 appuser
WORKDIR /app
COPY main.c .
RUN gcc -Os -s -o my_app main.c -static

FROM scratch
LABEL org.opencontainers.image.authors="Hubert Luszczew"
COPY --from=builder /etc/passwd /etc/passwd
COPY --from=builder /app/my_app /my_app
USER appuser
EXPOSE 8080
ENTRYPOINT ["/my_app"]