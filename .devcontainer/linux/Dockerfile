# Example Dockerfile to showcase possibilities of prepping a dev container
# This file can be customized to include anything you would need on your Linux installation.

ARG NUKE_VERSION

FROM "ghcr.io/mateuszwojt/nukedockerbuild:${NUKE_VERSION}-linux"

ARG OIDN_VERSION

# Install dependencies
RUN dnf install -y git wget

# Download and install OpenImageDenoise
RUN cd /tmp && \
    wget -q https://github.com/OpenImageDenoise/oidn/releases/download/v${OIDN_VERSION}/oidn-${OIDN_VERSION}.x86_64.linux.tar.gz && \
    mkdir -p /opt && \
    tar -C /opt -xzf oidn-${OIDN_VERSION}.x86_64.linux.tar.gz && \
    rm -f oidn-${OIDN_VERSION}.x86_64.linux.tar.gz

# Add OpenImageDenoise to the system path
ENV PATH="/opt/oidn-${OIDN_VERSION}.x86_64.linux/bin:${PATH}"