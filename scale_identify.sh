#!/bin/bash
# RUN AS ROOT

while [ 1 ]; do
	for (( i=0; i<5; i++)); do
		echo -ne "SCALE $i\t"
		cat /dev/ttyUSB$i  | head -2 | tail -1
		echo ""

	done
	sleep 0.1
	clear
done