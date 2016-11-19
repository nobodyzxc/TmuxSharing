#!/bin/bash
SH_PATH="$(dirname $0)"
# directory and tmux main program setting
#chmod 755 ${SH_PATH}
#if [ ! -d ${SH_PATH}/../.sessions ];then
#    mkdir ${SH_PATH}/../.sessions
#fi
#chmod 733 ${SH_PATH}/../.sessions

gcc tmuxSharing.c -o tmuxSharing
