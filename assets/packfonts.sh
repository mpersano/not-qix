#!/bin/bash

PACKFONT=packfont
FONT=/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf

${PACKFONT} \
	-w 512 -h 256 \
	-s 20 \
	-g 2 \
	-t fonts \
	-d 2 -e 2 -S .6 -B 4 \
	-i ff0000-ffff00-ffffff -o 800000-808000-808080 \
	${FONT} \
	small \
	x20-x7e

${PACKFONT} \
	-w 512 -h 256 \
	-s 12 \
	-g 2 \
	-t fonts \
	-d 2 -e 2 -S .6 -B 4 \
	-i ff0000-ffff00-ffffff -o 800000-808000-808080 \
	${FONT} \
	tiny \
	x20-x7e
