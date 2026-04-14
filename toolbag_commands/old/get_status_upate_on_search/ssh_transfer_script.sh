#!/bin/zsh
# useful ssh script to execute commands on all mac minis. 
# written on 1202604046.203804

scp get_status.sh dwrr-mini0.local:/Users/dwrr/root/
scp get_status.sh dwrr-mini1.local:/Users/dwrr/root/
scp get_status.sh dwrr-mini2.local:/Users/dwrr/root/
scp get_status.sh dwrr-mini3.local:/Users/dwrr/root/
scp get_status.sh dwrr-mini4.local:/Users/dwrr/root/
scp get_status.sh dwrr-mini5.local:/Users/dwrr/root/
scp get_status.sh dwrr-mini6.local:/Users/dwrr/root/
scp get_status.sh dwrr-mini7.local:/Users/dwrr/root/
scp get_status.sh dwrr-mini8.local:/Users/dwrr/root/
scp get_status.sh dwrr-mini9.local:/Users/dwrr/root/

ssh dwrr-mini0.local "./root/get_status.sh"
ssh dwrr-mini1.local "./root/get_status.sh"
ssh dwrr-mini2.local "./root/get_status.sh"
ssh dwrr-mini3.local "./root/get_status.sh"
ssh dwrr-mini4.local "./root/get_status.sh"
ssh dwrr-mini5.local "./root/get_status.sh"
ssh dwrr-mini6.local "./root/get_status.sh"
ssh dwrr-mini7.local "./root/get_status.sh"
ssh dwrr-mini8.local "./root/get_status.sh"
ssh dwrr-mini9.local "./root/get_status.sh"
























#ssh-keygen
#
#ssh-copy-id dwrr-mini.local
#ssh-copy-id dwrr-mini1.local
#ssh-copy-id dwrr-mini2.local
#ssh-copy-id dwrr-mini3.local
#ssh-copy-id dwrr-mini4.local
#ssh-copy-id dwrr-mini5.local
#ssh-copy-id dwrr-mini6.local
#ssh-copy-id dwrr-mini7.local
#ssh-copy-id dwrr-mini8.local
#ssh-copy-id dwrr-mini9.local
#
#


#ssh dwrr-mini.local
#cd root/projects/ua
#git pull
#git status 
#cd ~
#exit

#ssh dwrr-mini1.local
#cd root/projects/ua
#git pull
#git status 
#cd ~
#exit

# ...

#ssh dwrr-mini9.local
#cd root/projects/ua
#git pull
#git status 
#cd ~
#exit


































