#!/bin/bash
SH_PATH="$(dirname $0)"
# directory and tmux main program setting
chmod 755 ${SH_PATH}
if [ ! -d ${SH_PATH}/../.sessions ];then
    mkdir ${SH_PATH}/../.sessions
fi
chmod 733 ${SH_PATH}/../.sessions
if [ -f /usr/bin/tmux ];then
    cp /usr/bin/tmux ${SH_PATH}
else
    echo "cannot find tmux in /usr/bin"
    echo "please cp your tmux exe file to ${SH_PATH}"
fi

gcc tmuxSharing.c -o tmuxSharing
