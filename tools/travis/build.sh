#!/bin/bash

set -e

# we start out in tree, cd out and then build
cd ..
mkdir build-commkit
cd build-commkit
cmake ../commkit -DFASTRTPS:STRING=/usr/local
make -j6 VERBOSE=1

# run tests
# test commkit::Node::init() currently fails with "Function not implemented" on Travis.
# have not identified what is throwing that, so disable until we do.
# tests run OK on my OS X machine.
if [[ "$TRAVIS_OS_NAME" != "osx" ]]; then 
    ./test/unit/commkit-tests
fi

# clang-format 3.8 is installed on OS X, which generates different formatting output
# and we haven't updated to it locally yet. re-enable this once we've updated.
if [[ "$TRAVIS_OS_NAME" != "osx" ]]; then 
    make fmt-diff
fi
