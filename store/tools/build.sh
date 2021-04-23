for server in `cat replicas`
do
  echo $server
  rsync -r ../../../tapir $server:/data/yc/
  ssh ${server} "source ~/.profile; cd /data/yc/tapir; make;" &
done

cd /data/yc/tapir
make
cd store/tools
