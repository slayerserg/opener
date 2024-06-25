#!/bin/bash

project_path="/home/sergey/work/opole/opener-dev"

cd ./OpENer/bin/posix

truncate -s 0 $project_path/opener.log
echo "Starting OpENer..." >> $project_path/opener.log
echo "Date: " $(date -u) >> $project_path/opener.log
echo "###############################" >> $project_path/opener.log

./setup_posix.sh >> $project_path/opener.log 2>> $project_path/opener.log

make clean
make VERBOSE=1 2>> $project_path/opener.log

sudo setcap cap_net_raw+ep ./src/ports/POSIX/OpENer 2>> $project_path/opener.log
./src/ports/POSIX/OpENer eth0 2>> $project_path/opener.log &

for pid in $(ps -C OpENer -L -o lwp=)
do
  sudo taskset -cp 3-3 $pid
  sudo renice -n -5 -p $pid
done
