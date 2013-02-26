#!/bin/bash

for (( i=0; i<9; i++)); do
	stty -F /dev/ttyUSB$i 19200
done
