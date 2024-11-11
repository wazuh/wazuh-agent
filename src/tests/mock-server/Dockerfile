ARG VERSION=4.2.1
FROM outofcoffee/imposter:${VERSION}

ARG VERSION
COPY --chown=imposter config /opt/imposter/config
ADD --chown=imposter https://github.com/outofcoffee/imposter/raw/refs/tags/v${VERSION}/server/src/main/resources/keystore/ssl.jks /opt/imposter/config

ENV IMPOSTER_LOG_LEVEL=TRACE
