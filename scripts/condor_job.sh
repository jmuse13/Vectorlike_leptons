#! /bin/bash

args="$*"
shift $#

ouhep_number=`hostname | cut -d "." -f 1 | tail -c 3`
infile=`echo $args | grep -Eo '\-i .+ ' | awk '{print $2}'`
new_infile=`echo $infile | sed 's^raid..^raid'$ouhep_number'^g'`
if [ -f $new_infile ]; then
    args=`echo $args | sed 's^'$infile'^'$new_infile'^g'`
fi

export ALRB_rootVersion=6.20.06-x86_64-centos7-gcc8-opt
export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
lsetup root

nice -n -10 $args
