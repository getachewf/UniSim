#!/bin/bash
set +v off #echo off

for f in $(find ../../applications -executable ); do rm -f $f 2> /dev/null; done
 
for f in $(find ../../applications -name 'engine*.so.*' -or -name 'base*.so.*'); do rm -f $f; done

for f in $(find ../../library -name '*.so.*' -or -name '*.a'); do rm -f $f; done

for f in $(find ../../plugins -name '*.so.*' -or -name '*.a'); do rm -f $f; done

for f in $(find ../own_tools -executable ); do rm -f $f 2> /dev/null; done

echo "*"
echo "* UniSim project cleaned for all target files *"
echo "*"
