let folder=$1

rm shard*.config
cp $folder/* ./
if [ $# -eq 1 ]; then
    python gen_conf.py
else
    python gen_conf.py $2
fi
