#!/bin/bash
touch "f1_statistics.txt"
touch "f2_statistics.txt"

cd statsF1
for FILE in *; do
	
	echo $FILE >> "../f1_statistics.txt"
	(.././estimate < $FILE) >> "../f1_statistics.txt"
	echo "---------------------" >> "../f1_statistics.txt"
	
done


cd ../statsF2
for FILE in *; do
	
	echo $FILE >> "../f2_statistics.txt"
	(.././estimate < $FILE) >> "../f2_statistics.txt"
	echo "---------------------" >> "../f2_statistics.txt"
	
done

cd ../theo_script
python3 msq_stat.py > ../verifica.txt
python3 multi_prio.py >> ../verifica.txt

cd ..
python3 plot_blocks_pop_F1.py
python3 plot_blocks_pop_F2.py
python3 plot_queues_pop_F1.py
python3 plot_queues_pop_F2.py
python3 plot_wait_F1.py
python3 plot_wait_F2.py