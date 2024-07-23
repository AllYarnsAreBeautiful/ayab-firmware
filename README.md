## AYAB - All Yarns Are Beautiful


_The goal of the AYAB project is to provide an alternative way to control the famous Brother KH-9xx range of knitting machines using a computer_

http://ayab-knitting.com


-----

# AYAB Firmware

This is the firmware for the Arduino-based hardware that is part of the AYAB project.

## Updating your AYAB firmware

In [ayab-desktop](https://github.com/AllYarnsAreBeautiful/ayab-desktop): go to Tools \> Load AYAB Firmware.

## Development Environment

### Compiling the firmware

To set up a working development environment follow these steps:

 1. Clone the repository and update all submodules.

    Ubuntu (>= 22.04):
    ```bash
    sudo apt install -y git
    git clone --recurse-submodules https://github.com/AllYarnsAreBeautiful/ayab-firmware.git ayab
    ```
    
    > **_NOTE:_**  If you checkout a non master branch you need to update submodules again
    ```bash
    git submodule update --init --recursive
    ```
    
 2. The AYAB firmware uses [PlatformIO](https://platform.io/) to build the binaries.
    Please [download the PlatformIO plugin](https://platformio.org/install/integration) for your favorite IDE, i.e. VSCode.
    Then, open the ayab-firmware project and hit Build and/or Upload to compile and upload to hardware.

### Enabling stack overflow detection

You can build a version of the firmware that will try to detect memory corruption due to stack overflow as soon as it happens.

 1. Open the `platformio.ini` file and uncomment the line that contains `-DENABLE_STACK_CANARY=1`

 2. Build and upload the firmware to your Arduino board

 3. Use the firmware as you would normally. If at any point the firmware stops responding to the AYAB desktop application, and you see the yellow LED flashing repeatedly, congratulations: you have hit a stack overflow condition. Please open an issue in this repository describing what you were doing when the problem occurred.

### Unit tests and code analysis

 1. Install the [Arduino.mk](https://github.com/sudar/Arduino-Makefile) package and setup environment variables.
    This is required to run the unit tests.

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
 Running `./test/test.sh` should work now.


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

 3. Install [pre-commit](https://pre-commit.com/) and use it to install git hooks.

    Ubuntu:
    ```bash
    sudo apt install -y pre-commit
    pre-commit install
    ```

    MacOS:
    ```bash
    pip3 install --user pre-commit
    pre-commit install
    ```

 4. Optionally create a pre-push hook:

    ```bash
    cat << SNIPPET >> .git/hooks/pre-push
    #!/bin/bash
    set -e
    pio run
    ./test/test.sh -c
    SNIPPET
    chmod +x .git/hooks/pre-push
    ```
## CI/CD on GitHub

### Triggering a new build

A new build is triggered when a new tag is created, either starting with

* v (i.e. v1.0.0), or
* test (i.e. test230517)

Convention for the test-tag is to suffix the current date in the YYMMdd format. If there is already an existing test build for a single day, attach a letter.
The test tags and releases will be manually removed from time for a better overview.

The tag can be pushed from your local environment, or via the ["Draft a new Release"](https://github.com/AllYarnsAreBeautiful/ayab-desktop/releases/new) button on the GitHub website.
