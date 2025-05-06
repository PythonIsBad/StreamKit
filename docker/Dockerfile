FROM ubuntu:latest

RUN apt-get update && \
    apt-get install -y g++ build-essential \
    libsfml-dev \
    cmake \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY *.cpp *.h /app/

RUN g++ -o myserver Main.cpp -lsfml-network

EXPOSE 55000/udp
EXPOSE 55001/udp
EXPOSE 55002/udp
EXPOSE 55003/udp

CMD ["./myserver"]