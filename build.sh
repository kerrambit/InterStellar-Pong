#!/bin/bash

print_green() {
  echo -e "\e[32m$1\e[0m"
}

print_red() {
  echo -e "\e[31m$1\e[0m"
}

cd src
gcc main.c termify/draw.c termify/errors.c termify/page_loader.c termify/terminal.c termify/utils.c interstellar-pong-implementation/interstellar_pong.c interstellar-pong-implementation/player.c interstellar-pong-implementation/materials.c interstellar-pong-implementation/levels.c -o ../InterStellar-Pong.app
cd ..

if [ ! -d "src/termify/termp" ]; then
  mkdir "src/termify/termp"
fi

if [ $? -eq 0 ]; then
  print_green "Build successful."
else
  print_red "Build failed."
fi
