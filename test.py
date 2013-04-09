count = 0
for i in range(1, 10000000):
	for h in range(1, 20):
		if i%h==0:
			count=count+1
	if count>18:
		print i
	count=0
