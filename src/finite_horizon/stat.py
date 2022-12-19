# stat.py: Calcola le medie dei tempi di risposta dal file del campionamento wait_global.dat
from statistics import mean
import sys

REP = 256           # number of simulation repetitions
STOP = 57600.0      # maximum simulation time
SAMPLING = 100.0    # sampling interval


def calculateMean(content):

    # Calculate interval statistics
    interval = int(STOP/SAMPLING)
    index = 0

    inputList = []
    listAvg = []

    for i in range(interval):
        index = i
        inputList.clear()
        for j in range(REP):
            inputList.append(float(content[index]))
            index += interval
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
    # close opened file
    statistics.close()

    listAvg = calculateMean(content)

    # create new file
    newFile = open("wait_global_py.dat", "w")
    for line in listAvg:
        newFile.write(str(line) + "\n")
    newFile.close()