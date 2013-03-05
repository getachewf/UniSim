#!/bin/bash
set +v  #echo off

for D in ../../applications/*build-Desktop*
do 
  rm -Rf $D 
done

for D in ../../applications/unisim/*build-Desktop*
do 
  rm -Rf $D 
done

for D in ../../library/*build-Desktop*
do 
  rm -Rf $D 
done

for D in ../../plugins/*build-Desktop*
do 
  rm -Rf $D 
done

for D in ../../own_tools/*build-Desktop*
do 
  rm -Rf $D 
done

for D in ../../plugins/*/moc_*.*
do 
  rm -Rf $D 
done

