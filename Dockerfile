FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    cmake g++ git make \
    qt6-base-dev libsqlite3-dev libssl-dev \
    libgl1 libegl1 \
    libgl1-mesa-dev libegl1-mesa-dev libgles2-mesa-dev \
    x11-apps \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .
RUN mkdir -p /app/data

# Собираем в подкаталоге, но копируем бинарь в текущую директорию (/app)
RUN mkdir build && \
    cd build && \
    cmake .. && \
    make -j$(nproc) && \
    mv threat_db /app/

CMD ./threat_db
