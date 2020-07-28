# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html)
(at least since version 1.0.0).

## 1.0.0 / Unreleased

* Migrate to generic firmware from machine-specific versions
* Change libraries to submodules
* Add unit tests that can run in the absence of the hardware
* Add GPLv3 license for the repository, and LGPLv2.1 for the SerialCommand library
* Add development environment documentation to README
* Add firmware update instructions to README
* Add CHANGELOG.md
* Add automatic formatting using clang-format

## 0.95 / 2018-12-06

* Reduce the distance the carriage must travel to the left side before turning

## 0.90 / 2017-09-14

* Change I2C library to automatically detect MCP23008 and PCF8574 hardware
* Change double beep to be more distinct from single beep
* Fix bug in which final row is selected twice

## 0.80 / 2017-04-16

