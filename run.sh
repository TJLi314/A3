#!/bin/bash
clear
cd /storage-home/t/tl107/comp432/A3/Build
echo 6 | ~/scons/bin/scons-3.1.2
if [ -n "$1" ]; then
    bin/sortUnitTest > "../$1"
else
    bin/sortUnitTest
fi