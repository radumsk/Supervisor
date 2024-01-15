#!/bin/sh

# Make lib folder if it doesn't exist
mkdir -p ../test/lib

# Move the library to the test folder
cp libSupervisorLibrary.dylib ../test/lib

# Move the library header to the test folder
cp supervisor.h ../test/lib

cp supervisor.c ../test/lib

