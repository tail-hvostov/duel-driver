#!/bin/sh
module="duel"
device="duel"
# remove stale nodes
rm -f /dev/${device}[0-2]
/sbin/rmmod ./$module.ko $* || exit 1
