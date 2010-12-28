#!/usr/bin/sed

s/^\(\[1\]\) [0-9]*/\1 PID/;
s/Killing process [0-9]*/Killing process PID/;

