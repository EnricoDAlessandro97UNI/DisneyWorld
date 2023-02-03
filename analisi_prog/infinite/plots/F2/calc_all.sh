#!/bin/bash
touch "f2_statistics.txt"
for FILE in *; do
	if [ "$FILE" != "calc_all.sh" ] && [ "$FILE" != "estimate" ] && [ "$FILE" != "f2_statistics.txt" ]; then
		echo $FILE >> "f2_statistics.txt"
		(./estimate < $FILE) >> "f2_statistics.txt"
		echo "---------------------" >> "f2_statistics.txt"
	fi
done