for host in `cat replicas`
do
  echo $host
  rsync -a /data/yc/tapir $host:/data/yc/
done
for host in `cat clients`
do
  echo $host
  rsync -a /data/yc/tapir $host:/data/yc/
done

