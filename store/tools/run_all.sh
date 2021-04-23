wperc=(20)
nclient=(2 4 6 8 10)
nodes=(1 4 8 12 16)
rtime=120

wpers=$( IFS=$','; echo "${wperc[*]}" )
servers=$( IFS=$','; echo "${nodes[*]}" )
clients=$( IFS=$','; echo "${nclient[*]}" )

for i in ${wperc[@]}
do
  for j in ${nodes[@]}
  do
    ./load.sh $j
    for c in ${nclient[@]}
    do
      let k=$c*$j

      echo ====================================
      echo -e $i % writes, $j nodes, $k clients
      echo ====================================

      sed -i -e "s/tlen=[0-9]*/tlen=10/g" run_test.sh
      sed -i -e "s/rtime=[0-9]*/rtime=${rtime}/g" run_test.sh
      sed -i -e "s/wper=[0-9]*/wper=$i/g" run_test.sh
      sed -i -e "s/nshard=[0-9]*/nshard=$j/g" run_test.sh
      sed -i -e "s/nclient=[0-9]*/nclient=$k/g" run_test.sh
      sed -i -e "s/zalpha=-1/zalpha=0/g" run_test.sh
      
      ./clean.sh
      ./run_test.sh
      ./clean.sh
    done
  done
done

python parse_result.py ../../data/ $wpers $servers $clients
