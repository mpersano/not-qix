#!/bin/bash

PACKFONT=packfont
FONT=/usr/share/fonts/truetype/Coda-Heavy.ttf

${PACKFONT} \
	-w 512 -h 256 \
	-s 20 \
	-g 2 \
	-t fonts \
	-i ffffffff \
	-o ff404040 \
	${FONT} \
	hud-big \
	x20-x7e

${PACKFONT} \
	-w 512 -h 256 \
	-s 16 \
	-g 2 \
	-t fonts \
	-i ffffffff \
	-o ff404040 \
	${FONT} \
	hud-small \
	x20-x7e

${PACKFONT} \
	-w 512 -h 256 \
	-s 20 \
	-g 2 \
	-t fonts \
	-d 2 -e 2 -S .6 -B 4 \
	-i ffff0000-ffffff00-ffffffff \
	-o ff800000-ff808000-ff808080 \
	${FONT} \
	powerup \
	x20-x7e

${PACKFONT} \
	-w 512 -h 256 \
	-s 42 \
	-g 2 \
	-t fonts \
	-d 2 -e 2 -S .6 -B 2 \
	-i ffffff00 \
	-o ffff0000 \
	${FONT} \
	title-yellow \
	x41-x5a x30-x39 x20 x21

${PACKFONT} \
	-w 512 -h 256 \
	-s 42 \
	-g 2 \
	-t fonts \
	-d 2 -e 2 -S .6 -B 2 \
	-i ffff0000 \
	-o ffffffff \
	${FONT} \
	title-red \
	x41-x5a x30-x39 x20 x21

${PACKFONT} \
	-w 512 -h 256 \
	-s 42 \
	-g 2 \
	-t fonts \
	-d 2 -e 2 -S .6 -B 2 \
	-i 00ffffff \
	-o ffffffff \
	${FONT} \
	title-border \
	x41-x5a x30-x39 x20 x21
