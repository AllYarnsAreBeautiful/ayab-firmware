## AYAB - All Yarns Are Beautiful


_The goal of the AYAB project is to provide an alternative way to control the famous Brother KH-9xx range of knitting machines using a computer_

http://ayab-knitting.com


-----

# AYAB Firmware

This is the firmware for the Arduino-based hardware that is part of the AYAB project.

## Updating your AYAB firmware

Clone this repository to a directory called ayab.
`git clone --recurse-submodules https://github.com/AllYarnsAreBeautiful/ayab-firmware.git ayab`

Download the [Arduino IDE](https://www.arduino.cc/en/Main/Software) if you haven't before,
in which case the [Arduino getting started guide](https://www.arduino.cc/en/Guide/ArduinoUno) is recommended.

Open ayab.ino in Arduino IDE.

Press 'verify' to check that your setup is in good shape.

Press 'upload' to update your AYAB hardware with new firmware.

## Development Environment

To set up a working development environment follow these steps:

 0. Clone the repository and update all submodules

    `git clone --recurse-submodules https://github.com/AllYarnsAreBeautiful/ayab-firmware.git ayab`

 1. Install the [Arduino.mk](https://github.com/sudar/Arduino-Makefile) package and setup environment variables

    Ubuntu:
    ```bash
    sudo apt install arduino-mk
    export ARDMK_DIR=/usr/share/arduino
    ```
    Running ./build.sh should work now.

 2. Install clang-format and gcovr

    Ubuntu:
    ```bash
    sudo apt install clang-format gcovr
    ```

 3. Install [pre-commit](https://pre-commit.com/) via pip and use it to install git hooks

    ```bash
    pip3 install --user pre-commit
    pre-commit install
    ```

 4. Optionally create a pre-push hook

    Add the following snippet in a file called .git/hooks/pre-push
    ```bash
    #!/bin/bash
    set -e

    ./build.sh
    ./test/test.sh -c
    ```

