#! /bin/bash

[ $# -ne 1 ] && echo "Provide one argument: path to a directory containing the output of prepare_jobs.sh" && exit 1

start_time=`date +%s`

jobs_dir=$1
bin_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cp $bin_dir/vll_flat.submit $jobs_dir/logs/
sed -i 's^BINDIR^'$bin_dir'^g' $jobs_dir/logs/vll_flat.submit
sed -i 's^OUTDIR^'$jobs_dir'^g' $jobs_dir/logs/vll_flat.submit
condor_submit $jobs_dir/logs/vll_flat.submit

end_time=`date +%s`
time_diff=`echo $end_time - $start_time | bc`
echo "Finished after "$time_diff" seconds."
