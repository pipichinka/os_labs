#!/bin/bash

rm my*
gcc main.c -o myhl


./myhl myhl mymkdir
./myhl myhl myls
./myhl myhl myrmdir
./myhl myhl mytouch
./myhl myhl mycat
./myhl myhl myrm
./myhl myhl mylns
./myhl myhl mycatvsl
./myhl myhl mycatsl
./myhl myhl myrmsl
./myhl myhl myrmhl
./myhl myhl myinfo
./myhl myhl mychmod