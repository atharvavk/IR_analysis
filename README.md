# Intermedite Code Representation Analysis

<br/>

## Features:

Create IR (Intermediate Representation) from C source code.<br/>
Create Control Flow Graph (CFG).<br/>
Perform Reaching Definition Analysis. (GEN/KILL and IN/OUT)<br/>
Find Dominators from CFG.<br/>
Find Natural Loops in the program.<br/>

## Tech Stack used:

LLVM<br/>
Clang<br/>

## Usage:

Install LLVM and Clang on your system (LLVM-11 if possible).<br/>
Run commands.sh file<br/>
This will create a shared object and intermediate representation of hello.c file.<br/>
Last command in the file will run a function pass on the given code.<br/>

## Report:
Follow [report](./report) for more information
