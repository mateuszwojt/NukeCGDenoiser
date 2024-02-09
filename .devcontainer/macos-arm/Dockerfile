# Example Dockerfile to showcase possibilities of prepping a dev container
# This file can be customized to include anything you would need on your MacOS ARM installation.

ARG NUKE_VERSION=15.0
ARG OIDN_VERSION=2.1.0
FROM "ghcr.io/gillesvink/nukedockerbuild:${NUKE_VERSION}-macos-arm-latest"
RUN apt-get update && apt-get install -y git
RUN cd /tmp && \
    wget -q -P /tmp/ https://github.com/OpenImageDenoise/oidn/releases/download/v2.1.0/oidn-2.1.0.x86_64.macos.tar.gz && \
    mkdir -p /opt && \
    tar -C /opt -xvzf /tmp/oidn-2.1.0.x86_64.macos.tar.gz && \
    rm -vf /tmp/*