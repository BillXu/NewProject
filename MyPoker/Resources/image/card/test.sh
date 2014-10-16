#!/bin/sh
echo "hello world"
#ead PERSON
#echo " hello $PERSON"
myurl="www.baidu"
echo "${myurl}.com"

#for skill in as c++ java
#do 
#echo "this ${skill}language script"
#done
#readonly myurl
#unset myurl 
#echo "${myurl} is ok ?"
#myurl="hello"
echo $$
echo $0

#echo "\$*="$*
#echo "\$@"=$@
#echo " print each param from \$*"
#for var in $*
#do 
#echo "$var"
#done 

#echo "print each param from \$@"
#for var in $@
#do
#	echo "$var"
#done
DATE=`date`
echo $DATE 
a=2
b=3
result=`expr $a + $b`
echo "the sum is $result"