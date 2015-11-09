#!/bin/bash

width=256
height=256
radius=100
num_frames=64

shield() {
	frame=$1
	x_radius=$( echo "c(${frame}*2.*3.14159265/${num_frames})*${radius}" | bc -l )

	if [ $((frame % $((num_frames/2)))) -lt $((num_frames/4)) ]; then
		sweep_inner=1
	else
		sweep_inner=0
	fi

	if [ ${frame} -lt $((num_frames/2)) ]; then
		sweep_outer=1
	else
		sweep_outer=0
	fi

	cat<<EOF
translate $( echo "$width/2" | bc -l ),$( echo "$height/2" | bc -l )

fill matte
stroke white
stroke-width 1
circle 0,0 ${radius},0

fill SteelBlue
stroke-width 0
stroke-opacity 0
path 'M 0,${radius} A ${radius},${radius} 0 0,${sweep_outer} 0,-${radius} A ${x_radius},${radius} 0 0,${sweep_inner} 0,${radius}'
EOF
}

for frame in $( seq 0 $(($num_frames - 1)) ); do
	echo "generating frame $frame..."

	a0=$( echo "${frame}*360/${num_frames}" | bc -l )
	a1=$( echo "$a0 + 90" | bc -l )

	convert -size ${width}x${height} \
		\( \
  		  \( xc:none -draw "translate 128,128 fill #ffc0c0 polygon -72,0 0,-72 0,0 fill #800000 polygon 72,0 0,72 0,0" \) \
  		  \( xc:none -draw "translate 128,128 rotate $a0 arc -40,-12 40,12 0,360" -channel Alpha -blur 0x16 -evaluate multiply .75 \
  		    \( +clone -fill white -draw 'color 0,0 reset' \) \
  		    -compose ATop -composite \) \
		  -compose ATop -composite \) \
		\( \
  		  \( xc:none -draw "translate 128,128 fill #ff0000 polygon 72,0 0,-72 0,0 polygon -72,0 0,72 0,0" \) \
		  \( xc:none -draw "translate 128,128 rotate $a1 arc -60,-20 60,20 0,360" -channel Alpha -blur 0x16 -evaluate multiply .75 \
  		    \( +clone -fill white -draw 'color 0,0 reset' \) \
  		    -compose ATop -composite \) \
		  -compose ATop -composite \) \
		-compose Blend -define compose:args=100,100 -composite -resize 40 png32:player-core-$( printf "%02d" $frame ).png
done

for frame in $( seq 0 $(($num_frames - 1)) ); do
	echo "generating frame $frame..."

	a0=$( echo "${frame}*360/${num_frames}" | bc -l )
	a1=$( echo "$a0 + 90" | bc -l )

	convert -size ${width}x${height} \
		player-core-$( printf "%02d" $frame ).png \
		\( -background matte mvg:<(shield $frame) \
		   \( xc:none -draw 'translate 80,80 circle 0,0 40,0' -channel Alpha -blur 0x12 \
	   	      \( +clone -fill white -draw 'color 0,0 reset' \) -compose ATop -composite \) \
		   -compose ATop -composite -resize 40 \) \
		-compose Blend -define compose:args=70,100 -composite png32:player-shield-$( printf "%02d" $frame ).png
done
