#! /bin/bash

start_time=`date +%s`

mc_campaigns="a d e"
data_campaigns="1516 17 18"


do_systematics=1
do_fake_systematics=1
while getopts "sb" opt; do
    case $opt in
        s) do_systematics=1 ;;
        b) do_fake_systematics=1 ;;
    esac
done

bin_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
out_dir=output_eventloop_`date +%F-%s`
mkdir -p $out_dir/logs
out_dir=`readlink -f $out_dir`

source $bin_dir/sample_dir_defs.sh
source $bin_dir/func_defs.sh

for i in "${EXOT12_data_ntuple_inputs[@]}"; do make_jobs $i data 0 $([ $do_systematics -ne 0 ] && echo "-s") -d; done
for i in "${EXOT12_signal_ntuple_inputs[@]}"; do make_jobs $i / 1 $([ $do_systematics -ne 0 ] && echo "-s") --is-signal --keep-fake-mc; done
for i in "${EXOT12_noboson_bkg_ntuple_inputs[@]}"; do make_jobs $i / 1 $([ $do_systematics -ne 0 ] && echo "-s"); done
for i in "${EXOT12_wz_bkg_ntuple_inputs[@]}"; do make_jobs $i / 1 $([ $do_systematics -ne 0 ] && echo "-s") -w; done
for i in "${EXOT12_wz_bkg_ntuple_inputs[@]}"; do make_jobs $i wz_up 1 $([ $do_systematics -ne 0 ] && echo "-s") -z; done
for i in "${EXOT12_wz_bkg_ntuple_inputs[@]}"; do make_jobs $i wz_down 1 $([ $do_systematics -ne 0 ] && echo "-s") -q; done
for i in "${EXOT12_twz_bkg_ntuple_inputs[@]}"; do make_jobs $i / 1 $([ $do_systematics -ne 0 ] && echo "-s"); done
for i in "${EXOT12_tz_bkg_ntuple_inputs[@]}"; do make_jobs $i / 1 $([ $do_systematics -ne 0 ] && echo "-s"); done
for i in "${EXOT12_alt_ttz_bkg_ntuple_inputs[@]}"; do make_jobs $i ttz_alt 1 $([ $do_systematics -ne 0 ] && echo "-s"); done

for i in "${EXOT12_data_ntuple_inputs[@]}"; do make_jobs $i fakes/data 0 $([ $do_fake_systematics -ne 0 ] && echo "-b") -f lep -d; done
for i in "${EXOT12_bkg_ntuple_inputs[@]}"; do make_jobs $i fakes 1 $([ $do_fake_systematics -ne 0 ] && echo "-b") -f lep; done

end_time=`date +%s`
time_diff=`echo $end_time - $start_time | bc`
echo "Finished after "$time_diff$" seconds."
