#!/bin/bash

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
  sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 100
  ${SHELL} --version && gcc -v && qmake -v 

  qmake ${PROJECT_NAME}.pro -spec ${QMAKESPEC} CONFIG+=release PREFIX=/usr
  make -j$(nproc)
elif [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  qmake ${PROJECT_NAME}.pro -spec ${QMAKESPEC} CONFIG+=release
  make -j$(sysctl -n hw.physicalcpu)
fi
