#!/bin/bash
echo "Очистка проекта threat_db..."

if [ -d "build" ]; then
    rm -rf build
    echo "Папка build удалена."
fi

if [ -f "threats_manager.db" ]; then
    read -p "Удалить базу данных threats_manager.db? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        rm -f threats_manager.db
        echo "База данных удалена."
    fi
fi

echo "Очистка завершена."
