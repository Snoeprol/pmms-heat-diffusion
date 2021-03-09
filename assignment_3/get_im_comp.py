import numpy as np

data_array = []
data_array_means = []
i_array = []

# Structure

# data_array[c] accesses the core.
# data_array[c][i] accesses the grid.
# data_array[x][i] accesses the i-th method of the x-th i (lol).
dat = np.zeros((2, 6, 5))
dat_stds = np.zeros((2, 6))
dat_means = np.zeros((2, 6))
with open('data_img_comp.txt') as f:
    #print(data_array)   
    img_bin_count = -1
    lines = f.readlines()
    for line in lines:
        if "image" in line:
            img_bin_count += 1
        if not "image" in line:
            split_line = line.split(",")
            p = dat[img_bin_count][int(split_line[0])]
            for i, val in enumerate(p):
                if val == 0:
                    dat[img_bin_count][int(split_line[0])][i] = float(split_line[1])

#print(data_array)
#print(i_array)  
#print(dat)      


for i, image in enumerate(dat):
    for j, method in enumerate(image):
        dat_means[i][j] = np.mean(method)

print(dat_means)
bars = dat_means
############################################################################################################################################# START PLOT
# set width of bar
barWidth = 0.08

import numpy as np
import matplotlib.pyplot as plt
# Cores grids methods
bars1 = bars[1,5]/bars[:, 0]
bars2 = bars[1,5]/bars[:, 1]
bars3 = bars[1,5]/bars[:, 2]
bars4 = bars[1,5]/bars[:, 3]
bars5 = bars[1,5]/bars[:, 4]



# Set position of bar on X axis
r1 = np.arange(len(bars1))
r2 = [x + barWidth for x in r1]
r3 = [x + barWidth for x in r2]
r4 = [x + barWidth for x in r3]
r5 = [x + barWidth for x in r4]

plt.bar(r1, bars1, color='#7f6d5f', width=barWidth, edgecolor='white', label='avoid mutex')
plt.bar(r2, bars2, color='purple', width=barWidth, edgecolor='white', label='atomic')
plt.bar(r3, bars3, color='green', width=barWidth, edgecolor='white', label='mutex')
plt.bar(r4, bars4, color='red', width=barWidth, edgecolor='white', label='sema')
plt.bar(r5, bars5, color='blue', width=barWidth, edgecolor='white', label='STM')

# Add xticks on the middle of the group bars
plt.yscale('log')
plt.xlabel('Pixel value', fontweight='bold')
plt.xticks([r  + barWidth for r in range(len(bars1))], ['rand() % 256', '0'])
plt.rcParams['axes.axisbelow'] = True
plt.ylabel('Speedup versus sequential', fontweight='bold')
# Create legend & Show graphic
plt.legend(loc='top left')
plt.grid()
plt.savefig("histo_plot" + str(i) + ".png",dpi=500)
plt.show()
'''
for i in range(6):
    bars1 = bars[i, 0]
    bars2 = bars[i, 1]
    bars3 = bars[i, 2]
    bars4 = bars[i, 3]
    bars5 = bars[i, 4]


    # Set position of bar on X axis
    r1 = np.arange(len(bars1))
    r2 = [x + barWidth for x in r1]
    r3 = [x + barWidth for x in r2]
    r4 = [x + barWidth for x in r3]
    r5 = [x + barWidth for x in r4]

    plt.bar(r1, bars1, color='#7f6d5f', width=barWidth, edgecolor='white', label='avoid')
    plt.bar(r2, bars2, color='#557f2d', width=barWidth, edgecolor='white', label='atomic')
    plt.bar(r3, bars3, color='green', width=barWidth, edgecolor='white', label='mutex')
    plt.bar(r4, bars4, color='red', width=barWidth, edgecolor='white', label='sema')
    plt.bar(r5, bars5, color='blue', width=barWidth, edgecolor='white', label='STM')

    # Add xticks on the middle of the group bars
    plt.xlabel('Image sizee (NxN)', fontweight='bold')
    plt.xticks([r + 0.25 + barWidth for r in range(len(bars1))], ['10x10', '100x100', '1000x1000', '2000x2000', '2500x2500' '5000x5000'])
    plt.rcParams['axes.axisbelow'] = True
    plt.ylabel('Execution time', fontweight='bold')
    # Create legend & Show graphic
    plt.legend(loc='top left')
    plt.grid()
    plt.savefig("histo_plot_different grids" + str(i) + ".png",dpi=500)
    plt.show()
'''