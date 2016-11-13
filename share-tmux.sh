#!/bin/bash
SH_PATH="$(dirname $0)/"
SESSION="${SH_PATH}.sessions/`whoami`-session"
#TERM=dtterm
${SH_PATH}tmux -f ${SH_PATH}tmux.conf -S $SESSION new -d -s `whoami`
chmod 666 $SESSION
${SH_PATH}tmux -S $SESSION attach -t `whoami`
rm $SESSION
#rm /tmp/sharing

