#!/bin/sh

RECURSIVE=0 # Be recursive, default FALSE
MODE=0 # UPPERCASE[1] lowercase[2] or sed pattern [3]

#function rename() gets one parameter [$1] file or directory name
# optionally it uses created earlier variable $PATTERN
# rename() function is called for every file/dir we want to rename
rename() {
    	# check if it exists this file or directory
	if [ ! -f "$1" ] && [ ! -d "$1" ] ; then 
		#if no file and no directory 
		`echo "$1 not found..." 1>&2` # send to error input
		return
	fi
    
    	# if we are in recursive mode
	if [ $RECURSIVE -eq 1 ] ; then
		# visit every dir
		if [ -d $1 ] ; then
			cd ./$1; # go to that directory
			# and rename every file/dir inside
			for RECORD in `ls` ;
			do
				rename $RECORD;
			done
			cd .. # go back
		fi
	fi
    
    	# execute sed to obtain new name
	case $MODE in
		2) NAME=`basename "$1" | sed -e 's/\(.*\)/\U\1/'` ;; #create patern zero or more any character followed by dot => upercase, use created pattern 1st 
		1) NAME=`basename "$1" | sed -e 's/\(.*\)/\L\1/'` ;;
		*) NAME=`basename "$1" | sed "$PATTERN"`;; # just paste sed pattern 
	esac

	# we need to lowercase extension of a file (this is the best way i could manage :/ )
	# I don't like this echo commnand, but it's the best way I found to send data throught pipe into sed command, and then maps it using regex
	NAME="`echo $NAME | sed 's/\(\..*\)/\L\1/'`"; 

	NAME="`dirname $1`/"$NAME; #here in NAME variable we concatenated DIRNAME + (BASENAME | SED_OPERATION_ON_BASE_NAME)
	
    	# and rename if we can
	# if new name is different, if there is no file with this name, and no directory of this name
	if [ ! "$1" = "$NAME" ] && [ ! -f "$NAME" ] && [ ! -d "$NAME" ] ; then
		mv "$1" "$NAME";
	fi
}

# parsing first parameter
while [ "$1" != "" ] ; do
    case "$1" in
		"-r") RECURSIVE=1 ;; # RECURSIVE set to TRUE 
		"-l") MODE=1 ;; # MODE set to lowercase
		"-u") MODE=2 ;; # Mode set to uppercase 
		"-h") 	echo "Script which modify file names."
			echo "Usage: $0 -r [-l|-u] | <sed command> <filelist>";
			echo "[-r]		When specyfied, changes are made to object in all subdirectories." ;
			echo "[-l|-u]		Flag dedicated to lowerizing (-l) file names or uppercasing (-u).";
			echo "<sed command>	Pattern internally given to sed command during operation on files names.";
			echo "<filelist>	List of files to change.";			
			 exit;;
		-*)`echo "Unknown parameter $1" 1>&2`; exit;; # send to error input		
		*)	# this is not a parameter
	  		
			if [ $MODE -eq 0 ] ; then # it is sed pattern
				MODE=3; # MODE set to sed pattern
				PATTERN="$1"; # CREATE VARIABLE PATTERN ONLY WHEN PATTERN APPEARS 				
			else #it is filename	
				rename $1;
			fi;;
    esac
    
    shift 1;	# shift all parameteres
done
