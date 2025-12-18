FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Установка всех зависимостей
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

# Сборка
RUN mkdir build && cd build && cmake .. && make -j$(nproc)

# Запуск
CMD if [ -z "$DISPLAY" ]; then \
        echo "Ошибка: запустите с -e DISPLAY и томом X11"; \
        exit 1; \
    else \
        /app/build/threat_db; \
    fi
