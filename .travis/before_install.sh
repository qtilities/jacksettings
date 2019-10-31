#!/bin/bash

set -e

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
  sudo add-apt-repository ppa:beineri/opt-qt-5.12.3-xenial -y
  sudo apt-get update -qq
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
  brew update
fi
