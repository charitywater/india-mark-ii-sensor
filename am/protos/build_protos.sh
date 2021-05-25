#!/bin/sh

echo
echo "Removing old protobuf files..."
rm *.pb.c
rm *.pb.h

echo
echo "Generating messages.pb..."
protoc -omessages.pb -I. -I/nanopb/generator/proto messages.proto

echo "Generating C and H files..."
python /nanopb/generator/nanopb_generator.py messages.pb

