import sys

def process(infile, outfile, eol):
  with open(infile, "r") as fp:
    for line in fp:
      tps = float(line)
      outfile.write(str(tps) + eol)
      break

path = sys.argv[1]
wpers = sys.argv[2].split(",")
servers = sys.argv[3].split(",")
clients = sys.argv[4].split(",")

for wper in wpers:
  ftps = open(path + "/tps_" + wper, "w")
  flat = open(path + "/lat_" + wper, "w")

  ftps.write("\"#Servers\"")
  flat.write("\"#Servers\"")
  for c in clients:
    ftps.write("\t\"" + c + "\"")
    flat.write("\t\"" + c + "\"")
  ftps.write("\n")
  flat.write("\n")

  for s in servers:
    ftps.write(s)
    flat.write(s)
    for client in clients:
      c = str(int(client) * int(s))
      infile = open(path + "/" + wper + "_" + s + "_" + c, "r")
      data = infile.read().splitlines()
      ftps.write("\t\"" + data[0] + "\"")
      flat.write("\t\"" + data[1] + "\"")
    ftps.write("\n")
    flat.write("\n")

  ftps.close()
  flat.close()
