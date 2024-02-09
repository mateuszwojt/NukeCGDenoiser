# Example Dockerfile to showcase possibilities of prepping a dev container
# This file can be customized to include anything you would need on your Linux installation.

ARG NUKE_VERSION=15.0
ARG OIDN_VERSION=2.1.0
FROM "ghcr.io/gillesvink/nukedockerbuild:${NUKE_VERSION}-linux-latest"
RUN dnf install -y git wget
RUN cd /tmp
RUN wget -q -P /tmp/ https://github.com/OpenImageDenoise/oidn/releases/download/v2.1.0/oidn-2.1.0.x86_64.linux.tar.gz
RUN mkdir -p /opt
RUN tar -C /opt -xvzf /tmp/oidn-2.1.0.x86_64.linux.tar.gz
RUN rm -vf /tmp/*