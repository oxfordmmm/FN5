FROM ubuntu:latest

RUN apt update --fix-missing

RUN apt upgrade -y

RUN apt install -y git bash g++
RUN apt install -y make
RUN apt install -y cmake
RUN apt install -y python3 python3-pip mysql-client libmysqlclient-dev
RUN apt install -y curl jq

RUN git clone https://github.com/oxfordmmm/FN5.git && \
    cd FN5 && \
    git checkout refactor-cmake && \ 
    bash build.sh && \
    cp fn5 /usr/bin/fn5 && \
    pip install -r requirements.txt
