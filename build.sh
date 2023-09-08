#!/bin/bash

print_green() {
  echo -e "\e[32m$1\e[0m"
}

print_red() {
  echo -e "\e[31m$1\e[0m"
}

gcc main.c draw.c errors.c page_loader.c terminal.c utils.c interstellar_pong.c player.c materials.c levels.c -o InterStellar-Pong.app

if [ $? -eq 0 ]; then
  print_green "Build successful."
else
  print_red "Build failed."
fi
