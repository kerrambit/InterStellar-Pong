#!/bin/bash

print_red() {
  echo -e "\e[31m$1\e[0m"
}

if command -v valgrind &> /dev/null; then

  if [ "$1" == "debug" ] || [ "$1" == "DEBUG" ] || [ "$1" == "d" ] || [ "$1" == "D" ]; then
    valgrind --leak-check=full --show-reachable=yes --track-origins=yes ./InterStellar-Pong.app 2> debug.log
    echo -e "Debug data are to be found in "debug.log"."
  else
    ./InterStellar-Pong.app
  fi
else
  print_red "Valgrind is not installed. Please install it to use debugging mode."
  exit 1
fi
