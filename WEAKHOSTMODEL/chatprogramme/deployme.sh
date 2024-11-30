#!/bin/sh
set -ex
curl --silent --remote-name https://raw.githubusercontent.com/RinicomBG/internetapplicationsengineeringguesttalk/refs/heads/main/WEAKHOSTMODEL/chatprogramme/chatprogramme.c
gcc -o chatprogramme chatprogramme.c
