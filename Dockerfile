FROM tudinfse/cds_server:latest

RUN apt update && apt install -y --no-install-recommends && apt-get -y install g++ && apt-get -y install libssl-dev && apt-get install make

COPY . /
COPY cds_server.json /etc/cds_server.json

RUN make