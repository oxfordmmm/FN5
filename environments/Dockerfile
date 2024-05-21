FROM ubuntu:22.04

#Setup
RUN apt update --fix-missing
RUN apt upgrade -y

#Install FN5 compile requirements
RUN apt install -y git bash g++
RUN apt install -y make
RUN apt install -y cmake

#Install python for DB things
RUN apt install -y python3 python3-pip mysql-client libmysqlclient-dev

#Script requirements
RUN apt install -y curl jq pigz

#Compile and install FN5
RUN git clone https://github.com/oxfordmmm/FN5.git && \
    cd FN5 && \
    bash build.sh && \
    cp fn5 /usr/bin/fn5 && \
    pip install -r requirements.txt
