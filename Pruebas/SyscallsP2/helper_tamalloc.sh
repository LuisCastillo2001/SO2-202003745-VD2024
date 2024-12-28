#!/bin/bash

# Check if a PID is provided as an argument
if [ -z "$1" ]; then
    echo "Usage: $0 <pid>"
    exit 1
fi

PID=$1

# Check if the PID directory exists in /proc
if [ ! -d "/proc/$PID" ]; then
    echo "Error: Process with PID $PID does not exist."
    exit 1
fi

# Extract VmSize, VmRSS, and OOM Score
VMSIZE=$(grep -E 'VmSize:' /proc/$PID/status | awk '{print $2, $3}')
VMRSS=$(grep -E 'VmRSS:' /proc/$PID/status | awk '{print $2, $3}')
OOM_SCORE=$(cat /proc/$PID/oom_score)

# Display the information
echo "Process Info for PID $PID:"
echo "  VmSize: $VMSIZE"
echo "  VmRSS:  $VMRSS"
echo "  OOM Score: $OOM_SCORE"
