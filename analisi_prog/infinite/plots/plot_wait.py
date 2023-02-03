# importing the required module
import matplotlib.pyplot as plt
import sys
import numpy as np
  



arguments = sys.argv
if (len(arguments) != 2):
    print("\nUsage: " + arguments[0] + " file_stastistics\n")
    sys.exit()

filename = arguments[1]

# y axis values
y = []
statistics = open(filename)
for e in statistics.readlines():
	y.append(eval(e)/60)
statistics.close()

# corresponding y axis values
#x = []
#for i in range (0, 128):
#	x.append(i)
 
# setting the x - coordinates
x = np.arange(0, 128)

plt.axhline(y = 494.65456/60, color = 'b', linestyle="dashdot", label='Theorical Mean')
plt.axhline(y = 600/60, color = 'r', linestyle="dashdot", label='QoS')


plt.legend(bbox_to_anchor = (1.0, 1), loc = 'lower center') 

# plotting the points 
plt.plot(x, y)
  
# naming the x axis
plt.xlabel('x - batches')
# naming the y axis
plt.ylabel('y - Response time (m)')


  
# function to show the plot
plt.show()