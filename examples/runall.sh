#!/bin/bash


for bas in *.bas
do
    echo '- - - - -' $bas '- - - - -'
    ../code/basic $bas
    #echo 'Status: ' $?
done

