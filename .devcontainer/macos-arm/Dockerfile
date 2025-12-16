# Example Dockerfile to showcase possibilities of prepping a dev container
# This file can be customized to include anything you would need on your MacOS ARM installation.

ARG NUKE_VERSION

FROM "ghcr.io/mateuszwojt/nukedockerbuild:${NUKE_VERSION}-macos-arm"

ARG OIDN_VERSION

RUN apt-get update && apt-get install -y git

RUN cd /tmp && \
    wget -q -P /tmp/ https://github.com/OpenImageDenoise/oidn/releases/download/v2.1.0/oidn-2.1.0.x86_64.macos.tar.gz && \
    mkdir -p /opt && \
    tar -C /opt -xvzf /tmp/oidn-2.1.0.x86_64.macos.tar.gz && \
    rm -vf /tmp/*