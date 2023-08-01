# VLL Flat Ntuple Maker

## Dependencies
* CMake 3.11 or higher
* ROOT 6.14 or higher

## Quick Start

Instructions for building and running on lxplus (CERN):

```
setupATLAS && lsetup cmake && lsetup root
mkdir build && cd build
cmake ..
make -j`nproc`
```

## Running the code
To run the nominal flat ntuple generation:
```
cd build
bin/prepare_jobs.sh
```

To run the flat ntuple generation for BDT training:
```
cd build
bin/prepare_training_jobs.sh
```

An output directory will be created in the build directory

To run on your local machine:
```
bin/run_local.sh output_directory_made_by_prepare_jobs
```

To run on condor:
```
bin/run_condor.sh output_directory_made_by_prepare_jobs
```

To check condor status:
```
condor_q
```

When jobs are finished:
```
bin/postprocessing.sh output_directory_made_by_prepare_jobs.sh
```

To run with systematics, edit the following lines in `scripts/prepare_jobs.sh`:
```
do_systematics=0
do_fake_systematics=0
```
to
```
do_systematics=1
do_fake_systematics=1
```
in the build directory, make sure you run `make` before `prepare_jobs.sh`
