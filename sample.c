#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#include <ctype.h>
#include <string.h>
/*
 * マクロ定義
 */
#define min(A, B) ((A)<(B) ? (A) : (B))
#define max(A, B) ((A)>(B) ? (A) : (B))

/*
 * 画像構造体の定義
 */
typedef struct
{
    int width;              /* 画像の横方向の画素数 */
    int height;             /* 画像の縦方向の画素数 */
    int maxValue;           /* 画素の値(明るさ)の最大値 */
    unsigned char *data;    /* 画像の画素値データを格納する領域を指す */
                            /* ポインタ */
} image_t;

 

/*======================================================================
 * このプログラムに与えられた引数の解析
 *======================================================================
 */
void
parseArg(int argc, char **argv, FILE **infp, FILE **outfp)
{     
    FILE *fp;

    /* 引数の個数をチェック */
    if (argc!=3)
    {
        goto usage;
    }

    *infp = fopen(argv[1], "rb"); /* 入力画像ファイルをバイナリモードで */
                                /* オープン */

    if (*infp==NULL)		/* オープンできない時はエラー */
    {
        fputs("Opening the input file was failend\n", stderr);
        goto usage;
    }

    *outfp = fopen(argv[2], "wb"); /* 出力画像ファイルをバイナリモードで */
                                /* オープン */

    if (*outfp==NULL)		/* オープンできない時はエラー */
    {
        fputs("Opening the output file was failend\n", stderr);
        goto usage;
    }

    return;

/* このプログラムの使い方の説明 */
usage:
    fprintf(stderr, "usage : %s <input pgm file> <output pgm file>\n", argv[0]);
    exit(1);
}


/*======================================================================
 * 画像構造体の初期化
 *======================================================================
 * 画像構造体 image_t *ptImage の画素数(width × height)、階調数
 * (maxValue)を設定し、画素値データを格納するのに必要なメモリ領域を確
 * 保する。 
 */
void
initImage(image_t *ptImage, int width, int height, int maxValue)
{
    ptImage->width = width;
    ptImage->height = height;
    ptImage->maxValue = maxValue;

    /* メモリ領域の確保 */
    ptImage->data = (unsigned char *)malloc((size_t)(width * height));

    if (ptImage->data==NULL)    /* メモリ確保ができなかった時はエラー */
    {
        fputs("out of memory\n", stderr);
        exit(1);
    }
}


/*======================================================================
 * 文字列一行読み込み関数
 *======================================================================
 *   FILE *fp から、改行文字'\n'が表れるまで文字を読み込んで、char型の
 * メモリ領域 char *buf に格納する。1行の長さが n 文字以上の場合は、先
 * 頭から n-1 文字だけを読み込む。
 *   読み込んだ文字列の先頭が '#' の場合は、さらに次の行を読み込む。
 *   正常に読み込まれた場合は、ポインタ buf を返し、エラーや EOF (End
 * Of File) の場合は NULL を返す。
 */
char *
readOneLine(char *buf, int n, FILE *fp)
{
    char *fgetsResult;

    do
    {
        fgetsResult = fgets(buf, n, fp);

        // if(isspace(buf[0]))
        // {
        //     printf("%s", buf);
        //     printf("True\n");
        //     printf("サイズ%ld\n",strlen(buf));
        // }
        // else
        // {
        //     printf("%s", buf);
        //     printf("False\n");
        //     printf("サイズ%ld\n",strlen(buf));
        // }

    } while(fgetsResult!=NULL && (buf[0]=='#' || buf[0]=='\n' ) );
            /* エラーや EOF ではなく、かつ、先頭が '#' 改行 の時は、次の行 */
            /* を読み込む */

    return fgetsResult;
}   


/*======================================================================
 * PGM-RAW フォーマットのヘッダ部分の読み込みと画像構造体の初期化
 *======================================================================
 *   PGM-RAW フォーマットの画像データファイル FILE *fp から、ヘッダ部
 * 分を読み込んで、その画像の画素数、階調数を調べ、その情報に従って、
 * 画像構造体 image_t *ptImage を初期化する。
 *   画素値データを格納するメモリ領域も確保し、この領域の先頭を指すポ
 * インタを ptImage->data に格納する。
 *
 * !! 注意 !!
 *   この関数は、ほとんどの場合、正しく動作するが、PGM-RAWフォーマット
 * の正確な定義には従っておらず、正しいPGM-RAWフォーマットのファイルに
 * 対して、不正な動作をする可能性がある。なるべく、本関数をそのまま使
 * 用するのではなく、正しく書き直して利用せよ。
 */
void
readPgmRawHeader(FILE *fp, image_t *ptImage)
{
    int width, height, maxValue;
    char buf[128];

    /* マジックナンバー(P5) の確認 */
    if(readOneLine(buf, 128, fp)==NULL)
    {
        goto error;
    }
    if (buf[0]!='P' || buf[1]!='5')
    {
        goto error;
    }

    /* 画像サイズの読み込み */
    if (readOneLine(buf, 128, fp)==NULL)
    {
        goto error;
    }
    if (sscanf(buf, "%d %d", &width, &height) != 2)
    {
        goto error;
    }
    if ( width<=0 || height<=0)
    {
        goto error;
    }

    /* 最大画素値の読み込み */
    if (readOneLine(buf, 128, fp)==NULL)
    {
        goto error;
    }
    if (sscanf(buf, "%d", &maxValue) != 1)
    {
        goto error;
    }
    if ( maxValue<=0 || maxValue>=256 )
    {
        goto error;
    }

    /* 画像構造体の初期化 */
    initImage(ptImage, width, height, maxValue);

    return;

/* エラー処理 */
error:
    fputs("Reading PGM-RAW header was failed\n", stderr);
    exit(1);
}
     

/*======================================================================
 * PGM-RAWフォーマットの画素値データの読み込み
 *======================================================================
 *   入力ファイル FILE *fp から総画素数分の画素値データを読み込んで、
 * 画像構造体 image_t *ptImage の data メンバーが指す領域に格納する
 */
void
readPgmRawBitmapData(FILE *fp, image_t *ptImage)
{
    if( fread(ptImage->data, sizeof(unsigned char),
            ptImage->width * ptImage->height, fp)
            != ptImage->width * ptImage->height )
    {
        /* エラー */
        fputs("Reading PGM-RAW bitmap data was failed\n", stderr);
        exit(1);
    }
}


/*======================================================================
 * フィルタリング(ネガポジ反転)
 *======================================================================
 *   画像構造体 image_t *originalImage の画像をフィルタリング(ネガポジ
 * 反転)して、image_t *resultImage に格納する
 */
void
filteringImage(image_t *resultImage, image_t *originalImage)
{
    int     x, y;
    int     width, height;

    /* originalImage と resultImage のサイズが違う場合は、共通部分のみ */
    /* を処理する。*/
    width = min(originalImage->width, resultImage->width);
    height = min(originalImage->height, resultImage->height);

    for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {
            resultImage->data[x+resultImage->width*y]
                    = ( originalImage->maxValue -originalImage->data[x+originalImage->width*y] )
                            *resultImage->maxValue/originalImage->maxValue;
        }
    }
}
/*======================================================================
 *  数字をunsighed char型に合わせて
 *  0-255までの数字にし，小数は四捨五入して返す 
 *======================================================================
 */
int int_8bit(double num){
    if(num < 0)return 0;
    else if(255 < num ) return 255;
    else return round(num);
}

/*======================================================================
 * imageのx,y座標の画素値を取得し返す
 *======================================================================*/
int
get_pixel_value(image_t *image, int x, int y){
    if(x < 0 || image->width <= x)return 0;
    if(y < 0 || image->height <= y)return 0;
    return image->data[x + image->width * y];
}

/*======================================================================
 * resultImageの端の処理を行う
 *======================================================================*/
void
fillForEdge(image_t *resultImage, image_t *originalImage, int directions, int height , int width){
    int x,y;
    // 上下左右の端の処理
    if(directions == 4){
        for(y=0; y<height; y++){
            resultImage->data[0+resultImage->width*y] = get_pixel_value(resultImage, 1, y);
            resultImage->data[(width-1) + resultImage->width*y] = get_pixel_value(resultImage, width-2, y);
        }
        for(x=0; x<width; x++){
            resultImage->data[x+resultImage->width*0] = get_pixel_value(resultImage, x, 1);
            resultImage->data[x+resultImage->width*(height-1)] = get_pixel_value(resultImage, x, height-2);
        }
    }
    // 右と下の端の処理
    else if(directions == 2){
        for(y=0; y<height; y++){
            resultImage->data[(width-1) + resultImage->width*y] = get_pixel_value(resultImage, width-2, y);
        }
        for(x=0; x<width; x++){
            resultImage->data[x+resultImage->width*(height-1)] = get_pixel_value(resultImage, x, height-2);
        }
    }    
}
// (0) (1)
void
prewittImage(image_t *resultImage, image_t *originalImage, int method)
{
    int     x, y;
    int     width, height;

    /* originalImage と resultImage のサイズが違う場合は、共通部分のみ */
    /* を処理する。*/
    width = min(originalImage->width, resultImage->width);
    height = min(originalImage->height, resultImage->height);

    for(y=1; y<height-1; y++)
    {
        for(x=1; x<width-1; x++)
        {
            int horizon=0;
            int vertical=0;
            for(int i=-1; i<2; i++){
                for(int j=-1; j<2; j++){
                    horizon += i*get_pixel_value(originalImage, x+i, y+j);
                    vertical += j*get_pixel_value(originalImage, x+i, y+j);
                }
            }
            // 式(2)(3)の分岐を行う
            if(method == 0) resultImage->data[x+resultImage->width*y] =int_8bit( sqrt(pow(horizon,2)+pow(vertical,2)) );
            if(method == 1) resultImage->data[x+resultImage->width*y] = int_8bit( abs(horizon) + abs(vertical) ); 
        }
    }
    fillForEdge(resultImage, originalImage, 4, height, width);
}




// (2) (3)
void
sobelImage(image_t *resultImage, image_t *originalImage, int method)
{
    int     x, y;
    int     width, height;
    int filter_horizon[9] = {-1, 0, 1, 
                             -2, 0, 2, 
                             -1, 0, 1};
    int filter_vertical[9] = {-1, -2, -1,
                              0, 0, 0,
                              1, 2, 1};
    /* originalImage と resultImage のサイズが違う場合は、共通部分のみ */
    /* を処理する。*/
    width = min(originalImage->width, resultImage->width);
    height = min(originalImage->height, resultImage->height);


    for(y=1; y<height-1; y++)
    {
        for(x=1; x<width-1; x++)
        {   
            int horizon=0;
            int verti=0;
            for(int i=-1; i<2; i++){
                for(int j=-1; j<2; j++){
                    horizon+= filter_horizon[(i+1) + 3*(j+1)] * get_pixel_value(originalImage, x+i, y+j);
                    verti+= filter_vertical[(i+1) + 3*(j+1)] * get_pixel_value(originalImage, x+i, y+j);
                }
            }
            // 式(2)(3)の分岐を行う
            if(method == 0)resultImage->data[x+resultImage->width*y] = int_8bit( sqrt(pow(horizon,2)+pow(verti,2)) );
            if(method == 1)resultImage->data[x+resultImage->width*y] = int_8bit( abs(horizon) + abs(verti) );
        }
    }
    fillForEdge(resultImage, originalImage, 4, height, width);
}



// (4)
void
fourLapImage(image_t *resultImage, image_t *originalImage)
{
    int     x, y;
    int     width, height;
    int filter[9] = {0, 1, 0,
                     1, -4, 1,
                     0, 1, 0};

    /* originalImage と resultImage のサイズが違う場合は、共通部分のみ */
    /* を処理する。*/
    width = min(originalImage->width, resultImage->width);
    height = min(originalImage->height, resultImage->height);


    for(y=1; y<height-1; y++)
    {
        for(x=1; x<width-1; x++)
        {   
            
            if(x==0 || y ==0 || y==height-1 || x==width-1){}
            else{
                int tmp=0;

                for(int i=-1; i<2; i++){
                    for(int j=-1; j<2; j++){
                        tmp+= filter[(i+1) + 3*(j+1)] * get_pixel_value(originalImage, x+i, y+j);
                    }
                }

                resultImage->data[x+resultImage->width*y] = int_8bit( tmp );
            }
        }
    }
    fillForEdge(resultImage, originalImage, 4, height, width);
}

// (5)
void
eightLapImage(image_t *resultImage, image_t *originalImage)
{
    int     x, y;
    int     width, height;
    int filter[9] = {1, 1, 1,
                     1, -8, 1, 
                     1, 1, 1};

    /* originalImage と resultImage のサイズが違う場合は、共通部分のみ */
    /* を処理する。*/
    width = min(originalImage->width, resultImage->width);
    height = min(originalImage->height, resultImage->height);


    for(y=1; y<height-1; y++)
    {
        for(x=1; x<width-1; x++)
        {   
            int tmp=0;

            for(int i=-1; i<2; i++){
                for(int j=-1; j<2; j++){
                    tmp+= filter[(i+1) + 3*(j+1)] * get_pixel_value(originalImage, x+i, y+j);
                }
            }

            resultImage->data[x+resultImage->width*y] = int_8bit( tmp );
            
        }
    }
    fillForEdge(resultImage, originalImage, 4, height, width);
}

// (6)
void
robertsImage(image_t *resultImage, image_t *originalImage)
{
    int     x, y;
    int     width, height;

    /* originalImage と resultImage のサイズが違う場合は、共通部分のみ */
    /* を処理する。*/
    width = min(originalImage->width, resultImage->width);
    height = min(originalImage->height, resultImage->height);

    for(y=0; y<height-1; y++)
    {
        for(x=0; x<width-1; x++)
        {
            double tmp;
            tmp = sqrt(pow(sqrt(get_pixel_value(originalImage, x, y)) - sqrt(get_pixel_value(originalImage, x+1, y+1)),2)+
                        pow(sqrt(get_pixel_value(originalImage, x, y+1)) - sqrt(get_pixel_value(originalImage, x+1, y)),2));
            resultImage->data[x+resultImage->width*y] = int_8bit( tmp );
                            
        }
    }
    fillForEdge(resultImage, originalImage, 2, height, width);
}

// (7)
void
forsenImage(image_t *resultImage, image_t *originalImage)
{
    int     x, y;
    int     width, height;

    /* originalImage と resultImage のサイズが違う場合は、共通部分のみ */
    /* を処理する。*/
    width = min(originalImage->width, resultImage->width);
    height = min(originalImage->height, resultImage->height);

    for(y=0; y<height-1; y++)
    {
        for(x=0; x<width-1; x++)
        {
            int tmp;
            tmp = abs(get_pixel_value(originalImage, x, y) - get_pixel_value(originalImage, x+1, y+1))+
                abs(get_pixel_value(originalImage, x, y+1) - get_pixel_value(originalImage, x+1, y));

            resultImage->data[x+resultImage->width*y] = int_8bit( tmp );
        
        }
    }
    fillForEdge(resultImage, originalImage, 2, height, width);
}

// (8)
void
rangeImage(image_t *resultImage, image_t *originalImage)
{
    int     x, y;
    int     width, height;
    
    

    /* originalImage と resultImage のサイズが違う場合は、共通部分のみ */
    /* を処理する。*/
    width = min(originalImage->width, resultImage->width);
    height = min(originalImage->height, resultImage->height);


    for(y=1; y<height-1; y++)
    {
        for(x=1; x<width-1; x++)
        {   
                int tmp=0;
                int MAX = 0;
                int MIN = 255;

                for(int i=-1; i<2; i++){
                    for(int j=-1; j<2; j++){
                        MAX = max(get_pixel_value(originalImage, x+i, y+j), MAX);
                        MIN = min(get_pixel_value(originalImage, x+i, y+j), MIN);
                    }
                }
                resultImage->data[x+resultImage->width*y] = int_8bit( MAX - MIN );
            
        }
    }
    fillForEdge(resultImage, originalImage, 4, height, width);
}

double interclass_distribution(double p_i[256], int k){
    double omega_0 = 0;
    double omega_1 = 0;
    for(int i=0; i<=k; i++){
        omega_0 += p_i[i];
    }
    for(int i=k+1; i<=255; i++){
        omega_1 += p_i[i];
    }

    double mu_0 = 0;
    double mu_1 = 0;
    double mu_T = 0;
    for(int i=0; i<=k; i++){
        mu_0 += i*p_i[i];
    }
    mu_0 = omega_0==0 ? 0 : mu_0/omega_0;

    for(int i=k+1; i<=255; i++){
        mu_1 += i*p_i[i];
    }
    mu_1 = omega_1==0 ? 0 : mu_1/omega_1;

    for(int i=0; i<=255; i++){
        mu_T += i*p_i[i];
    }

    return omega_0*pow(mu_0 - mu_T,2) + omega_1*pow(mu_1 - mu_T,2);
}

void
bi_image(image_t *resultImage, image_t *originalImage, int T)
{
    int     x, y;
    int     width, height;
    width = min(originalImage->width, resultImage->width);
    height = min(originalImage->height, resultImage->height);

    // Tによって2値化
    for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {   
            int bi;
            if(get_pixel_value(originalImage, x, y) <= T) bi=0;
            else bi=255;
            resultImage->data[x+resultImage->width*y] = bi;
        }
    }  
}

void histogram(double array[256]){
    // make csv file
    FILE *fp;
    fp = fopen("histogram.csv", "w");
    for(int i=0; i<256; i++){
        fprintf(fp, "%d,%d\n", i, (int)array[i]);
    }
    fclose(fp);
}

int
k_image(image_t *resultImage, image_t *originalImage)
{
    int     x, y;
    int     width, height;
    double p_i[256];
    for(int i=0; i<256; i++)
        p_i[i] = 0;
    

    /* originalImage と resultImage のサイズが違う場合は、共通部分のみ */
    /* を処理する。*/
    width = min(originalImage->width, resultImage->width);
    height = min(originalImage->height, resultImage->height);

    // 画素値の分布を計算
    for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {   
            p_i[originalImage->data[x+originalImage->width*y]]+=1;
        }
    }

    // 画素値の分布をcsvファイルに出力
    histogram(p_i);
    

    for(int i=0; i<256; i++){
        p_i[i] = p_i[i]/width/height;
    }


    double MAX = DBL_MIN;
    int max_k;
    for(int k=1; k<256; k++){
        if(interclass_distribution(p_i, k)>MAX){
            MAX = interclass_distribution(p_i, k);
            max_k = k;
        }
    }
    printf("k = %d\n", max_k);

    return max_k;

}

void exHistogram(image_t *resultImage){
    int width = resultImage->width;
    int height = resultImage->height;
    int max = 0;
    int min = 255;
    for(int y=0; y<height; y++){
        for(int x=0; x<width; x++){
            if(max < get_pixel_value(resultImage, x, y) ){
                max = get_pixel_value(resultImage, x, y);
            }
            if(min > get_pixel_value(resultImage, x, y) ){
                min = get_pixel_value(resultImage, x, y);
            }
        }
    }

    for(int y=0; y<height; y++){
        for(int x=0; x<width; x++){
            resultImage->data[x+resultImage->width*y] = int_8bit( (get_pixel_value(resultImage, x, y)-min)*255/(max-min) );
        }
    }
 

}


/*======================================================================
 * PGM-RAW フォーマットのヘッダ部分の書き込み
 *======================================================================
 *   画像構造体 image_t *ptImage の内容に従って、出力ファイル FILE *fp
 * に、PGM-RAW フォーマットのヘッダ部分を書き込む。
 */
void
writePgmRawHeader(FILE *fp, image_t *ptImage)
{
    /* マジックナンバー(P5) の書き込み */
    if(fputs("P5\n", fp)==EOF)
    {
        goto error;
    }

    /* 画像サイズの書き込み */
    if (fprintf(fp, "%d %d\n", ptImage->width, ptImage->height)==EOF)
    {
        goto error;
    }

    /* 画素値の最大値を書き込む */
    if (fprintf(fp, "%d\n", ptImage->maxValue)==EOF)
    {
        goto error;
    }

    return;

error:
    fputs("Writing PGM-RAW header was failed\n", stderr);
    exit(1);
}


/*======================================================================
 * PGM-RAWフォーマットの画素値データの書き込み
 *======================================================================
 *   画像構造体 image_t *ptImage の内容に従って、出力ファイル FILE *fp
 * に、PGM-RAW フォーマットの画素値データを書き込む
 */
void
writePgmRawBitmapData(FILE *fp, image_t *ptImage)
{
    if( fwrite(ptImage->data, sizeof(unsigned char),
            ptImage->width * ptImage->height, fp)
            != ptImage->width * ptImage->height )
    {
        /* エラー */
        fputs("Writing PGM-RAW bitmap data was failed\n", stderr);
        exit(1);
    }
}
 

/*
 * メイン
 */
int
main(int argc, char **argv)
{
    image_t originalImage, resultImage;
    FILE *infp, *outfp;
  
    /* 引数の解析 */
    parseArg(argc, argv, &infp, &outfp);

    /* 元画像の画像ファイルのヘッダ部分を読み込み、画像構造体を初期化 */
    /* する */
    readPgmRawHeader(infp, &originalImage);

    /* 元画像の画像ファイルのビットマップデータを読み込む */
    readPgmRawBitmapData(infp, &originalImage);

    /* 結果画像の画像構造体を初期化する。画素数、階調数は元画像と同じ */
    initImage(&resultImage, originalImage.width, originalImage.height,
            originalImage.maxValue);

    /* フィルタリング説明 */
    printf("(0) Prewitt フィルタ + 式 (2)\n");
    printf("(1) Prewitt フィルタ + 式 (3)\n");
    printf("(2) Sobel フィルタ + 式 (2)\n");
    printf("(3) Sobel フィルタ + 式 (3)\n");
    printf("(4) 4 近傍ラプラシアンフィルタ\n");
    printf("(5) 8 近傍ラプラシアンフィルタ\n");
    printf("(6) Robers フィルタ\n");
    printf("(7) Forsen フィルタ\n");
    printf("(8) レンジフィルタ\n");
    printf("(9) 2値化\n");

    // フィルタを選択
    int filter, T;
    printf("選択してください:");
    scanf("%d", &filter);
    printf("\n");

    // 選択されたフィルタを適用
    switch (filter)
    {
    case 0:
        prewittImage(&resultImage, &originalImage, 0);
        break;

    case 1:
        prewittImage(&resultImage, &originalImage, 1);
        break;

    case 2:
        sobelImage(&resultImage, &originalImage, 0);
        break;

    case 3:
        sobelImage(&resultImage, &originalImage, 1);
        break;

    case 4:
        fourLapImage(&resultImage, &originalImage);
        break;

    case 5:
        eightLapImage(&resultImage, &originalImage);
        break;

    case 6:
        robertsImage(&resultImage, &originalImage);
        // https://codezine.jp/article/detail/214
        exHistogram(&resultImage); 
        break;

    case 7:
        forsenImage(&resultImage, &originalImage);
        break;

    case 8:
        rangeImage(&resultImage, &originalImage);
        break;
    
    case 9:
        T = k_image(&resultImage, &originalImage);
        bi_image(&resultImage, &originalImage, T);
        break;

    default:
        break;
    }
    
    /* 画像ファイルのヘッダ部分の書き込み */
    writePgmRawHeader(outfp, &resultImage);

    /* 画像ファイルのビットマップデータの書き込み */
    writePgmRawBitmapData(outfp, &resultImage);

    return 0;
}
