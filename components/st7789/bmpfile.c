#include "bmpfile.h"
static const char *TAG = "BMP";

/**
 * @brief 打开一张bmp图片
 * @param tft       tft设备
 * @param file      文件路径
 * @param width     展示的宽
 * @param height    展示的高
 * 
*/
void bmp_show(TFT_t *tft,char* file,uint16_t width,uint16_t height)
{
    TickType_t startTick, endTick, diffTick;
	startTick = xTaskGetTickCount();

	FILE* fbmp = fopen(file, "rb");
	if (NULL == fbmp) 
    {
        ESP_LOGW(TAG,"File open failure");
        return;
    }
	bmpfile_t* bmp = (bmpfile_t*)malloc(sizeof(bmpfile_t));
	if (NULL == bmp)
    {
        ESP_LOGW(TAG,"bmp malloc failure");
        return;
    }
	// read bmp header
	fread(bmp->header.bhtype, 1, 2, fbmp);
	if (bmp->header.bhtype[0] != 'B' || bmp->header.bhtype[1] != 'M') {
		ESP_LOGW(TAG,"%s File is not BMP", __FUNCTION__);
		free(bmp);
		fclose(fbmp);
		return;
	}
	fread(&bmp->header.bhsize, 4, 1, fbmp);
	ESP_LOGI(TAG,"The size of the bmp file is %dKB", bmp->header.bhsize/1024);
	fread(&bmp->header.reserved1, 2, 1, fbmp);
	fread(&bmp->header.reserved2, 2, 1, fbmp);
	fread(&bmp->header.offset, 4, 1, fbmp);

	// read info header
	fread(&bmp->info.bisize, 4, 1, fbmp);
	fread(&bmp->info.biwidth, 4, 1, fbmp);
	fread(&bmp->info.biheight, 4, 1, fbmp);
	fread(&bmp->info.biplanes, 2, 1, fbmp);
	fread(&bmp->info.biBitCount, 2, 1, fbmp);
	fread(&bmp->info.biCompression, 4, 1, fbmp);
	fread(&bmp->info.biSizeImages, 4, 1, fbmp);
	fread(&bmp->info.biXPels, 4, 1, fbmp);
	fread(&bmp->info.biYPels, 4, 1, fbmp);
	fread(&bmp->info.biClrUsed, 4, 1, fbmp);
	fread(&bmp->info.biClrImportant, 4, 1, fbmp);
    ESP_LOGI(TAG,"The width of the bmp file is %d", bmp->info.biwidth);
	ESP_LOGI(TAG,"The height of the bmp file is %d", bmp->info.biheight);
    ESP_LOGI(TAG,"The depth of the bmp file is %d", bmp->info.biBitCount);

	if(bmp->info.biBitCount == 24 && bmp->info.biCompression == 0)
	{
        //相当于整除4 在乘4 得到的是每行像素的字节数 >= 真实
		uint32_t rowSize = (bmp->info.biwidth * 3 + 3) & ~3;
		uint32_t w = bmp->info.biwidth;
		uint32_t h = bmp->info.biheight;
		int _x,_y,_w,_cols,_cole,_rows,_rowe;
		if (width >= w) 
		{
			_x = (width - w) / 2; 
			_w = w;
			_cols = 0;
			_cole = w - 1;
		}
		else 
		{
			_x = 0;
			_w = width;
			_cols = (w - width) / 2;
			_cole = _cols + width - 1;
		}
		if (height >= h) 
		{
			_y = (height - h) / 2;
			_rows = 0;
			_rowe = h - 1;
		}
		else 
		{
			_y = 0;
			_rows = (h - height) / 2;
			_rowe = _rows + height - 1;
		}
        ESP_LOGI(TAG,"x=%d y=%d w=%d cols=%d cole=%d rows=%d rowe=%d", _x, _y, _w, _cols, _cole,_rows,_rowe);
		uint16_t* colors = (uint16_t*)malloc(sizeof(uint16_t) * _w);
		if (NULL == colors)
        {
            ESP_LOGW(TAG,"colors malloc failure");
            return;
        }
        uint16_t* colorsptr;
        uint8_t sdbuffer[60];
		for(int row = _rows; row <= _rowe; row++)
		{
			int pos = bmp->header.offset + (h - row - 1) * rowSize;
			fseek(fbmp, pos, SEEK_SET);
			int buffidx = sizeof(sdbuffer);
            colorsptr = colors;
			for (int col = _cols; col <= _cole; col++)
			{
				if (buffidx >= sizeof(sdbuffer)) 
				{
					fread(sdbuffer, sizeof(sdbuffer), 1, fbmp);
					buffidx = 0;
				}
				uint8_t b = sdbuffer[buffidx++];
				uint8_t g = sdbuffer[buffidx++];
				uint8_t r = sdbuffer[buffidx++];
				*colorsptr++ = rgb565_conv(r, g, b);
			}
			ESP_LOGD(__FUNCTION__, "ST7789_DrawMultipoint _x=%d _y=%d row=%d", _x, _y, row);
			ST7789_DrawMultipoint(tft, _x, _y++, _w, colors); 
		}
		free(colors);
	}
	free(bmp);
	fclose(fbmp);
    endTick = xTaskGetTickCount();
	diffTick = endTick - startTick;
    ESP_LOGI(__FUNCTION__, "elapsed time[ms]:%d",diffTick*portTICK_PERIOD_MS);
}