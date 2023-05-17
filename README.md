# space-invaders-8080-emulator
An Intel 8080 emulator to run Space Invaders. OSU Senior Capstone Project - Spring 2023

## Installation
- Run "make" to build both the disassembler, the emulator, and the shell
- Run "make disassembler_8080" to build just the disassembler
- Run "make shell" to build just the emulator and its shell
- Run "make test" to build and run the tests executable
- Run "make clean" to remove all object files and executables

## Running the Disassembler
- After building the disassembler, run `./disassembler_8080 file_path` to disassemble the ROM with the ROM file path as an argument.

## Running the Emulator
- After building the emulator and its shell, run `./shell -[options] file_path` to run the emulator with the ROM file path as an argument.
- Options:
  - -p to print instructions as they are executed
  - -d to print cpu state before and after instructions are executed

[![cpp-linter](https://github.com/cpp-linter/cpp-linter-action/actions/workflows/cpp-linter.yml/badge.svg)](https://github.com/cpp-linter/cpp-linter-action/actions/workflows/cpp-linter.yml)
