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

for i in `seq 9`
do
dec=`expr $i - 1`
echo  "($dec) ${filters[$i-1]}"
done
echo
echo -n Select Program: 
read num


if [ -d ./edge ] ;then
echo "ディレクトリが存在します"
else
mkdir edge
fi
if [ -d ./bi_image ] ;then
echo "ディレクトリが存在します"
else
mkdir bi_image
fi
sleep 3
for i in `seq 13`
do
echo $num | ./sample sampleImages/sample$i.pgm edge/sample$i.pgm
echo 9 | ./sample edge/sample$i.pgm bi_image/sample$i.pgm
done

echo
echo mode:${filters[$num]}