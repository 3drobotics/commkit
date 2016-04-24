
# commkit

[![Build Status](https://travis-ci.com/3drobotics/commkit.svg?token=mTutn7HE1YdXxSJUA6Ds&branch=master)](https://travis-ci.com/3drobotics/commkit)

pub/sub middleware layer for an in-development 3DR SDK

### deps

* cmake: `brew install cmake` on OS X, or `apt-get install cmake`
* fast-rtps: https://github.com/eProsima/Fast-RTPS (this must be built and installed somewhere on your system)
* clang-format: OS X: `brew install clang-format`, Linux: `apt-get install clang-format-3.6` (or more recent if available)

## building

```bash
$ git clone git@github.com:3drobotics/commkit.git
$ mkdir build-commkit
$ cd build-commkit
$ cmake ../commkit -DFASTRTPS:STRING=/path/to/installed/fast-rtps
$ make -j8
```

uniform code formatting is enforced via `clang-format`, with formatting rules defined in `.clang-format`. run `make fmt` to format code, or `make fmt-diff` to see which files would be formatted.

## testing

unit testing is supported via [googletest](https://github.com/google/googletest/blob/master/googletest/docs/Primer.md), with tests defined in `test/unit`, generating the test binary that be run via `./test/unit/commkit-tests`.

to disable testing at build time, invoke cmake with `-DCOMMKIT_NO_TEST`.

### notes

commkit currently relies on Fast-RTPS.

What is Fast-RTPS? It is an LGPL implementation in C++ of rtps, the pub sub system we're using. More info:
* [manual](http://www.eprosima.com/docs/fast-rtps/1.0.5/pdf/User-Manual.pdf)
* [performance data](http://www.eprosima.com/index.php/resources-all/performance/40-eprosima-fast-rtps-performance)
* [API reference (doxygen)](http://www.eprosima.com/docs/fast-rtps/1.0.5/html/index.html)
