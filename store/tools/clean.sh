for server in `cat replicas`
do
  echo $server
  rsync -r ../../../tapir/store/tools $server:/data/yc/tapir/store/
  ssh ${server} "killall -9 strongstore; killall -9 server; killall -9 timeserver; rm -rf /data/yc/tapir/logs;"
done
rm -rf /data/yc/tapir/logs
