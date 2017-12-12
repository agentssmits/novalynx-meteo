#!/bin/bash

# trap ctrl-c and call ctrl_c()
trap ctrl_c INT
function ctrl_c() {
        echo "** Trapped CTRL-C"
		sleep 2
		read line
		echo "$(date): ${line}"
}
while read line ; do
    echo "$(date): ${line}"
done