# importing the required module
import matplotlib.pyplot as plt
import sys
import numpy as np
  


filename = "stats/wait_block4_class1_py.dat"
filename1 = "stats/wait_block4_class2_py.dat"

# y axis values
y = []
statistics = open(filename)
for e in statistics.readlines():
	y.append(eval(e))
statistics.close()

y1 = []
statistics = open(filename1)
for e in statistics.readlines():
	y1.append(eval(e))
statistics.close()


# setting the x - coordinates
x = np.arange(0, 57600/100)

plt.axvline(x = 36000/100, color = 'y', linestyle="dashdot", label='Change Slot Time')
plt.axvline(x = 57600/100, color = 'black', linestyle="dashdot", label='End')



plt.title("Global wait")


# plotting the points 
plt.plot(x, y, label='Response time Class 1')
plt.plot(x, y1, label='Response time Class 2')
  
# naming the x axis
plt.xlabel('x - Samplings')
# naming the y axis
plt.ylabel('y - Response time (s)')


plt.legend(bbox_to_anchor = (1.0, 1), loc = 'lower center') 
# function to show the plot
plt.tight_layout()
#plt.show()
plt.savefig('wait_global_classes.png')