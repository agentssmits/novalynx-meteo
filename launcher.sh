#!/bin/bash
LOG='/home/oper/meteo2/server_log/meteo_launcher.log'

function echo_log {
    echo `date`" $1" >> $LOG
}

pgrep -x "server" > /dev/null
if [ $? != 0 ]
then
	echo_log "Relaunching meteo"
	#tmux new -d -s meteo "/home/oper/meteo2/server | /home/oper/meteo2/predate.sh 2>&1  >> "/home/oper/meteo2/server_log/meteo2.log""
	tmux kill-session -t meteo &>/dev/null
	tmux new -d -s meteo "stdbuf -oL -eL /home/oper/meteo2/server 2>&1 | /home/oper/meteo2/predate.sh >> /home/oper/meteo2/server_log/meteo2.log"
	#tmux new -d -s meteo "unbuffer /home/oper/meteo2/server"
fi
