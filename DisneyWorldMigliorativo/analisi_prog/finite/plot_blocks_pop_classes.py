import matplotlib.pyplot as plt
import numpy as np
import sys

path = "stats"

# y axis values
y1 = []
statistics = open(path + '/avgnode_block4_class1_py.dat')
for e in statistics.readlines():
	y1.append(eval(e))
statistics.close()

y2 = []
statistics = open(path + '/avgnode_block4_class2_py.dat')
for e in statistics.readlines():
	y2.append(eval(e))
statistics.close()


x = np.arange(0, 57600/100)



#plt.errorbar(x=x, y=y3, yerr=err_3, color="blue", capsize=3, linestyle="None", marker="s", markersize=7, mfc="blue", mec="blue")

#plt.errorbar(x=x, y=y4, yerr=err_4, color="green", capsize=3, linestyle="None", marker="s", markersize=7, mfc="green", mec="green")

#plt.errorbar(x=x, y=y5, yerr=err_5, color="gray", capsize=3, linestyle="None", marker="s", markersize=7, mfc="gray", mec="gray")


#plt.axhline(y = 122.05196292731779, color = 'b', linestyle="dashdot", label='Avg queue')
#plt.axhline(y = 142.05356292731778, color = 'r', linestyle="dashdot", label='Avg block')

plt.axvline(x = 36000/100, color = 'y', linestyle="dashdot", label='Change Slot Time (10h)')
plt.axvline(x = 57600/100, color = 'black', linestyle="dashdot", label='End (16h)')

plt.title("Classes population")


plt.plot(x, y1, label='Class 1 population', color = 'r')
plt.plot(x, y2, label='Class 2 population', color = 'g')

plt.legend(bbox_to_anchor = (1.0, 1), loc = 'lower center') 

# naming the x axis
plt.xlabel('x - Samplings')
# naming the y axis
plt.ylabel('y - Node population')

plt.tight_layout()
#plt.show()
plt.savefig('classes_node_pop.png')

