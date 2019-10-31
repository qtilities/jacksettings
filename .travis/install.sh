#!/bin/bash

set -e

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
  sudo apt-get -y install libgl1-mesa-dev qt512base
fi
