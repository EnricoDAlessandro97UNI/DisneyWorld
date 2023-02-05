# importing the required module
import matplotlib.pyplot as plt
import sys
import numpy as np
 

filename = "statsF1/delay_block4_class1_py.dat"
filename1 = "statsF1/delay_block4_class2_py.dat"
# y axis values
y = []
statistics = open(filename)
for e in statistics.readlines():
	y.append(eval(e)/60)
statistics.close()

y1 = []
statistics = open(filename1)
for e in statistics.readlines():
	y1.append(eval(e)/60)
statistics.close()

# corresponding y axis values
#x = []
#for i in range (0, 128):
#	x.append(i)
 
# setting the x - coordinates
x = np.arange(0, 128)

plt.axhline(y = 2.009937825544866/60, color = 'b', linestyle="dashdot", label='Theorical Mean Class1')
plt.axhline(y = 37.903430979853354/60, color = 'g', linestyle="dashdot", label='Theorical Mean Class2')


plt.title("Classes delay")
plt.legend(bbox_to_anchor = (1.0, 1), loc = 'lower center') 

# plotting the points 
plt.plot(x, y, color = 'b')
plt.plot(x, y1, color = 'g')
  
# naming the x axis
plt.xlabel('x - batches')
# naming the y axis
plt.ylabel('y - Response time (m)')

plt.tight_layout()
  
# function to show the plot
#plt.show()
plt.savefig('class1_delay_F1.png')
