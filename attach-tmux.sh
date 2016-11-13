#!/bin/bash
if [ $# != 1 ];then
    echo "give me user you want to attach"
    exit
fi
SH_PATH="$(dirname $0)/"
SESSION="${SH_PATH}.sessions/$1-session"
#TERM=dtterm
${SH_PATH}tmux -S $SESSION attach -t $1
