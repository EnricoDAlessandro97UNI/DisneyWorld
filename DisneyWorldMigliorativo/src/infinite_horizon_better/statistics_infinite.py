# stat.py: Calcola le medie dei tempi di risposta dal file del campionamento wait_global.dat
from statistics import mean
import sys

K = 128       # number of batches 
B = 1024      # sampling interval 
X = 32768     # number of rows in the file

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
    if (len(arguments) != 3):
        print("\nUsage: " + arguments[0] + " file_stastistics file_output\n")
        sys.exit()

    filename = arguments[1]
    foutput = arguments[2]
    # open the statistics file in read-only mode
    statistics = open(filename)
    # read the content of the file opened, readlines returns a list of strings
    content = statistics.readlines()
    print(len(content))
    # close opened file
    statistics.close()

    listAvg = calculateMean(content)

    # create new file
    newFile = open(foutput, "w")
    for line in listAvg:
        newFile.write(str(line) + "\n")
    newFile.close()