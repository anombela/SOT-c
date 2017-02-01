#!/bin/bash
if [ $# != 4 ];
then
	echo "Introducir 4 ficheros"
	exit
fi
for i in $(cat $* | awk '{ print $1}' | sort -u)  #cada pasado un nombre
do
   	var=$(grep $i $* | awk '{ print $2}' | grep -c 'si')
   	echo -n $i
   	if [ '2' -gt $var ]; #2 mas grande que $var comprarndolo como string(puedn ser 0..4)
	then
		echo " no"
	else
		echo " si"
	fi
done
exit