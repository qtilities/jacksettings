#!/bin/bash

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
  [[ "$CXX" != "clang++" ]] || export QMAKESPEC=linux-clang
  [[ "$CXX" != "g++" ]] || export QMAKESPEC=linux-g++
  source /opt/qt$QT/bin/qt$QT-env.sh
  export QT_SELECT=qt5
elif [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  [[ "$CXX" != "clang++" ]] || export QMAKESPEC=macx-clang
  [[ "$CXX" != "g++" ]] || export QMAKESPEC=macx-g++
fi
