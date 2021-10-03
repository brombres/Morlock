#!/bin/sh

HOME="/opt/morlock"
if ! [ -d "$HOME" ]; then
  echo --------------------------------------------------------------------------------
  echo Installing Morlock
  echo --------------------------------------------------------------------------------
  echo Creating home folder: $HOME
  mkdir "$HOME" &> /dev/null
  if ! [ -d "$HOME" ]; then
    echo '> sudo mkdir' "$HOME"
    sudo mkdir "$HOME"
  fi
fi

GROUP=""
if ! chown $USER$GROUP "$HOME" &> /dev/null; then
  echo '>' sudo chown $USER$GROUP "$HOME"
  sudo chown $USER$GROUP "$HOME"
fi

mkdir -p "$HOME/build/abepralle/morlock"
if ! [ -f "$HOME/build/abepralle/morlock/download.success" ]; then
  echo Downloading Morlock bootstrap source...
  curl -fsSL https://raw.githubusercontent.com/AbePralle/Morlock/main/Source/Bootstrap/Morlock.h \
    -o "$HOME/build/abepralle/morlock/Morlock.h"
  curl -fsSL https://raw.githubusercontent.com/AbePralle/Morlock/main/Source/Bootstrap/Morlock.cpp \
    -o "$HOME/build/abepralle/morlock/Morlock.cpp"
  echo success >> "$HOME/build/abepralle/morlock/download.success"
fi

if ! [ -f "$HOME/build/abepralle/morlock/compile.success" ]; then
  if c++ -O3 -Wall -std=gnu++11 -fno-strict-aliasing -Wno-invalid-offsetof \
    "$HOME/build/abepralle/morlock/Morlock.cpp" \
    -o "$HOME/build/abepralle/morlock/morlock"; then
    chmod a+x "$HOME/build/abepralle/morlock/morlock"
    echo success >> "$HOME/build/abepralle/morlock/compile.success"
  fi
fi

"$HOME/build/abepralle/morlock/morlock"

