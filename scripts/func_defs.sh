#! /bin/bash

function limit_background_jobs () {
    local exe=$1
    shift
    local njobs=$1
    [ $# -eq 0 ] && njobs=`nproc`

    while [ `pgrep -f -u $USER $exe | wc -l` -gt $( echo "$njobs + 1" | bc) ]; do
        sleep $(echo $RANDOM" % 10" | bc -l)
    done
    sleep 1
}

function basename_with_dotroot_if_needed () {
    echo `basename $1``[ $(basename $1 | rev | cut -d"." -f 1 | rev) != "root" ] && echo ".root"`
}

function make_jobs {
    local input_ntup_dir=$1
    local out_subdir=$2
    local is_mc=$3
    shift; shift; shift
    local additional_evloop_arguments="$*"
    
    if [ $is_mc -eq 1 ]; then
        local campaigns=$mc_campaigns
    else
        local campaigns=$data_campaigns
    fi
    local jobs_out_dir=$out_dir/$out_subdir/`basename $input_ntup_dir | sed "s/_\?MC16X//g; s/_\?dataX//g" | sed "s/+/_/g"`

    for campaign in `echo $campaigns`; do
        local input_ntup_dir_camp=`echo $input_ntup_dir | sed "s/MC16X/MC16$campaign/g; s/dataX/data$campaign/g"`
        mkdir -p $jobs_out_dir/$campaign
        for input in `ls $input_ntup_dir_camp`; do
            local output=$jobs_out_dir"/"$campaign"/"`basename_with_dotroot_if_needed $input`
            echo $bin_dir"/event_loop" -t 1 -p $campaign --disable-progress-bar $additional_evloop_arguments -i $input_ntup_dir_camp"/"$input -o $output | tee -a $out_dir/logs/jobs.txt
        done
    done
}
