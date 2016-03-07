#!/bin/bash
for i in *.geom *.h *.hpp *.cpp *.c *.frag *.glsl *.vert; do 
	echo "$i"
	DIR=$(cat /mnt/win-C/Shlomi/git/simulation/Simulation/Simulation.vcxproj.filters | grep -i -n1 "$i" | tail -n1 | sed -e 's/.*<Filter>\(.*\)<\/Filter>.*/\1/g')
	DIR=$(echo "$DIR" | sed 's/\\/\//g')
	echo $DIR
	if [ ! -z "$DIR" ]; then 
		mkdir -p "$DIR"
		mv $i "$DIR/"
	fi
done
