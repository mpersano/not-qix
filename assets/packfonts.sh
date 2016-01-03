#!/bin/bash

PACKFONT=packfont
FONT=/usr/share/fonts/truetype/Coda-Heavy.ttf

${PACKFONT} \
	-w 512 -h 512 \
	-s 32 \
	-g 3 \
	-t fonts \
	-i ffff0000-ffffc0c0 \
	-o ffffffff \
	${FONT} \
	hud-big \
	x20-x7e

${PACKFONT} \
	-w 512 -h 512 \
	-s 26 \
	-g 3 \
	-t fonts \
	-i ffff0000-ffffc0c0 \
	-o ffffffff \
	${FONT} \
	hud-small \
	x20-x7e

${PACKFONT} \
	-w 512 -h 512 \
	-s 32 \
	-g 2 \
	-t fonts \
	-d 3 -e 3 -S .6 -B 6 \
	-i ffff0000-ffffff00-ffffffff \
	-o ff800000-ff808000-ff808080 \
	${FONT} \
	powerup \
	x20-x7e

${PACKFONT} \
	-w 512 -h 512 \
	-s 68 \
	-g 3 \
	-t fonts \
	-d 3 -e 3 -S .6 -B 3 \
	-i ffffff00 \
	-o ffff0000 \
	${FONT} \
	title-yellow \
	x41-x5a x30-x39 x20 x21

${PACKFONT} \
	-w 512 -h 512 \
	-s 68 \
	-g 3 \
	-t fonts \
	-d 3 -e 3 -S .6 -B 3 \
	-i ffff0000 \
	-o ffffffff \
	${FONT} \
	title-red \
	x41-x5a x30-x39 x20 x21

${PACKFONT} \
	-w 512 -h 512 \
	-s 68 \
	-g 3 \
	-t fonts \
	-d 3 -e 3 -S .6 -B 3 \
	-i 00ffffff \
	-o ffffffff \
	${FONT} \
	title-border \
	x41-x5a x30-x39 x20 x21
