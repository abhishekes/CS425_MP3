#!/bin/bash -debug

#ipArray=(192.168.2.3 192.168.2.1 192.168.2.10 192.168.2.15 192.168.2.20)
ipArray=(192.168.2.3 192.168.2.45)

echo  "These are the choices you have :"
echo  "1. Frequent pattern"
echo  "2. Scarce pattern"
echo  "3. User defined pattern"

read choice

case $choice in
1) 
   pattern="FREQUENT"
   ;;

2)
  pattern="SCARCE"
   ;;

3) 
  echo "Enter pattern"
  read pattern
   ;;

esac
rm -rf command_output*.txt   
./dgrep_client $pattern >> /dev/null

((j = 1))

for i in "${ipArray[@]}"
do
  echo "Enter user name"
  read user
  
  ssh $user@$i "grep -n $pattern ~/dgrep/server/machine.$j.log &> /tmp/machine.$j.test.txt"
  scp $user@$i:/tmp/machine.$j.test.txt ./

  if diff command_output_$j.txt machine.$j.test.txt  ; then
	echo "Test passed on machine $j -> $i"
  else
        echo "Mismatch in grep output. Scan files and compare manually "
  fi

  (( j++ ))

done
