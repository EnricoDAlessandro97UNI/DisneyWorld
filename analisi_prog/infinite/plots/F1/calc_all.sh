#!/bin/bash
touch "f1_statistics.txt"
for FILE in *; do
	if [ "$FILE" != "calc_all.sh" ] && [ "$FILE" != "estimate" ] && [ "$FILE" != "f1_statistics.txt" ]; then
		echo $FILE >> "f1_statistics.txt"
		(./estimate < $FILE) >> "f1_statistics.txt"
		echo "---------------------" >> "f1_statistics.txt"
	fi
done