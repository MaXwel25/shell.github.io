#!/bin/bash

# Проверяем, существует ли директория /tmp/vfs, и создаём её при необходимости
if [ ! -d "/tmp/vfs" ]; then
    mkdir -p /tmp/vfs
fi

# Монтируем tmpfs в /tmp/vfs
mountpoint -q /tmp/vfs || mount -t tmpfs tmpfs /tmp/vfs

# Логирование
echo "$(date): VFS смонтирована в /tmp/vfs" >> /var/log/mount_vfs.log

