import sys

replication = str(0)
if (len(sys.argv) == 2 and sys.argv[1] == "-r"):
    replication = str(1)

count = int(0)
portno = int(29)
tss_port = int(25)

infile = open("replicas", "r")
ips = infile.read().splitlines()
infile.close()

n_shards = len(ips)
for i in range(n_shards):
    outfile = open("shard" + str(i) + ".config", "w")
    outfile.write("f " + replication + "\n")
    outfile.write("replica " + ips[i] + ":517" + str(portno) + "\n")
    if replication == "1":
        outfile.write("replica " + ips[(i+1)%n_shards] + ":517" + str(portno) + "\n")
        outfile.write("replica " + ips[(i+2)%n_shards] + ":517" + str(portno) + "\n")
    outfile.close()
    if i == 0:
        tss = open("shard.tss.config", "w")
        tss.write("f " + replication + "\n")
        tss.write("replica " + ips[i] + ":517" + str(tss_port) + "\n")
        if replication == "1":
            tss.write("replica " + ips[(i+1)%n_shards] + ":517" + str(tss_port) + "\n")
            tss.write("replica " + ips[(i+2)%n_shards] + ":517" + str(tss_port) + "\n")
        tss.close()
    portno = portno + 1
