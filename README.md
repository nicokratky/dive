# DiVE - Distance Vector Routing Simulation

## Prerequisites

Faps has following dependencies:

- [asio](https://github.com/chriskohlhoff/asio)
- [spdlog](https://github.com/gabime/spdlog)
- [fmt](https://github.com/fmtlib/fmt)
- [json](https://github.com/nlohmann/json)
- [clipp](https://github.com/muellan/clipp)

## Installation

```sh
export ASIO_INCLUDE_PATH=~/clones/asio/asio/include/
export SPDLOG_INCLUDE_PATH=~/clones/spdlog/include/
export FMT_PATH=~/clones/fmt/
export JSON_INCLUDE_PATH=~/clones/json/single_include/
export CLIPP_INCLUDE_PATH=~/clones/clipp/include/

hg clone https://bitbucket.org/nicokratky/dive
cd dive
mkdir build
cd build
cmake ..
make
```

## Usage

```
SYNOPSIS
        ./router <topology file> <router id> [-i <seconds>] [-v]

OPTIONS
        -i, --interval
                    router update interval (default: 5)

        -v, --verbose
                    print additional debug information
```

## Licence

Boost Software License
