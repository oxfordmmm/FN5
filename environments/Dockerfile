FROM ubuntu:22.04

#Setup
RUN apt update
RUN apt upgrade -y
RUN apt install -y git bash g++ make cmake python3 python3-pip mysql-client libmysqlclient-dev curl jq pigz

#Compile and install FN5
RUN git clone https://github.com/oxfordmmm/FN5.git && \
    cd FN5 && \
    bash build.sh && \
    cp fn5 /usr/bin/fn5 && \
    pip install -r requirements.txt
