#############################################################
# Author: Murat Ulu
# Date: 2/13
# Pledge: I pledge my honor that I have abided by the Stevens Honor System.
# Description: sh program that mimics the recycle bin
#############################################################
#!/bin/bash

help_flag=0
list_flag=0
purge_flag=0

readonly filePath=~/.junk

#HereDoc
usageMsg()
{
cat<<PICKLE
Usage: $(basename $0) [-hlp] [list of files]
   -h: Display help.
   -l: List junked files.
   -p: Purge all files.
   [list of files] with no other arguments to junk those files.
PICKLE
}

#Uses getopts to deal with flags
while getopts ":hlp" option; do
	case "$option" in
		h) help_flag=1
			;;
		l) list_flag=1
			;;
		p) purge_flag=1
			;;
		?) echo "Error: Unknown option '-$OPTARG'." >&2
		usageMsg
		exit 1
		;;
	esac
done



#Base case (no command line arguments, returns usage)
if [ $# -eq 0 ]; then
	usageMsg
	exit 0
fi



#Sets count for how many flags are input, checks if more than one flag is specified
count=$(( help_flag + list_flag + purge_flag ))
if [ $count -gt 1 ]; then
	echo "Error: Too many options enabled."
	usageMsg
	exit 1
fi

#Checks if one or more flags are specified and if files are supplied
if [ $count -eq 1 ]; then
	if [ $# -gt 1 ]; then
		echo "Error: Too many options enabled."
		usageMsg
		exit 1
	fi
fi



#creates ".junk" directory if existing directory not found
if [ ! -d $filePath ]; then
	mkdir $filePath
fi



#Help flag
if [ $help_flag -eq 1 ]; then
	usageMsg
	exit 0
fi

#List flag
if [ $list_flag -eq 1 ]; then
	ls -lAF $filePath
	exit 0
fi

#Purge flag
if [ $purge_flag -eq 1 ]; then
	rm -rf $filePath{*,.*}
	exit 0
fi



#Moving files into junk bin
for f in "$@";do
	if [ -f "$f" ]; then
		mv "$f" $filePath
	else
		if [ -d "$f" ]; then
			mv "$f" $filePath
		else
			echo "Warning '"$f"' not found."
		fi
	fi
done
exit 0
