## AYAB - All Yarns Are Beautiful


_The goal of the AYAB project is to provide an alternative way to control the famous Brother KH-9xx range of knitting machines using a computer_

http://ayab-knitting.com


-----

# AYAB Firmware

This is the firmware for the Arduino-based hardware that is part of the AYAB project.

## Updating your AYAB firmware

In [ayab-desktop](https://github.com/AllYarnsAreBeautiful/ayab-desktop): go to Tools \> Load AYAB Firmware.

## Development Environment

To set up a working development environment follow these steps:

 0. Clone the repository and update all submodules.

    Ubuntu:
    ```bash
    sudo apt install -y git
    git clone --recurse-submodules https://github.com/AllYarnsAreBeautiful/ayab-firmware.git ayab
    ```

 1. Install the [Arduino.mk](https://github.com/sudar/Arduino-Makefile) package and setup environment variables.

    Ubuntu:
    ```bash
    sudo apt install -y arduino-mk cmake
    export ARDMK_DIR=/usr/share/arduino
    ```

    MacOS:
    ```bash
    brew tap sudar/arduino-mk
    brew install arduino-mk
    ```
 Running `./build.sh` should work now.


 2. Install `clang-format`, `gcovr`, and update `gcc` to version 9.

    Ubuntu:
    ```bash
    sudo apt install -y clang-format gcovr \
                        gcc-9 g++-9 cpp-9 gcc-9-base gcc-10-base \
                        libgcc-9-dev libstdc++-9-dev
    ```

    MacOS:
    ```bash
    brew install clang-format gcovr gcc
    ```

 3. Install [pre-commit](https://pre-commit.com/) via pip and use it to install git hooks.
    ```
    pip3 install --user pre-commit
    pre-commit install
    ```

 4. Optionally create a pre-push hook.

 Add the following snippet in a file called `.git/hooks/pre-push`:
    ```
    #!/bin/bash
    set -e
    ./build.sh
    ./test/test.sh -c
    ```
