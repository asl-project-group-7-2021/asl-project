#!/bin/bash

# log all commands executed using bash
echo "HISTFILE=~/.bash_history" >> /etc/profile
echo "HISTSIZE=10000" >> /etc/profile
echo "HISTFILESIZE=999999" >> /etc/profile
echo "HISTIGNORE=\"\"" >> /etc/profile
echo "HISTCONTROL=\"\"" >> /etc/profile

# make the variables unmutable
echo "readonly HISTFILE" >> /etc/profile
echo "readonly HISTSIZE" >> /etc/profile
echo "readonly HISTFILESIZE" >> /etc/profile
echo "readonly HISTIGNORE" >> /etc/profile
echo "readonly HISTCONTROL" >> /etc/profile
echo "export HISTFILE HISTSIZE HISTFILESIZE HISTIGNORE HISTCONTROL" >> /etc/profile

# now set the file ~/.bash_history for all users to append-only
# for root it is a lost cause as the root user can remove the append-only flag anyway

# first create it if it doesn't exist yet
touch /home/administrator/.bash_history
# then change ownership
chown administrator:administrator /home/administrator/.bash_history
# and change the permissions
chmod 700 /home/administrator/.bash_history
# Only the superuser or a process possessing the CAP_LINUX_IMMUTABLE capability can set or clear this attribute. 
chattr +a /home/administrator/.bash_history

# logcheck: https://www.debian.org/doc/manuals/securing-debian-manual/log-alerts.en.html

# now also install auditd, a much more powerful tool
apt -y install auditd

# configure it

# Use these rules https://github.com/Neo23x0/auditd/blob/master/audit.rules
cp ./config/audit.rules /etc/audit/rules.d/audit.rules

# and enable it
systemctl enable auditd
service auditd start

# forward journald to syslog that is synced to a backup server
sed -i 's/#ForwardToSyslog=yes/ForwardToSyslog=yes/' /etc/systemd/journald.conf