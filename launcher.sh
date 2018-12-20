#!/bin/bash

declare -r VM_NAME="windows"
declare -r MAIN_PATH="./notification.log"

VBoxManage startvm $VM_NAME
while inotifywait -e close_write $MAIN_PATH;
do
	VBoxManage controlvm $VM_NAME poweroff soft
	VBoxManage startvm $VM_NAME
done
