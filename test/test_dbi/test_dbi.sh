#! /bin/bash

# Save a copy of the environment (for debugging).

env > env.txt

# Exit if stash cache isn't mounted.

UBOONE_EXAMPLE_DATA_DIR=/cvmfs/uboone.osgstorage.org/stash/uboone_example_data
if [ ! -d $UBOONE_EXAMPLE_DATA_DIR ]; then
  echo "Quittig because stash cache isn't available."
  exit
fi

# This script runs fcl file test_dbi.fcl

input=$UBOONE_EXAMPLE_DATA_DIR/swizzled/PhysicsRun-2016_3_14_9_22_21-0005432-00021_20160322T065603_ext_bnb_20160323T041757_merged.root
cmd="lar --rethrow-all -c test_dbi.fcl -s $input -n 5"
echo $cmd
$cmd > test_dbi.out 2> test_dbi.err
stat=$?
echo "Command finished with status $stat"
exit $stat
