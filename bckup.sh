#!/bin/bash

LOG='/home/oper/meteo2/bckup_logs/bckup.log'

function echo_log {
    echo `date`" $1" >> $LOG
}

now=`date +"%m_%d_%Y"`
DAY_LOG='/home/oper/meteo2/bckup_logs/bckup_'$now.log


echo_log "Rsync called"
rsync -aAXv --no-perms --no-owner --no-group --include '*.log' --exclude '*' /home/oper/meteo2/meteo_data/ oper@193.105.155.168://Docs/Dokumenti/Observations/Logfiles/meteo  >>  $DAY_LOG 2>&1
ret_val=$?
log_string='Return value is '$ret_val', see '$DAY_LOG
echo_log "$log_string"
if [ $ret_val -ne 0 ]
then
	/home/oper/gmail/gmail.py -s 'Error from meteo rsync' -t "$log_string" -a $DAY_LOG >> /home/oper/gmail/gmail.log 2>&1
fi

