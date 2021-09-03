####

#Make sure LLVM and Clang is installed on your system and they are working fine

###

#Converts the given C code to LLVM Bitcode and generates human readable Intermediate representation (IR) in .ll file
clang -O0 -emit-llvm -fno-discard-value-names -S hello.c

#Creates optimizer (Function pass) from given CPP file and makes executable shared object for he same
clang -fPIC -g3 -shared -o sharedObject.so demo.cpp

#runs the optimizer on given IR
opt -load sharedObject.so --rda hello.ll
