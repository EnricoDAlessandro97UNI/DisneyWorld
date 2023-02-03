import matplotlib.pyplot as plt
import numpy as np
import sys

arguments = sys.argv
if (len(arguments) != 2):
    print("\nUsage: " + arguments[0] + " file_stastistics\n")
    sys.exit()

path = arguments[1]

# y axis values
y1 = []
statistics = open(path + '/avgnode_block1_py.dat')
for e in statistics.readlines():
	y1.append(eval(e))
statistics.close()

y2 = []
statistics = open(path + '/avgnode_block2_py.dat')
for e in statistics.readlines():
	y2.append(eval(e))
statistics.close()

y3 = []
statistics = open(path + '/avgnode_block3_py.dat')
for e in statistics.readlines():
	y3.append(eval(e))
statistics.close()

y4 = []
statistics = open(path + '/avgnode_block4_py.dat')
for e in statistics.readlines():
	y4.append(eval(e))
statistics.close()

y5 = []
statistics = open(path + '/avgnode_block5_py.dat')
for e in statistics.readlines():
	y5.append(eval(e))
statistics.close()

x = np.arange(0, 57600/100)



#plt.errorbar(x=x, y=y3, yerr=err_3, color="blue", capsize=3, linestyle="None", marker="s", markersize=7, mfc="blue", mec="blue")

#plt.errorbar(x=x, y=y4, yerr=err_4, color="green", capsize=3, linestyle="None", marker="s", markersize=7, mfc="green", mec="green")

#plt.errorbar(x=x, y=y5, yerr=err_5, color="gray", capsize=3, linestyle="None", marker="s", markersize=7, mfc="gray", mec="gray")


#plt.axhline(y = 122.05196292731779, color = 'b', linestyle="dashdot", label='Avg queue')
#plt.axhline(y = 142.05356292731778, color = 'r', linestyle="dashdot", label='Avg block')

plt.axvline(x = 36000/100, color = 'y', linestyle="dashdot", label='Change Slot Time (10h)')
plt.axvline(x = 57600/100, color = 'black', linestyle="dashdot", label='End (16h)')




plt.plot(x, y1, label='Block1 population', color = 'r')
plt.plot(x, y2, label='Block2 population', color = 'g')
plt.plot(x, y3, label='Block3 population', color = 'b')
plt.plot(x, y4, label='Block4 population', color = 'violet')
plt.plot(x, y5, label='Block5 population', color = 'gray')
plt.legend(bbox_to_anchor = (1.0, 1), loc = 'lower center') 

# naming the x axis
plt.xlabel('x - Samplings')
# naming the y axis
plt.ylabel('y - Node population')

plt.show()


