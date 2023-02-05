# importing the required module
import matplotlib.pyplot as plt
import sys
import numpy as np
  


filename = "stats/wait_global_py.dat"

# y axis values
y = []
statistics = open(filename)
for e in statistics.readlines():
	y.append(eval(e))
statistics.close()


# setting the x - coordinates
x = np.arange(0, 57600/100)

plt.axvline(x = 36000/100, color = 'y', linestyle="dashdot", label='Change Slot Time')
plt.axvline(x = 57600/100, color = 'black', linestyle="dashdot", label='End')

plt.axhline(y = 600, color = 'r', linestyle="dashdot", label='QoS')


plt.title("Global wait")


# plotting the points 
plt.plot(x, y, label='Response time')
  
# naming the x axis
plt.xlabel('x - Samplings')
# naming the y axis
plt.ylabel('y - Response time (s)')


plt.legend(bbox_to_anchor = (1.0, 1), loc = 'lower center') 
# function to show the plot
plt.tight_layout()
#plt.show()
plt.savefig('wait_global.png')