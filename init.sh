#!/bin/bash
SH_PATH="$(dirname $0)/"
chmod 755 ${SH_PATH}
mkdir ${SH_PATH}.session
chmod 733 .session
if [ -f /usr/bin/tmux ];then
    cp /usr/bin/tmux ${SH_PATH}
else
    echo "cannot find tmux in /usr/bin"
    echo "please cp your tmux exe file to ${SH_PATH}"
fi
