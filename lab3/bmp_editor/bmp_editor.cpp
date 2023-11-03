#include "stdafx.h"

#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;
#pragma pack(2)


//Заголовок файла KZS (Харитонович Захар Сергеевич)
typedef struct tKZSHEADER {
    WORD fileType;
    char author[20];
    char editor[8];
    WORD headerSize;
    DWORD size; // в пикселях
    WORD bitCount;
    DWORD width;
} sKzsHeader;

//Заголовок файла BMP
typedef struct tBITMAPFILEHEADER
{
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
}sFileHead;

//Заголовок BitMap's
typedef struct tBITMAPINFOHEADER
{
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
}sInfoHead;

struct Color
{
    BYTE blue;
    BYTE green;
    BYTE red;

    Color() {}


    Color(int red, int green, int blue) {
        this->red = red;
        this->green = green; 
        this->blue = blue;
    }
};

struct IntColor
{
    int blue;
    int green;
    int red;

    IntColor() {}

    IntColor(int red, int green, int blue) {
        this->red = red;
        this->green = green; 
        this->blue = blue;
    }
};

void bmpHeaderToKzsHeader();

bool openImage(string path); // открытие файла

bool saveImage(string path); // сохранение изображения

void showBmpHeaders(tBITMAPFILEHEADER fileHeader, tBITMAPINFOHEADER infoHeader); // вывод заголовков BMP

void showKzsHeader(tKZSHEADER header); // вывод заголовка ХЗС

void copyImage(Color* src, Color* dst); // копирование из в

void addNoise(double probability);

void imgToBrightness();

void contrastRoberts();

void limit(int limValue);

//int chooseLimit();

void showImage(string path);

string readPath();

int readNoiseProbability();

int readCenterX();

int readCenterY();

int readLimit();

void clearMemory();

void filter(int cX, int cY, int limit);

void stretchWidth(double scale);

boolean isPixelEmpty(IntColor pixel);

IntColor fromColor(Color pixel);

Color fromIntColor(IntColor pixel);

double readScale();



Color *srcImage = 0, *dstImage = 0;
BYTE *brightness = 0, *contrast;
sFileHead bmpFileHead;
sInfoHead bmpInfoHead;
sKzsHeader kzsHead;

int pixelSize = sizeof(Color);
int imgType = 0;

int width = 0, height = 0;
//---------------------------------
int main() {
    
    string path = readPath();
    openImage(path);
    showImage(path);
    

    stretchWidth(readScale());

    path = readPath();
    saveImage(path);
    showImage(path);


    clearMemory();
    return 0;
}
//---------------------------------



void stretchWidth(double scale) {
    int newWidth = width * scale;
    IntColor *image = new IntColor[height * newWidth];

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < newWidth; j++) {
            image[i * newWidth + j].red = -1;
        }
    }
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            image[i * newWidth + (int)(j * scale)] = fromColor(srcImage[i * width + j]);
        }
    }

    delete[] srcImage;
    srcImage = new Color[newWidth * height];

    for (int i = 0; i < height; i++) {
        if (isPixelEmpty(image[i * newWidth])) {
            image[i * newWidth].red = image[i * newWidth + 1].red;
            image[i * newWidth].green = image[i * newWidth + 1].green;
            image[i * newWidth].blue = image[i * newWidth + 1].blue;
        }
        srcImage[i * newWidth] = fromIntColor(image[i * newWidth]);
        for (int j = 1; j < newWidth; j++) {
            if (isPixelEmpty(image[i * newWidth + j])) {
                image[i * newWidth + j].red = image[i * newWidth + j - 1].red;
                image[i * newWidth + j].green = image[i * newWidth + j - 1].green;
                image[i * newWidth + j].blue =  image[i * newWidth + j - 1].blue;
            }
            srcImage[i * newWidth + j] = fromIntColor(image[i * newWidth + j]);
        }
    }

    
    bmpFileHead.bfSize = bmpFileHead.bfSize + (newWidth - width) * height * sizeof(Color);
    bmpInfoHead.biWidth = newWidth;
    bmpInfoHead.biSizeImage = newWidth * height;
    width = newWidth;
}

IntColor fromColor(Color pixel) {
    return IntColor(pixel.red, pixel.green, pixel.blue);
}

Color fromIntColor(IntColor pixel) {
    return Color(pixel.red, pixel.green, pixel.blue);
}


boolean isPixelEmpty(IntColor pixel) {
    return pixel.red == -1;
}

void imgToBrightness() {
    delete[] brightness;
    brightness = new BYTE[height * width];
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            brightness[i * width + j] = srcImage[i * width + j].red * 0.3 + srcImage[i * width + j].green * 0.6
                + srcImage[i * width + j].blue * 0.1;
        }
    }
}

void contrastRoberts() {
    delete[] contrast;
    contrast = new BYTE[height * width];
    for (int i = 0; i < height - 1; i++) {
        for (int j = 0; j < width - 1; j++) {
            contrast[i * width + j] = sqrt((int)((brightness[i * width + j] - brightness[(i + 1) * width + j + 1]) *
                (brightness[i * width + j] - brightness[i * width + j + 1]) +
                (brightness[i * width + j] - brightness[(i + 1) * width + j]) * 
                (brightness[i * width + j] - brightness[(i + 1) * width + j])));
        }
    }
}

void limit(int limValue) {
    /*if (limValue == -1) {
        limValue = chooseLimit();
    }*/
    for (int i = 0; i < height - 1; i++) {
        for (int j = 0; j < width - 1; j++) {
            if (contrast[i * width + j] > limValue) {
                srcImage[i * width + j].red = 0;
                srcImage[i * width + j].green = 0;
                srcImage[i * width + j].blue = 0;
            }
            else {
                srcImage[i * width + j].red = 255;
                srcImage[i * width + j].green = 255;
                srcImage[i * width + j].blue = 255;
            }
        }
    }
}

void filter(int cX, int cY, int limit) {

    // обход изображения
    for (int i = 0; i < height; i++) { // начало от ближайшего доступного к обработке пикселя
        for (int j = 0; j < width; j++) {
            int red = 0, green = 0, blue = 0;
            int count = 0;
            // обход окна
            for (int y = 0; y < 3; y++) {
                if ((i - cY + y) < 0 || (i - cY + y) >= height) continue;
                count++;
                red += srcImage[(i - cY + y) * width + j].red;
                green += srcImage[(i - cY + y) * width + j].green;
                blue += srcImage[(i - cY + y) * width + j].blue;
            }
            for (int x = 0; x < 3; x++) { // аналогично
                if ((j - cX + x) < 0 || (j - cX + x) >= width) continue;
                if (x != cX) {
                    count++;
                    red += srcImage[i * width + (j - cX + x)].red;
                    green += srcImage[i * width + (j - cX + x)].green;
                    blue += srcImage[i * width + (j - cX + x)].blue;                   
                }
            }
            red /= count; // вычисление среднего
            green /= count;
            blue /= count;
            if (srcImage[i * width + j].red - red > limit) { // сравнение с заданным порогом
                srcImage[i * width + j].red = red;
            }

            if (srcImage[i * width + j].green - green > limit) {
                srcImage[i * width + j].green = green;
            }

            if (srcImage[i * width + j].blue - blue > limit) {
                srcImage[i * width + j].blue = blue;
            }
        }
    }
}

void showBmpHeaders(tBITMAPFILEHEADER fh, tBITMAPINFOHEADER ih)
{
    cout << "Type: " << (CHAR)fh.bfType << endl;
    cout << "Size: " << fh.bfSize << endl;
    cout << "Shift of bits: " << fh.bfOffBits << endl;
    cout << "Width: " << ih.biWidth << endl;
    cout << "Height: " << ih.biHeight << endl;
    cout << "Planes: " << ih.biPlanes << endl;
    cout << "BitCount: " << ih.biBitCount << endl;
    cout << "Compression: " << ih.biCompression << endl;
}

void showKzsHeader(tKZSHEADER header) {
    cout << "Type: " << (CHAR)header.fileType << endl;
    cout << "Author: " << header.author << endl;
    cout << "Editor: " << header.editor << endl;
    cout << "Header size: " << header.headerSize << endl;
    cout << "Size: " << header.size << endl; // в пикселях
    cout << "Bitcount: " << header.bitCount << endl;
    cout << "Width: " << header.width << endl;
    WORD fileType;
    char author[20];
    char editor[8];
    WORD headerSize;
    DWORD size; // в пикселях
    BYTE bitCount;
    DWORD width;
}

bool openImage(string path) {

    ifstream imgFile;

    imgFile.open(path.c_str(), ios::in | ios::binary);
    if (!imgFile)
    {
        cout << "File isn`t open!" << endl;
        return false;
    }

    string ext = path.substr(path.find_last_of('.') + 1);
    if (ext == "bmp") imgType = 1;
    if (ext == "kzs") imgType = 2;
    switch (imgType) {
    case 1:
        imgFile.read((char*)&bmpFileHead, sizeof(bmpFileHead));
        imgFile.read((char*)&bmpInfoHead, sizeof(bmpInfoHead));

        showBmpHeaders(bmpFileHead, bmpInfoHead);

        width = bmpInfoHead.biWidth;
        height = bmpInfoHead.biHeight;
        break;
    case 2:
        imgFile.read((char*)&kzsHead, sizeof(kzsHead));

        showKzsHeader(kzsHead);

        width = kzsHead.width;
        height = kzsHead.size / width;

        break;
    default:
        return false;
        break;
    }

    srcImage = new Color[width * height];
    Color temp;
    char buf[3];

    int i = 0, j = 0;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            imgFile.read((char*)&temp, pixelSize);
            srcImage[i * width + j] = temp;
        }
        imgFile.read((char*)buf, j % 4);
    }
    imgFile.close();
}

string readPath() {
    cout << "Enter path: " << endl;
    string path;
    cin >> path;
    return path;
}

bool saveImage(string path) {
    ofstream imgFile;
    char buf[3];

    //Открыть файл на запись
    imgFile.open(path.c_str(), ios::out | ios::binary);
    if (!imgFile)
    {
        return false;
    }

    string ext = path.substr(path.find_last_of('.') + 1);
    if (ext == "bmp") imgType = 1;
    if (ext == "kzs") {
        if (imgType == 1) bmpHeaderToKzsHeader();
        imgType = 2;
    }
    switch (imgType) {
    case 1:
        imgFile.write((char*)&bmpFileHead, sizeof(bmpFileHead));
        imgFile.write((char*)&bmpInfoHead, sizeof(bmpInfoHead));
        break;
    case 2:
        imgFile.write((char*)&kzsHead, sizeof(kzsHead));
        break;
    default: 
        return false;
    }

    dstImage = new Color[width * height];
    copyImage(dstImage, srcImage);


    //Записать файл
    int i, j;
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            imgFile.write((char*)&dstImage[i * width + j], pixelSize);
        }
        imgFile.write((char*)buf, j % 4);
    }
    imgFile.close();

    return true;
}

void addNoise(double probability) { // импульсный шум
    int size = width * height;
    int count = (int)(size * probability) / 100;
    int x, y;
    long pos;
    for (int i = 0; i < count; i++)
    {
        x = rand() % width;
        y = rand() % height;
        pos = y * width + x;
        srcImage[pos].blue = rand() % 256;
        srcImage[pos].green = rand() % 256;
        srcImage[pos].red = rand() % 256;
    }
    cout << "Point was added: " << count << endl;
}


void copyImage(Color* dst, Color* src) {
    if (src != 0) {
        memcpy(dst, src, width * height * sizeof(Color));
    }
}


void clearMemory(void) {
    if (srcImage != 0)
    {
        delete[] srcImage;
    }
    
    if (dstImage != 0)
    {
        delete[] dstImage;
    }
}

void showImage(string path) {
    system(path.c_str());
}

void bmpHeaderToKzsHeader() {
    kzsHead.fileType = bmpFileHead.bfType;
    strcpy(kzsHead.author, "Zakhar");
    strcpy(kzsHead.editor, "default");
    kzsHead.headerSize = sizeof(kzsHead);
    kzsHead.bitCount = bmpInfoHead.biBitCount;
    kzsHead.size = height * width;
    kzsHead.width = width;
}

int readNoiseProbability() {
    cout << "Enter noise probability: ";
    int tmp;
    cin >> tmp;
    return tmp;
}

int readCenterX() {
    cout << "Window center X: ";
    int tmp;
    cin >> tmp;
    return tmp;
}

int readCenterY() {
    cout << "Window center Y: ";
    int tmp;
    cin >> tmp;
    return tmp;
}

int readLimit() {
    cout << "Limit: ";
    int tmp;
    cin >> tmp;
    return tmp;
}

double readScale() {
    cout << "Scale: ";
    double tmp;
    cin >> tmp;
    return tmp;
}