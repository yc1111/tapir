for server in `cat replicas`
do
  echo $server
  ssh ${server} "source ~/.profile; cd /data/yc/tapir; make clean; make;" &
done
for server in `cat clients`
do
  echo $server
  ssh ${server} "source ~/.profile; cd /data/yc/tapir; make clean; make;" &
done

