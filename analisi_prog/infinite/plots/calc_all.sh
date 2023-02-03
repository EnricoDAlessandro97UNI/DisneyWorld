#!/bin/bash
for FILE in *; 
do
	if [ "$FILE" == "calc_all.sh" ] || [ "$FILE" == "estimate" ] then
		echo $FILE; 
	fi
done