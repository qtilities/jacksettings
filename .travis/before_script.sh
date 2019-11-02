#!/bin/bash

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
  source /opt/qt*/bin/qt*-env.sh
  export QT_SELECT=qt5
fi
