#!/bin/bash

#
# powerups
#

convert -size 256x256 xc:none \
	-draw @powerup-outer.mvg \
	-resize 40 \
	png32:powerup-outer.png

convert -size 256x256 xc:none \
	-draw @powerup-inner.mvg \
	-resize 40 \
	png32:powerup-inner.png

#
# lives left
#

convert -size 256x256 xc:none \
	-draw @lives-left-circle.mvg \
	png32:lives-left-circle.png

convert -size 256x256 xc:none \
	-draw @lives-left-arrow.mvg \
	png32:lives-left-arrow.png
