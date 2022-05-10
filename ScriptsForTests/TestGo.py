import subprocess
import csv
from os import listdir
from os.path import isfile, join


info = ["Group", "NumOfTasks", "Succesful"]


filename = "ResultData/Inputs/NumOfPCs/"
dots = "ResultData/Outputs/Dots/"
for k in range(0, 8):
    resDict = []
    curFile = filename + str(k) + "/"
    curDot = dots + str(k) + "_"
    for TaskNum in range(10, 111, 10):
        onlyfiles = [f for f in listdir(curFile) if (isfile(join(curFile, f)) and f.startswith(str(TaskNum) + "_"))]
        sum_ = 0
        for file in onlyfiles:
            resDot = curDot + file[0:-3] + "dot"
            print(f"./main {(join(curFile, file))} {resDot}")
            try:
                res = subprocess.check_output([f"./main {(join(curFile, file))} {resDot}"], shell=True)
            except subprocess.CalledProcessError as e:
                res = 0
            print(res)
            sum_ += int(res)
        if (len(onlyfiles) != 0):
            resDict.append({"Group" : k, "NumOfTasks": TaskNum, "Succesful": sum_ / len(onlyfiles)})
    with open('ResultData/Outputs/NumOfTasks/' + str(k) + '.csv', 'w') as csvfile:
        result = csv.DictWriter(csvfile, fieldnames = info)
        result.writeheader()
        result.writerows(resDict)