# dogGo

Framework for making walking dogs competetive at your local animal shelter. All you need to do to start a fresh change are minimal electronic skills and basic knowledge of programming (not for long!)

## About the project

### The "arcade"

#### Overview

The user-facing part of **dogGo** is the embedded system that records time for every person walking the dogs.
For now the system can:
1. Create and store users' profiles (login + code)
2. Log time of walks with indicating the start and end of a walk

#### Electronics

![Project's cicuit](/assets/images/doggo_circuit.png "circuit")

Parts you will need:
1. Arduino board
2. microSD module
3. 16x2 I2C LCD display
4. 5 pushbuttons

The build is very simple, all the libraries in the code should be compatible with nearly every module. Power, soldering components and closure are dependent on your needs and skill level. The user input is designed to consist of 5 buttons with following functionality: 
* UP THE ALPHABET / NEW USER
* DOWN THE ALPHABET / LOG IN
* BACKSPACE
* CONFIRM CHARACTER
* GO TO NEXT PAGE

The current design of input grid:

![Doggo input grid](/assets/images/doggo_instruction.jpg "input proposition")

the system is not the best, but if you have an idea for improvement, share it!

#### Firmware

You can clone the repository and use the directory as an Arduino project in Arduino IDE and Arduino-CLI straight away. The Makefile works with Arduino-CLI for Arduino Uno with ATMEGA 328p. 

### Generating leaderboards

#### Overview

The script takes the csv file generated by the arcade and finds best times of all time then the data gets generated by TKinter and then converted to a .jpg file using Ghostscript. When running the script on your machine, you will need to patch the script, as paths are for now hard-coded.

#### Prequesities

* Python 3

#### Sample leaderboard

![Sample leaderboard](/assets/images/sample_leaderboard.png "sample leaderboard")

## Roadmap for the future

The project is very basic for now and will need some work for it to make it better and more reliable. \
The priorities for the project are: cheap price, simplicity and reliability.

### Embedded system
* displaying time of the walk after finishing run
* clean-up mode (deleting unfinished records)

### Leaderboard generation
* basic GUI for choosing and creating templates for the leaderboard
* organizing data from the Arduino and making leaderboards in different categories:
    * yearly
    * monthly
    * weekly
    * frequency
    * net time

## Contributing

The project is under GPL license, so you are free to contribute your ideas \
and fork the project.


