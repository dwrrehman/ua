#!/bin/zsh
# useful ssh script to execute commands on all mac minis. 
# written on 1202604046.203804

scp 0/get_progress_update.sh dwrr-mini.local:/Users/dwrr/root/
scp 1/get_progress_update.sh dwrr-mini1.local:/Users/dwrr/root/
scp 2/get_progress_update.sh dwrr-mini2.local:/Users/dwrr/root/
scp 3/get_progress_update.sh dwrr-mini3.local:/Users/dwrr/root/
scp 4/get_progress_update.sh dwrr-mini4.local:/Users/dwrr/root/
scp 5/get_progress_update.sh dwrr-mini5.local:/Users/dwrr/root/
scp 6/get_progress_update.sh dwrr-mini6.local:/Users/dwrr/root/
scp 7/get_progress_update.sh dwrr-mini7.local:/Users/dwrr/root/
scp 8/get_progress_update.sh dwrr-mini8.local:/Users/dwrr/root/
scp 9/get_progress_update.sh dwrr-mini9.local:/Users/dwrr/root/

ssh dwrr-mini.local "./root/get_progress_update.sh"
ssh dwrr-mini1.local "./root/get_progress_update.sh"
ssh dwrr-mini2.local "./root/get_progress_update.sh"
ssh dwrr-mini3.local "./root/get_progress_update.sh"
ssh dwrr-mini4.local "./root/get_progress_update.sh"
ssh dwrr-mini5.local "./root/get_progress_update.sh"
ssh dwrr-mini6.local "./root/get_progress_update.sh"
ssh dwrr-mini7.local "./root/get_progress_update.sh"
ssh dwrr-mini8.local "./root/get_progress_update.sh"
ssh dwrr-mini9.local "./root/get_progress_update.sh"

























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


































