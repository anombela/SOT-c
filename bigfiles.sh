#!/bin/bash

var=$1
if ! [ -n "$var" ]; 
then
   	var=1
fi

allfiles=$(du -a | sort -nr |  awk '{ print $2}')

for file in $allfiles;
do

	if $(test -f "$file");
	then
		du $file 
	fi
	
done | head -$var | awk '{ print $2,$1}'

exit
