FROM ubuntu:24.04

WORKDIR /root

ARG DEBIAN_FRONTEND=noninteractive
ARG PYTHON_VERSION=3.10
ARG GCC_VERSION=10
ARG CLANG_VERSION=14

RUN apt-get update && \
    apt-get install -y --no-install-recommends software-properties-common gpg-agent && \
    apt-get update && \
    add-apt-repository ppa:deadsnakes/ppa && \
    add-apt-repository ppa:ubuntu-toolchain-r/test

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        software-properties-common \
        build-essential \
        gcc-${GCC_VERSION} \
        g++-${GCC_VERSION} \
        clang-${CLANG_VERSION} \
        llvm-${CLANG_VERSION} \
        lld-${CLANG_VERSION} \
        libc++-${CLANG_VERSION}-dev \
        libc++abi-${CLANG_VERSION}-dev \
        git \
        git-lfs \
        jq \
        openssh-client \
        curl \
        python${PYTHON_VERSION} \
        python${PYTHON_VERSION}-venv \
        libpython${PYTHON_VERSION}-dev && \
    apt-get clean && rm -rf /var/lib/apt/lists/* && \
    git lfs install

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-${GCC_VERSION} ${GCC_VERSION} \
        --slave /usr/bin/g++ g++ /usr/bin/g++-${GCC_VERSION} \
        --slave /usr/bin/gcov gcov /usr/bin/gcov-${GCC_VERSION} \
        --slave /usr/bin/gcc-ar gcc-ar /usr/bin/gcc-ar-${GCC_VERSION} \
        --slave /usr/bin/gcc-ranlib gcc-ranlib /usr/bin/gcc-ranlib-${GCC_VERSION} && \
    update-alternatives --install /usr/bin/llvm-symbolizer llvm-symbolizer /usr/bin/llvm-symbolizer-${CLANG_VERSION} ${CLANG_VERSION}

ENV VIRTUAL_ENV=/venv/py${PYTHON_VERSION}
ENV PATH=${VIRTUAL_ENV}/bin:${PATH}

RUN python${PYTHON_VERSION} -m venv ${VIRTUAL_ENV} && \
    pip install --no-cache-dir --upgrade pip setuptools wheel && \
    pip install --no-cache-dir cmake conan ninja pre-commit pybind11 twine pybind11-stubgen

CMD ["bash", "-l"]
