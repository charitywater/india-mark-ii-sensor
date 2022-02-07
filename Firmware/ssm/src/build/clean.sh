#!/bin/sh

find ../ -name "*.d_raw" -type f -delete
find ../ -name "*.obj" -type f -delete
find ../ -name "TI*" -type f -delete
find ../ -name "*.d" -type f -delete
find ../ -name "*.pp" -type f -delete
find ../ -name "*.rl" -type f -delete
rm ssm.map
rm ssm.out
rm src_linkInfo.xml
