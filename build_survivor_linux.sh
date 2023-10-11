#!/bin/sh
sudo git stash push --include-untracked
sudo git stash drop
sudo git pull origin master
cd ./mp/src
sudo chmod -R +x devtools/*
sudo chmod -R +x devtools/bin
sudo chmod -R +x devtools/bin/linux
sudo chmod +x creategameprojects
sudo bash creategameprojects
sudo make -f games_survivor.mak clean
sudo make -f games_survivor.mak
