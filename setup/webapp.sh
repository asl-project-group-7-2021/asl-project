#!/bin/bash

# set the hostname
./optional/set-hostname.sh "imovies.ch"

# general debian hardening
./debian-hardening.sh

# set the ip of the computer to 192.168.0.1
sed -i 's/address 192.168.0.0/address 192.168.0.1/' /etc/network/interfaces

# setup rsyslod sender
./optional/log-sender.sh

# setup nginx
./optional/nginx.sh

# redirect http to https
./optional/nginx-tls-only.sh

# install static page
./optional/nginx-webapp.sh

# install curl
apt -y install curl


# download certificates and private keys
git clone https://github.com/asl-project-group-7-2021/asl-project-keys.git ../asl-project-keys

# move them to /opt/tls
mkdir /opt/tls
cp ../asl-project-keys/imovies.ch/imovies.ch.crt /opt/tls
cp ../asl-project-keys/imovies.ch/imovies.ch.key /opt/tls

# only allow reading the files to the owner and the group
chmod -R 700 /opt/tls
# set the owner to root, nobody should be able to read this files except for the root user
chown -R root:root /opt/tls

# install gnupg, required for the following
apt -y install gnupg

# add yarn repo to apt
curl -sS https://dl.yarnpkg.com/debian/pubkey.gpg | apt-key add -
echo "deb https://dl.yarnpkg.com/debian/ stable main" | tee /etc/apt/sources.list.d/yarn.list

apt update

# install nodejs and yarn
apt -y install nodejs yarn

git clone https://github.com/asl-project-group-7-2021/asl-ca-frontend /srv/asl-ca-frontend

# add .env file
cp ./configs/.env-frontend /srv/asl-ca-frontend/.env

# install npm dependencies
yarn --cwd /srv/asl-ca-frontend install

# build webapp
yarn --cwd /srv/asl-ca-frontend build

# generate static files
yarn --cwd /srv/asl-ca-frontend export

# restart nginx
systemctl restart nginx

./cleanup.sh