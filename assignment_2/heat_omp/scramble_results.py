import re
with open('pretty_results.txt', "w") as write_file:
        with open('test_results.txt') as f:
            for line in f:
                if "schedule" in line:
                        type_omp = re.search("schedule(.*)", line)
                        write_file.write("{}\n".format(type_omp.group(1)))
                if "-n" in line:
                        n  = re.search('-n (.*) #', line)
                        write_file.write("{}, ".format(n.group(1)))
                if "-m" in line:
                        m  = re.search('-m (.*) #', line)
                        write_file.write("{}\n".format(m.group(1)))
                if line[0:3] == "100":
                        line = line.split()
                        line = [elem for elem in line if elem.strip()]
                        line = [float(i) for i in line]
                        write_file.write("Flops: {:e}\n".format(line[6]))


