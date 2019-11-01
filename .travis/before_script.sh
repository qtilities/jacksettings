#!/bin/bash

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
  source /opt/qt${QT}/bin/qt${QT}-env.sh
  export QT_SELECT=qt5
fi
