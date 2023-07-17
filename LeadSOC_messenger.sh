#!/bin/sh
gcc -Wall client.c -o LEADSOC_CHAT

#sudo tcpdump -i enp0s3 -W 10 -w client_file -C 10 -Z root &
sudo tcpdump -W 10 -w client_file -C 10 -Z root &

echo -e "\n"


./LEADSOC_CHAT

pid=$(pidof tcpdump)
echo $pid
sudo kill -SIGKILL $pid

