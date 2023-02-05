import matplotlib.pyplot as plt
import numpy as np
import sys

path = "statsF2"

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

plt.axhline(y = 7.9621121960823675 , color = 'r', linestyle="dashdot")
plt.axhline(y = 0.5827729356619695, color = 'g', linestyle="dashdot")
plt.axhline(y = 0.45039723118254815, color = 'b', linestyle="dashdot")
plt.axhline(y = 6.48066945093441, color = 'violet', linestyle="dashdot")
plt.axhline(y = 3.100083146896389, color = 'gray', linestyle="dashdot")


plt.title("Queues population")


plt.plot(x, y1, label='Block1 queue population', color = 'r')
plt.plot(x, y2, label='Block2 queue population', color = 'g')
plt.plot(x, y3, label='Block3 queue population', color = 'b')
plt.plot(x, y4, label='Block4 queue population', color = 'violet')
plt.plot(x, y5, label='Block5 queue population', color = 'gray')
plt.legend(bbox_to_anchor = (1.0, 1), loc = 'lower center') 

# naming the x axis
plt.xlabel('x - batches')
# naming the y axis
plt.ylabel('y - Queue population')
plt.tight_layout()
#plt.show()
plt.savefig('queue_pop_F2.png')


