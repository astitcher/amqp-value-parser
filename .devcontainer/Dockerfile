FROM registry.fedoraproject.org/fedora:44

ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=1000

RUN dnf -y install sudo shadow-utils passwd dnf-plugins-core @development-tools gcc-g++ cmake ninja-build pkgconfig re2c gperf lemon readline-devel qpid-proton-c-devel && \
    dnf clean all

RUN groupadd --gid ${USER_GID} ${USERNAME} || true \
 && useradd -m -s /bin/bash --uid ${USER_UID} --gid ${USER_GID} ${USERNAME} || true \
 && echo "${USERNAME} ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/${USERNAME} \
 && chmod 0440 /etc/sudoers.d/${USERNAME}

USER ${USERNAME}
ENV HOME=/home/${USERNAME}
