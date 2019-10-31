#!/bin/bash

if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  brew link --force qt5
  export PATH=/usr/local/opt/qt5/bin:$PATH
fi
