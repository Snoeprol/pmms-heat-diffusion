import numpy as np

data_array = []
data_array_means = []
i_array = []

# Structure

# data_array[c] accesses the core.
# data_array[c][i] accesses the grid.
# data_array[x][i] accesses the i-th method of the x-th i (lol).
dat = np.zeros((6, 6, 5, 1))
dat_means = np.zeros((6, 6, 5))
with open('data.txt') as f:

    # Build structure of array
    lines = f.readlines()
    for line in lines:
        if "c" in line:
            data_array.append([])
            data_array_means.append([])
    for line in lines:
        if "i" in line:
            split_line = line.split('=')
            for sub_array_1, sub_array_2 in zip(data_array, data_array_means):
                sub_array_1.append([])
                sub_array_2.append([])
            i_array.append(int(split_line[1]))

    for item_1, item_2 in zip(data_array, data_array_means):
        for list_1, list_2 in zip(item_1, item_2):
            for i in range(5):
                list_1.append([]) 
                list_2.append([])
   

    #print(data_array)   
    i_bin_count = -1
    c_bin_count = -1
        
    for line in lines:
        if "c" in line:
            c_bin_count += 1
            i_bin_count = -1
        if "i" in line:
            i_bin_count += 1
        if not "i" in line and not "c" in line:
            split_line = line.split(",")
            #print("bin count: " + str(i_bin_count) + "item: " + str(int(split_line[0])))
            #print(c_bin_count)
            x = data_array[c_bin_count][i_bin_count][int(split_line[0])].append(float(split_line[1]))
            p = dat[c_bin_count][i_bin_count][int(split_line[0])]
            for i, val in enumerate(p):
                if p == 0:
                    dat[c_bin_count][i_bin_count][int(split_line[0])][i] = float(split_line[1])

#print(data_array)
#print(i_array)  
#print(dat)      

print(dat)

for i, core_nr in enumerate(dat):
    for j, grid_size in enumerate(core_nr):
        for k, method in enumerate(grid_size):
            dat_means[i][j][k] = np.mean(method)

bars = dat_means
############################################################################################################################################# START PLOT
# set width of bar
barWidth = 0.08

import numpy as np
import matplotlib.pyplot as plt
# Cores grids methods
for i in range(6):
    bars1 = bars[0,i,5]/bars[:, i, 0]
    bars2 = bars[0,i,5]/bars[:, i, 1]
    bars3 = bars[0,i,5]/bars[:, i, 2]
    bars4 = bars[0,i,5]/bars[:, i, 3]
    bars5 = bars[0,i,5]/bars[:, i, 4]


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
    plt.xlabel('Threads', fontweight='bold')
    plt.xticks([r + 0.25 + barWidth for r in range(len(bars1))], ['1', '2', '4', '8', '16', '32'])
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