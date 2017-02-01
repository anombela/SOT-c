#!/bin/bash

var=$1
current_dir=$(pwd)

if ! [ -n "$var" ];
then
	exit
fi

for i in $(ls $current_dir)
do
    var2=$(cat $i 2>/dev/null | grep ^$var ) #salida de errores a estandar
    if [ -n "$var2" ]; 
    then
    	name2=$i.$var
    	mv $i $name2
    fi
done
exit
