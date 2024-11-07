ARG VERSION=4.2.1
FROM outofcoffee/imposter:${VERSION}

ARG VERSION
COPY --chown=imposter config /opt/imposter/config
ADD --chown=imposter https://github.com/outofcoffee/imposter/raw/refs/tags/v${VERSION}/server/src/main/resources/keystore/ssl.jks /opt/imposter/config

ENV IMPOSTER_LOG_LEVEL=TRACE
EXPOSE 27000

CMD ["--plugin=openapi", "--plugin=rest", "--plugin=soap", "--configDir=/opt/imposter/config", "--listenPort=27000", "--tlsEnabled", "--keystorePath=/opt/imposter/config/ssl.jks", "--keystorePassword=password"]
