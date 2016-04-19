#!/bin/bash

cd ${TRAVIS_BUILD_DIR}/..

# capn proto
curl -O https://capnproto.org/capnproto-c++-0.5.3.tar.gz
tar zxf capnproto-c++-0.5.3.tar.gz
cd capnproto-c++-0.5.3
./configure --disable-dependency-tracking
sudo make -j6 install

# fast rtps
git clone https://github.com/eProsima/Fast-RTPS.git
mkdir build-Fast-RTPS && cd build-Fast-RTPS
cmake ../Fast-RTPS
sudo make -j6 install
