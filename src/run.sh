#!/usr/bin/env bash

TOTAL_PHYSICAL_CORES=`grep '^core id' /proc/cpuinfo | sort -u | wc -l`
TOTAL_LOGICAL_CORES=`grep '^core id' /proc/cpuinfo | wc -l`

# Load MSR module
sudo modprobe msr

# Setup
samples=10000	# 10 seconds
num_thread=$TOTAL_LOGICAL_CORES
date=`date +"%m%d-%H%M"`

# Alert

### Warm Up ###
stress-ng -q --cpu $TOTAL_LOGICAL_CORES -t 5m
echo "hamming distance"
sudo ./hd
