#!/bin/bash
set -e

echo "🔹 Установка зависимостей для threat_db (Qt6 + C++ + SQLite)..."

if command -v apt &> /dev/null; then
    sudo apt update
    sudo apt install -y build-essential cmake qt6-base-dev libsqlite3-dev git
    echo "✅ Готово. Соберите: mkdir build && cd build && cmake .. && make"
elif command -v dnf &> /dev/null; then
    sudo dnf install -y gcc-c++ cmake qt6-qtbase-devel sqlite-devel git
    echo "✅ Готово."
elif command -v pacman &> /dev/null; then
    sudo pacman -S --needed --noconfirm base-devel cmake qt6-base sqlite git
    echo "✅ Готово."
else
    echo "❌ Неизвестная ОС. Установите вручную: CMake, Qt6, g++, libsqlite3-dev"
    exit 1
fi
