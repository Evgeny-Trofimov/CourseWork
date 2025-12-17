FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    cmake g++ make qt6-base-dev libsqlite3-dev libgl1 x11-apps \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN mkdir build && cd build \
    && cmake .. \
    && make -j$(nproc)

CMD if [ -z "$DISPLAY" ]; then \
        echo "Ошибка: Переменная DISPLAY не установлена."; \
        echo "Запустите так:"; \
        echo "  xhost +local:root"; \
        echo "  docker run -it --rm -e DISPLAY=\$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix threat_db"; \
        exit 1; \
    else \
        /app/build/threat_db; \
    fi
