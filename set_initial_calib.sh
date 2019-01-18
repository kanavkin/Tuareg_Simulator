#!/bin/bash

#run ./set_initial_calib.sh !!!

DEV="/dev/ttyACM0"

#data order: big endian!
# \x57 ADDR LSB MSB

#change to calibration Page
printf "%b" "\x50\x3c" > $DEV

#IAT

# x: 0 800 1600 2000 3000 4000
# y: 0 10  20   30   40   50

# x: 0 320 640 7d0 bb8 fa0
# y: 0 a   14  1e  28  32

#x
printf "%b" "\x57\x00\x00\x00" > $DEV
printf "%b" "\x57\x01\x03\x20" > $DEV
printf "%b" "\x57\x02\x06\x40" > $DEV
printf "%b" "\x57\x03\x07\xd0" > $DEV
printf "%b" "\x57\x04\x0b\xb8" > $DEV
printf "%b" "\x57\x05\x0f\xa0" > $DEV

#y
printf "%b" "\x57\x06\x00\x00" > $DEV
printf "%b" "\x57\x07\x00\x0a" > $DEV
printf "%b" "\x57\x08\x00\x14" > $DEV
printf "%b" "\x57\x09\x00\x1e" > $DEV
printf "%b" "\x57\x0a\x00\x28" > $DEV
printf "%b" "\x57\x0b\x00\x32" > $DEV

#CLT

# x: 0 800 1600 2000 3000 4000
# y: 0 40  80   90   120  160

# x: 0 320 640 7d0 bb8 fa0
# y: 0 28  50  5a  78  a0

#x
printf "%b" "\x57\x0c\x00\x00" > $DEV
printf "%b" "\x57\x0d\x03\x20" > $DEV
printf "%b" "\x57\x0e\x06\x40" > $DEV
printf "%b" "\x57\x0f\x07\xd0" > $DEV
printf "%b" "\x57\x10\x0b\xb8" > $DEV
printf "%b" "\x57\x11\x0f\xa0" > $DEV

#y
printf "%b" "\x57\x12\x00\x00" > $DEV
printf "%b" "\x57\x13\x00\x28" > $DEV
printf "%b" "\x57\x14\x00\x50" > $DEV
printf "%b" "\x57\x15\x00\x5a" > $DEV
printf "%b" "\x57\x16\x00\x78" > $DEV
printf "%b" "\x57\x17\x00\xa0" > $DEV


#TPS

# x: 0 1800 1600 2000 3000 4000
# y: 0 20  40   60   80   100

# x: 0 320 640 7d0 bb8 fa0
# y: 0 14  28  3c  50  64

#x
printf "%b" "\x57\x18\x00\x00" > $DEV
printf "%b" "\x57\x19\x03\x20" > $DEV
printf "%b" "\x57\x1a\x06\x40" > $DEV
printf "%b" "\x57\x1b\x07\xd0" > $DEV
printf "%b" "\x57\x1c\x0b\xb8" > $DEV
printf "%b" "\x57\x1d\x0f\xa0" > $DEV

#y
printf "%b" "\x57\x1e\x00\x00" > $DEV
printf "%b" "\x57\x1f\x00\x14" > $DEV
printf "%b" "\x57\x20\x00\x28" > $DEV
printf "%b" "\x57\x21\x00\x3c" > $DEV
printf "%b" "\x57\x22\x00\x50" > $DEV
printf "%b" "\x57\x23\x00\x64" > $DEV


#MAP
# M 379 = 0x17b
# N 45224 = 0xb0a8
# L 147 = 0x93
printf "%b" "\x57\x24\x01\x7b" > $DEV
printf "%b" "\x57\x25\xb0\xa8" > $DEV
printf "%b" "\x57\x26\x00\x93" > $DEV

#BARO M N L
printf "%b" "\x57\x27\x01\x7b" > $DEV
printf "%b" "\x57\x28\xb0\xa8" > $DEV
printf "%b" "\x57\x29\x00\x93" > $DEV

#O2
# M 379 = 0x17b
# N 45224 = 0xb0a8
# L 147 = 0x93
printf "%b" "\x57\x2A\x01\x7b" > $DEV
printf "%b" "\x57\x2B\xb0\xa8" > $DEV
printf "%b" "\x57\x2C\x00\x93" > $DEV

#VBAT
# M 4095 = 0xfff
# L 16000 = 0x3e80
printf "%b" "\x57\x2D\x0f\xff" > $DEV
printf "%b" "\x57\x2E\x3e\x80" > $DEV

#request page display
printf "%b" "\x4C" > $DEV
