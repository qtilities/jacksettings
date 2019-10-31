#!/bin/bash

set -e

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
  sudo apt-get -y install libasound2-dev libjack-jackd2-dev libgl1-mesa-dev qt512base
# source /opt/qt512/bin/qt512-env.sh
# exit 0
fi
