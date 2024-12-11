#!/bin/bash

# Проверка, существует ли файл history.txt, и удаляем его, если да
if [ -f history.txt ]; then
    rm history.txt
fi

# Компиляция программы на C
gcc shell.c -o shell

# Проверка успешной компиляции
if [ $? -eq 0 ]; then
    echo "Компиляция успешна, запускаем shell..."
    ./shell
else
    echo "Ошибка компиляции. Проверьте код."
fi
