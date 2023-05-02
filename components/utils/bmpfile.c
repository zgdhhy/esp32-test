#include "bmpfile.h"
static const char *TAG = "BMP";
/**
 * @brief 初始化图片文件
 * @param bmpfile       图片文件指针
 * @param path          图片文件路径
*/
void bmp_Init(bmpImage *bmpfile,char* path)
{
    printf("Initializing bmp\n");
    bmpfile->bmp = NULL;
    bmpfile->file = NULL;
    bmpfile->rowSize = 0;
    bmpfile->path = path;
    bmpfile->isopen = false;
}
/**
 * @brief 打开bmp图片
 * @param bmpfile       图片文件指针
*/
bool bmp_open(bmpImage *bmpfile)
{
    if (!bmpfile->isopen) {
        bmpfile->file = fopen(bmpfile->path, "rb");
        if (NULL == bmpfile->file) {
            ESP_LOGW(TAG, "File open failure");
            return false;
        }
        bmpfile->bmp = (bmpfile_t *) malloc(sizeof(bmpfile_t));
        if (NULL == bmpfile->bmp) {
            ESP_LOGW(TAG, "bmp malloc failure");
            return false;
        }
        // read bmp header
        fread(bmpfile->bmp->header.bhtype, 1, 2, bmpfile->file);
        if (bmpfile->bmp->header.bhtype[0] != 'B' || bmpfile->bmp->header.bhtype[1] != 'M') {
            ESP_LOGW(TAG, "%s File is not BMP", __FUNCTION__);
            free(bmpfile->bmp);
            fclose(bmpfile->file);
            return false;
        }
        fread(&bmpfile->bmp->header.bhsize, 4, 1, bmpfile->file);
        fread(&bmpfile->bmp->header.reserved1, 2, 1, bmpfile->file);
        fread(&bmpfile->bmp->header.reserved2, 2, 1, bmpfile->file);
        fread(&bmpfile->bmp->header.offset, 4, 1, bmpfile->file);
        ESP_LOGI(TAG, "The size of the bmp file is %dKB", bmpfile->bmp->header.bhsize / 1024);
        // read info header
        fread(&bmpfile->bmp->info.bisize, 4, 1, bmpfile->file);
        fread(&bmpfile->bmp->info.biwidth, 4, 1, bmpfile->file);
        fread(&bmpfile->bmp->info.biheight, 4, 1, bmpfile->file);
        fread(&bmpfile->bmp->info.biplanes, 2, 1, bmpfile->file);
        fread(&bmpfile->bmp->info.biBitCount, 2, 1, bmpfile->file);
        fread(&bmpfile->bmp->info.biCompression, 4, 1, bmpfile->file);
        fread(&bmpfile->bmp->info.biSizeImages, 4, 1, bmpfile->file);
        fread(&bmpfile->bmp->info.biXPels, 4, 1, bmpfile->file);
        fread(&bmpfile->bmp->info.biYPels, 4, 1, bmpfile->file);
        fread(&bmpfile->bmp->info.biClrUsed, 4, 1, bmpfile->file);
        fread(&bmpfile->bmp->info.biClrImportant, 4, 1, bmpfile->file);
        //相当于整除4 在乘4 得到的是每行像素的字节数 >= 真实
        bmpfile->rowSize = (bmpfile->bmp->info.biwidth * 3 + 3) & ~3;
        ESP_LOGI(TAG, "The width of the bmp file is %d", bmpfile->bmp->info.biwidth);
        ESP_LOGI(TAG, "The height of the bmp file is %d", bmpfile->bmp->info.biheight);
        ESP_LOGI(TAG, "The depth of the bmp file is %d", bmpfile->bmp->info.biBitCount);
        bmpfile->isopen = true;
        return true;
    }
    return false;
}
/**
 * @brief 关闭bmp图片
 * @param bmpfile       图片文件指针
*/
void bmp_close(bmpImage *bmpfile)
{
    if(bmpfile->isopen)
    {
        free(bmpfile->bmp);
        fclose(bmpfile->file);
        bmpfile->isopen = false;
    }
}
/**
 * @brief 获取bmp图像中一点的像素值
 * @param bmpfile       图片文件指针
 * @param x             图片文件指针
 * @param y             图片文件指针
 * @param buf           该点像素缓冲区
*/
void bmp_get_pixal(bmpImage *bmpfile, uint16_t x, uint16_t y, uint8_t *buf)
{
    if(bmp_open(bmpfile)||bmpfile->isopen)
    {
        if(bmpfile->bmp->info.biBitCount == 24 && bmpfile->bmp->info.biCompression == 0)
        {
            uint32_t pos = bmpfile->bmp->header.offset + 3*x + y*bmpfile->rowSize;
            fseek(bmpfile->file, pos, SEEK_SET);
            fread(buf, 3, 1, bmpfile->file);
        }
    }
}

