import subprocess
from random import randint, seed, uniform

seed(100)

filename = "../ResultData/Inputs/NumOfPCs/"
logs = "../ResultData/Inputs/NumOfPCs/LogRes/"
k = 0
for (PCFrom, PCTo) in [(0.1, 0.3), (0.3, 0.5)]:
	for (ModFrom, ModTo) in [(0.1, 0.3), (0.3, 0.5)]:
		for (LCMFrom, LCMTo) in [(3, 5), (6, 10)]:
			PCNum = uniform(PCFrom, PCTo)
			ModNum = uniform(ModFrom, ModTo)
			LCM = randint(LCMFrom, LCMTo)
			curFile = filename + str(k) + "/"
			Curlogs = logs + str(k) + "_"
			for TaskNum in range(10, 101, 10):
				for j in range(10):
					print(f"j = {j}")
					NumOfTAsk = randint(TaskNum, TaskNum + 10)
					resFile = curFile + str(TaskNum) + "_" + str(j) + ".xml"
					logFile = Curlogs + str(TaskNum) + "_" + str(j) + ".txt"
					NumOfMes = round(randint(0, 15) / 100 * NumOfTAsk)
					NumOfPC = max(round(PCNum * NumOfTAsk), 1)
					NumOfMod = max(round(ModNum * NumOfPC), 1)
					print(f"./main {LCM} {NumOfTAsk} {NumOfPC} {NumOfMod} {NumOfMes} 100 {resFile} > {logFile}")
					res = 0
					while (res == 0):
						try:
							subprocess.check_call([f"./main {LCM} {NumOfTAsk} {NumOfPC} {NumOfMod} {NumOfMes} 100 {resFile} > {logFile}"], shell=True)
						except subprocess.CalledProcessError as e:
							print(e)
							res = 0
						else:
							res = 1
			k += 1
    