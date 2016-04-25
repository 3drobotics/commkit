#!/bin/bash

cd ${TRAVIS_BUILD_DIR}/..

# capn proto
curl -O https://capnproto.org/capnproto-c++-0.5.3.tar.gz
tar zxf capnproto-c++-0.5.3.tar.gz
cd capnproto-c++-0.5.3
cmake -DBUILD_TESTING=OFF -DBUILD_TOOLS=OFF -DEXTERNAL_CAPNP=ON -DCMAKE_CXX_FLAGS=-fpic
sudo make -j6 install

# fast rtps
cd ..
git clone https://github.com/eProsima/Fast-RTPS.git
mkdir build-Fast-RTPS && cd build-Fast-RTPS
cmake ../Fast-RTPS
sudo make -j6 install
