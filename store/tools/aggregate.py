import sys
from glob import glob

numSuccess = 0.0
sumSuccess = 0.0
numTotal = 0.0
sumTotal = 0.0
duration = 0.0

path = sys.argv[1]
outpath = sys.argv[2]
for f in glob(path + "/client*log"):
  with open(f, "r") as fp:
    lines = fp.read().splitlines()
    numSuccess = numSuccess + float(lines[0])
    sumSuccess = sumSuccess + float(lines[1])
    numTotal = numTotal + float(lines[2])
    sumTotal = sumTotal + float(lines[3])
    duration = float(lines[4])

outfile = open(outpath, "w")
outfile.write(str(numSuccess/duration) + "\n")
outfile.write(str(sumSuccess/numSuccess) + "\n")
outfile.write(str(numTotal/duration) + "\n")
outfile.write(str(sumTotal/numTotal) + "\n")
outfile.write(str((numTotal - numSuccess)/numTotal) + "\n")
