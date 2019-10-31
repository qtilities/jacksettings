#!/bin/bash

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
  sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 100
  gcc -v && g++ -v && $SHELL --version

  qmake JackSettings.pro -spec linux-g++ CONFIG+=release PREFIX=/usr
  make -j$(nproc)
elif [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  qmake JackSettings.pro -spec macx-clang CONFIG+=release
  make -j$(sysctl -n hw.physicalcpu)
fi
