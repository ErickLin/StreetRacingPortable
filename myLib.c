#include "myLib.h"

unsigned short *videoBuffer = (unsigned short *)0x6000000;

void waitForVBlank()
{
	while(SCANLINECOUNTER > SCREEN_HEIGHT);
	while(SCANLINECOUNTER < SCREEN_HEIGHT);
}

void setPixel(int row, int col, u16 color)
{
	videoBuffer[OFFSET(row, col, SCREEN_WIDTH)] = color;
}

void drawRect(int row, int col, int height, int width, u16 color)
{
	DMA[3].src = &color;
    for (int i = MAX(-row, 0); i < height && row + i <= SCREEN_HEIGHT; i++) {
	    DMA[3].dst = videoBuffer + OFFSET(row + i, col, SCREEN_WIDTH);
	    DMA[3].cnt = width | DMA_ON | DMA_SOURCE_FIXED;
    }
}

/* drawimage3
* A function that will draw an arbitrary sized image
* onto the screen (with DMA).
* @param r row to draw the image
* @param c column to draw the image
* @param height height of the image
* @param width width of the image
* @param image Pointer to the first element of the image.
*/
void drawImage3(int r, int c, int height, int width, const u16* image)
{
    for (int i = MAX(-r, 0); i < height && r + i <= SCREEN_HEIGHT; i++) {
	    DMA[3].src = image + i * width;
	    DMA[3].dst = videoBuffer + OFFSET(r + i, c, SCREEN_WIDTH);
	    DMA[3].cnt = width | DMA_ON;
    }
    /*
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if (image[OFFSET(i, j, width)] != MAGENTA)
            {
                setPixel(r + i, c + j, image[OFFSET(i, j, width)]);
            }
        }
    }
    */
}

//draw image with center instead of upper-left corner at (r, c)
void drawImageCentered(int r, int c, int width, int height, const u16* image)
{
    drawImage3(r - width / 2, c - height / 2, width, height, image);
}
void drawChar(int row, int col, char ch, u16 color)
{
	int r,c;
	for(r=0; r<8; r++)
	{
		for(c=0; c<6; c++)
		{
			if(fontdata_6x8[OFFSET(r, c, 6) + ch*48])
			{
				setPixel(r+row, c+col, color);
			}
		}
	}
}

void drawString(int row, int col, char *s, u16 color)
{
	while (*s)
	{
		drawChar(row, col, *s++, color);
		col += 6;
	}
}

//draw string with center instead of upper-left corner at (r, c)
void drawStringCentered(int row, int col, char *s, u16 color)
{
    char *sCopy = s;
    int len = 0;
    while (*sCopy++)
	{
		len++;
	}
    drawString(row - 4, col - len * 3, s, color);
}

//ensures that *var is at least lo, and *var + size is at most hi
void boundsCheck(int *var, int lo, int hi, int size)
{
	if(*var < lo)
	{
		*var = lo;
	}
	if(*var > hi - size)
	{
		*var = hi - size;
	}
}

//generates random number between 0 (inclusive) and n (exclusive)
int genRandInt(int n)
{
    return (rand() >> 16) * n >> 15;
}

//returns true with probability 1/outcomes
int testChance(int outcomes)
{
    return !(genRandInt(outcomes));
}
