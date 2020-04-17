#!/bin/bash

performTest()
{
	echo;	
	echo "Currently performing: $1";
	echo " --- Output --- "; 
	time $1;
	read -p " --- To continue press enter --- "
	echo;
}

declare -a TESTS=(
	"./modify.sh -h" # help
	"./modify.sh" # no arguments
	"./modify.sh -U f4.txt" # wrong argument
	"./modify.sh -u f4.txt" # uppercase 1 file
	"./modify.sh -l F4.txt" # lower case 1 file
	"./modify.sh -u f4.txt f3.txt f2.txt" # uppercase multiple files
	"./modify.sh -l F4.txt F3.txt F2.txt" # lowercase multiple files
	"./modify.sh -u f0.txt d1" # uppercase file and dir at the same time
	"./modify.sh -l D1 F0.txt" # lowercase file and dir at the same time
	"./modify.sh s/alfa/beta/ alfa1.txt" # good sed pattern one files
	"./modify.sh s/alfa/beta/ alfa2.txt alfa3.txt alfa4.txt" # sed pattern multiple files
	"./modify.sh s/beta/alfa/ beta1.txt beta2.txt beta3.txt beta4.txt" # go back
	"./modify.sh s/alfa/beta/ alfa1.txt alfa2.txt alfaDIR" # sed pattern file and DIR
	"./modify.sh s/beta/alfa/ betaDIR beta2.txt beta1.txt" # go back
	"./modify.sh -u d1" # uppercase directory	
	"./modify.sh -l D1" # lowercase directory
	"./modify.sh -r -u d2" # recursive uppercase in directory
	"./modify.sh -r -l D2" # recursive lowercase in directory
	"./modify.sh -r -l -u d2" #-u and -l in the same line
	"./modify.sh -r -u -l D2" #-u and -l in the same line
	"./modify.sh -r s/alfa/beta/ d2" # recursive sed pattern
	"./modify.sh -r s/beta/alfa/ d2" # go back
	"./modify.sh -r -l" #no files
	"./modify.sh -r -u -u -u -u -u d2" #multiple arguments
	"./modify.sh -r -l -l -l -l -l D2" #multiple arguments
	"./modify.sh -u f1.txt" #making test case
	"./modify.sh -l F1.txt -u f2.txt" # different options for different files
	"./modify.sh -l F2.txt" # go back

)

for test in "${TESTS[@]}";
do
	performTest "$test";
done


