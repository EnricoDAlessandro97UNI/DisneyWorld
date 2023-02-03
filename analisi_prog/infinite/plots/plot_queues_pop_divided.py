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
statistics = open(path + '/avgqueue_block1_py.dat')
for e in statistics.readlines():
	y1.append(eval(e))
statistics.close()

y2 = []
statistics = open(path + '/avgqueue_block2_py.dat')
for e in statistics.readlines():
	y2.append(eval(e))
statistics.close()

y3 = []
statistics = open(path + '/avgqueue_block3_py.dat')
for e in statistics.readlines():
	y3.append(eval(e))
statistics.close()

y4 = []
statistics = open(path + '/avgqueue_block4_py.dat')
for e in statistics.readlines():
	y4.append(eval(e))
statistics.close()

y5 = []
statistics = open(path + '/avgqueue_block5_py.dat')
for e in statistics.readlines():
	y5.append(eval(e))
statistics.close()

x = np.arange(0, 128)



#plt.errorbar(x=x, y=y3, yerr=err_3, color="blue", capsize=3, linestyle="None", marker="s", markersize=7, mfc="blue", mec="blue")

#plt.errorbar(x=x, y=y4, yerr=err_4, color="green", capsize=3, linestyle="None", marker="s", markersize=7, mfc="green", mec="green")

#plt.errorbar(x=x, y=y5, yerr=err_5, color="gray", capsize=3, linestyle="None", marker="s", markersize=7, mfc="gray", mec="gray")


#plt.axhline(y = 122.05196292731779, color = 'b', linestyle="dashdot", label='Avg queue')
#plt.axhline(y = 142.05356292731778, color = 'r', linestyle="dashdot", label='Avg block')


fig, ax = plt.subplots(5)
 
# set data with subplots and plot
ax[0].plot(x, y1, c='r', label='Block1 queue population')
ax[1].plot(x, y2, c='b', label='Block2 queue population')
ax[2].plot(x, y3, c='g', label='Block3 queue population')
ax[3].plot(x, y4, c='y', label='Block4 queue population')
ax[4].plot(x, y5, c='gray', label='Block5 queue population')

ax[0].legend(bbox_to_anchor = (1.0, 1), loc = 'lower center') 
ax[1].legend(bbox_to_anchor = (1.0, 1), loc = 'lower center') 
ax[2].legend(bbox_to_anchor = (1.0, 1), loc = 'lower center') 
ax[3].legend(bbox_to_anchor = (1.0, 1), loc = 'lower center') 
ax[4].legend(bbox_to_anchor = (1.0, 1), loc = 'lower center') 



plt.show()