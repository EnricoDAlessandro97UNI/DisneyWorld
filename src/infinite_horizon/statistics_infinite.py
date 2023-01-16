# stat.py: Calcola le medie dei tempi di risposta dal file del campionamento wait_global.dat
from statistics import mean
import sys

K = 256       # number of batches 
B = 8192      # sampling interval 
X = 65536
N = 131072    # number of jobs to process (N=K*B) 


def calculateMean(content):

    # Calculate interval statistics
    index = 0

    inputList = []
    listAvg = []

    for i in range(K):
        inputList.clear()
        j=i
        while (j < X):
            inputList.append(float(content[j]))
            j += K
        listAvg.append(mean(inputList))

    return listAvg


if __name__ == "__main__":
    
    arguments = sys.argv
    if (len(arguments) != 2):
        print("\nUsage: " + arguments[0] + " file_stastistics\n")
        sys.exit()

    filename = arguments[1]
    # open the statistics file in read-only mode
    statistics = open(filename)
    # read the content of the file opened, readlines returns a list of strings
    content = statistics.readlines()
    print(len(content))
    # close opened file
    statistics.close()

    listAvg = calculateMean(content)

    # create new file
    newFile = open("wait_global_py.dat", "w")
    for line in listAvg:
        newFile.write(str(line) + "\n")
    newFile.close()