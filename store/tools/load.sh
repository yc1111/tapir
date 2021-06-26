let folder=$1

rm shard*.config
if [ $# -eq 1 ]; then
    python gen_conf.py $1
else
    python gen_conf.py $1 $2
fi
