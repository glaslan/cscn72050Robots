FROM ubuntu:latest

RUN apt update && apt install -y \
    gcc \
    g++ \
    vim \
    cmake \
    libboost-all-dev \
    libasio-dev

WORKDIR /web