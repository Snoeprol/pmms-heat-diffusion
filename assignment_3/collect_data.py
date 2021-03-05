data_array = []
i_array = []

with open('data.txt') as f:
    lines = f.readlines()
    for line in lines:
        if "i" in line:
            split_line = line.split('=')
            data_array.append([])
            i_array.append(int(split_line[1]))

    for item in data_array:
        for i in range(5):
            item.append([]) 
    print(data_array)   
    bin_count = -1
    print(i_array)
    print(len(data_array))
    for line in lines:
        if "i" in line:
            bin_count += 1
        if not "i" in line:
            split_line = line.split(",")
            print("bin count: " + str(bin_count) + "item: " + str(int(split_line[0])))
            data_array[bin_count][int(split_line[0])].append(float(split_line[1]))

print(data_array)
print(i_array)        