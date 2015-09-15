#!/bin/bash

convert -size 256x256 xc:none \
	-draw @powerup-outer.mvg \
	-resize 40 \
	png32:powerup-outer.png

convert -size 256x256 xc:none \
	-draw @powerup-inner.mvg \
	-resize 40 \
	png32:powerup-inner.png
