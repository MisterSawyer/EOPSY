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
	"./modify.sh -h"
	"./modify.sh"
	"./modify.sh -U f4.txt"
	"./modify.sh -u f4.txt"
	"./modify.sh -l F4.txt"
	"./modify.sh -a F4.txt"
	"./modify.sh -r -u d2"
	"./modify.sh -r -l D2"
	"./modify.sh -r -l D2"
	"./modify.sh -r -l -u d2"
	"./modify.sh -r -l"
	"./modify.sh -r -l -l -l -l -l D2"
)

for test in "${TESTS[@]}";
do
	performTest "$test";
done


