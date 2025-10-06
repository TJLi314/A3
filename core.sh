#!/bin/bash
clear
cd /storage-home/s/sb121/comp530/A3/Build
rm core*
echo 6 | ~/scons/bin/scons-3.1.2
bin/sortUnitTest
gdb bin/sortUnitTest core*