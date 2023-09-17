#!/bin/bash

print_green() {
  echo -e "\e[32m$1\e[0m"
}

print_red() {
  echo -e "\e[31m$1\e[0m"
}

cd src
gcc main.c termify/draw.c termify/log.c termify/page_loader.c termify/terminal.c termify/utils.c interstellar-pong-implementation/interstellar_pong.c interstellar-pong-implementation/interstellar_pong_pages.c interstellar-pong-implementation/player.c interstellar-pong-implementation/materials.c interstellar-pong-implementation/levels.c -o ../InterStellar-Pong.app -trigraphs
cd ..

if [ ! -d "src/termify/temp" ]; then
  mkdir "src/termify/temp"
fi

if [ ! -d "logs" ]; then
     mkdir "logs"
fi

if [ $? -eq 0 ]; then
  print_green "Build successful."
else
  print_red "Build failed."
fi
