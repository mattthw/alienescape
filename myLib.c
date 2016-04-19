#include "myLib.h"
#include "bg.h"

unsigned short *videoBuffer = (unsigned short *)0x6000000;

void waitForVblank()
{
    while(SCANLINECOUNTER > 160);
    while(SCANLINECOUNTER < 160);
}
void setPixel(int r, int c, unsigned short color)
{
	videoBuffer[OFFSET(r, c, 240)] = color;
}

void drawRect(int row, int col, int height, int width, unsigned short color)
{
	for(int r=0; r<height; r++)
	{
		for(int c=0; c<width; c++)
		{
			setPixel(row+r, col+c, color);
		}
	}
}

void delay(int n)
{
	volatile int x = 0;
	for(int i=0; i<n*300000; i++)
	{
		x++;
	}
}
//create planet floor. build to be able to
//add more things.
MAPOBJ defineMapElems() {
    //default element properties
    MAPOBJ basic = {
        .x = 0, .y = 0,
        .width = 240, .height = 160,
        .color = RED
    };
    MAPOBJ mapobject = basic;

    mapobject.y = 130;
    mapobject.height = 40;
    return mapobject;
}
/** drawimage3
* A function that will draw an arbitrary sized image
* onto the screen (with DMA).
* @param x row to draw the image
* @param y column to draw the image
* @param width width of the image
* @param height height of the image
* @param image Pointer to the first element of the image.
*/
void drawImage3(int x, int y, int width, int height, const unsigned short *image) {
    //do for each row in height
    for (int row = 0; row < height; row ++) {
        DMA[3].src = &image[width*row];
        // DMA[3].dst = videoBuffer + ((y + row) * 240+ x);
        DMA[3].dst = &videoBuffer[(y+row)*240 + x];
        DMA[3].cnt = width | DMA_ON;
    }
}

/**
 * fill screen with color using DMA
 * @param color either custom color or
 *              one defined in myLib.h
 */
void fillScreen3(int color) {
    unsigned volatile colorVar = color;
    DMA[3].src = &colorVar;
    DMA[3].dst = videoBuffer;
    DMA[3].cnt = (240*160) | DMA_SOURCE_FIXED | DMA_ON;
}