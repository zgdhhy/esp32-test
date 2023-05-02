#include "font.h"

static const char *TAG = "FONT";
/**
 * @brief 初始化字体文件
 * @param fp 字体文件指针
 * @param path 字体文件路径
 * @param wight 字体文件的宽
 * @param height 字体文件的高
*/
void InitFont(FontFile* fp, const char* path, uint8_t wight, uint8_t height)
{
    ESP_LOGI(TAG,"Initializing font");
    fp->file = NULL;
    fp->path = path;
    fp->wight = wight;
    fp->height = height;
    fp->isopen = false;
    fp->GBKoffset = (0X7F-0X20)*wight*height/16;
}
/**
 * @brief 打开字体文件
 * @param fp 字体文件指针
 * @return 文件是否打开成功
*/
bool OpenFont(FontFile* fp)
{
    if(!fp->isopen)
    {
        fp->file = fopen(fp->path,"r");
        if(fp->file == NULL)
        {
            ESP_LOGI(TAG,"open false");
            return false;
        }
        fp->isopen = true;
        return true;
    }
    return false;
}
/**
 * @brief 关闭字体文件
 * @param fp 字体文件指针
*/
void CloseFont(FontFile* fp)
{
    if(fp->isopen){
        fclose(fp->file);
        fp->isopen = false;
    }
}
/**
 * @brief 得到打印的字符对应编码取模
 * @param fp[in] 字体文件指针
 * @param font[in] 要打印的字符(支持ascii和gbk)
 * @param fcode[out] 字体内容结构体指针
 * @return 获取是否成功
*/
bool GetFontCode(FontFile *fp,char *font,FontCode *fcode)
{
    if(OpenFont(fp)||fp->isopen)
    {
        uint8_t FontH = *(font);
        if(FontH >= 0X81 && FontH <= 0XFE)
        {
            fcode->code = NULL;
            fcode->wight = fp->wight;
            fcode->height = fp->height;
            fcode->size = fp->height * fp->wight / 8;
            fcode->code = (uint8_t*)malloc(fcode->size);
            return GetGBKCode(fp,font,fcode);
        }
        else if(FontH >= 0X20 && FontH <= 0X7E)
        {
            fcode->code = NULL;
            fcode->wight = fp->wight/2;
            fcode->height = fp->height;
            fcode->size = fp->height * fp->wight / 16;
            fcode->code = (uint8_t*)malloc(fcode->size);
            return GetASCIICode(fp,font,fcode);
        }
        else
        {
            ESP_LOGW(TAG,"out of character range");
            return false;
        }
    }
    return false;
}
/**
 * @brief 得到GBK字符对应的编码取模
 * @param fp[in] 字体文件指针
 * @param font[in] GBK字符
 * @param fcode[out] 字体内容结构体指针
 * @return 获取是否成功
*/
bool GetGBKCode(FontFile *fp,char *font,FontCode *fcode)
{
    uint8_t FontH = *(font) - 0X81;
    uint8_t FontL = *(font + 1) - 0X40;
    uint32_t offset = (uint32_t)(192 * FontH + FontL) * fcode->size + fp->GBKoffset;

    if (0 != fseek(fp->file, offset, SEEK_SET))
    {
        ESP_LOGW(TAG,"Fseek error");
        return false;
    }
    if(fread(fcode->code, 1, fcode->size, fp->file) != fcode->size)
    {
        ESP_LOGW(TAG,"fread failed");
        return false;
    }
    return true;
}
/**
 * @brief 得到ascii字符对应的编码取模
 * @param fp[in] 字体文件指针
 * @param font[in] ascii字符
 * @param fcode[out] 字体内容结构体指针
 * @return 获取是否成功
*/
bool GetASCIICode(FontFile *fp,char *font,FontCode *fcode)
{
    uint32_t offset = (uint32_t)(*font - ' ') * fcode->size;
    if (0 != fseek(fp->file, offset, SEEK_SET))
    {
        ESP_LOGW(TAG,"Fseek error");
        return false;
    }
    if(fread(fcode->code, 1, fcode->size, fp->file) != fcode->size)
    {
        ESP_LOGW(TAG,"fread failed");
        return false;
    }
    return true;
}
/**
 * @brief 在字符显示中添加一个下划线
 * @param fcode 字体内容结构体指针
*/
void AddDrowline(FontCode *fcode)
{
    uint8_t offset = (fcode->height-1)*fcode->wight/8;
    fcode->code[offset++] = 0xFF;
    fcode->code[offset] = 0xFF;
}
/**
 * @brief 将字符显示进行反转
 * @param fcode 字体内容结构体指针
*/
void InvertFont(FontCode *fcode)
{
    for(uint8_t i=0;i<fcode->size;i++) fcode->code[i] = ~fcode->code[i];
}
/**
 * @brief 打印字符
 * @param fcode 字体内容结构体指针
*/
void ShowFont(FontCode *fcode)
{
    uint8_t *fp = fcode->code;
    for (int i = 0; i < fcode->height; i++)
    {
        for (int j = 0; j < fcode->wight; j++)
        {
            if (*fp & (0X80 >> (j % 8))) printf("*");
            else printf(" ");
            if (j % 8 == 7) fp++;
        }
        printf("\r\n");
    }
}