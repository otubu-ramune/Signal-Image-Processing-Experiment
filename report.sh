#!/bin/bash
gcc sample.c -o sample -lm

filters[0]="Prewitt フィルタ + 式 (2)"
filters[1]="Prewitt フィルタ + 式 (3)"
filters[2]="Sobel フィルタ + 式 (2)"
filters[3]="Sobel フィルタ + 式 (3)"
filters[4]="4近傍ラプラシアンフィルタ"
filters[5]="8近傍ラプラシアンフィルタ"
filters[6]="Robers フィルタ"
filters[7]="Forsen フィルタ"
filters[8]="レンジフィルタ"
filters[9]="2値化"

fil_num[0]="a"
fil_num[1]="b"
fil_num[2]="c"
fil_num[3]="d"
fil_num[4]="e"
fil_num[5]="f"
fil_num[6]="g"
fil_num[7]="h"
fil_num[8]="i"


# for i in `seq 9`
# do
# dec=`expr $i - 1`
# echo  "($dec) ${filters[$i-1]}"
# done
# echo
# echo -n Select Program: 
# read num


# if [ -d ./edge ] ;then
# echo "ディレクトリが存在します"
# else
# mkdir edge
# fi
# if [ -d ./bi_image ] ;then
# echo "ディレクトリが存在します"
# else
# mkdir bi_image
# fi
# # sleep 3


for i in `seq 9`
do
dec=`expr $i - 1`
file=${fil_num[$dec]}
echo
echo ${filters[$dec]}
echo $dec | ./sample sampleImages/sample4.pgm out.pgm
echo 2値化
echo 9 | ./sample out.pgm out2.pgm
# echo $dec | ./sample sampleImages/sample10.pgm result/$file/sample10.pgm
# echo 2値化
# echo 9 | ./sample result/$file/sample10.pgm result/$file/sample10_bi.pgm

echo -n Next 
read wait
done

echo
echo mode:${filters[$num]}