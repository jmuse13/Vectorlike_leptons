#! /bin/bash

[ $# -ne 1 ] && echo "Provide one argument: path to a directory containing the output of prepare_jobs.sh" && exit 1

start_time=`date +%s`

jobs_dir=$1
bin_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source $bin_dir/func_defs.sh

i=0
while read line; do
    limit_background_jobs event_loop
    echo $line
    nice -n 10 $line > $jobs_dir/logs/$i".out" 2> $jobs_dir/logs/$i".err" &
    ((i=i+1))
done < $jobs_dir/logs/jobs.txt
wait

end_time=`date +%s`
time_diff=`echo $end_time - $start_time | bc`
echo "Finished after "$time_diff" seconds."
