#! /bin/bash

start_time=`date +%s`

bin_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source $bin_dir/func_defs.sh
cd $1

COUNT=0
ddd=0
for dir in `find . -maxdepth 1 -mindepth 1 -type d -not -regex '^\./\(extras\|logs\)$'`; do
    if [ $COUNT == 0 ]; then
        if [ "$dir" == "./data" ]; then
            ddd=1
        fi
    fi
    limit_background_jobs split_outfile_by_SR.py 2
    nice -n 10 python $bin_dir/split_outfile_by_SR.py $dir $ddd $COUNT &
    ((COUNT++))
done
wait

end_time=`date +%s`
time_diff=`echo $end_time - $start_time | bc`
echo "Finished after "$time_diff" seconds."
