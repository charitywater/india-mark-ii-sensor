#!/bin/bash

# Confirm script has 1 arguments
if [ $# -ne 2 ] ; then
    echo "Usage: ./decode_proto.sh <status/gps/sensor> <proto_output>"
    exit 1
fi

# takes in base64 and converts to hex, then protobuf
echo $2 | base64 -d | hexdump -v -e '/1 "%02x" ' | xxd -r -p | protoc -I. -I/nanopb/generator/proto --decode $1 messages.proto
