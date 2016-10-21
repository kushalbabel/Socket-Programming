f = open("avg.txt")
sum = 0
for line in f.readlines():
	if "time" in line:
		sum = sum + float(line[4:])
		# print float(line[4:])
f1 = open("time8.dat",'a+')
f1.write(str(sum/10)+'\n')