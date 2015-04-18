#include <stdlib.h>

typedef unsigned short u16;

#define FALSE 0
#define TRUE 1
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define OFFSET(r, c, numcols) ((r)*(numcols)+(c))

#define REG_DISPCTL *(unsigned short *)0x4000000
#define MODE3 3
#define BG2_ENABLE (1<<10)
#define COLOR(r, g, b) ((r) | (g)<<5 | (b)<<10)
#define RED COLOR(31,0,0)
#define GREEN COLOR(0,31,0)
#define BLUE COLOR(0,0,31)
#define CYAN COLOR(0,31,31)
#define MAGENTA COLOR(31, 0,31)
#define YELLOW COLOR(31,31,0)
#define WHITE COLOR(31,31,31)
#define BLACK 0
#define LTGRAY COLOR(25, 25, 25)
#define SCANLINECOUNTER  (*(volatile unsigned short *)0x4000006)

#define BUTTON_A		(1<<0)
#define BUTTON_B		(1<<1)
#define BUTTON_SELECT	(1<<2)
#define BUTTON_START	(1<<3)
#define BUTTON_RIGHT	(1<<4)
#define BUTTON_LEFT		(1<<5)
#define BUTTON_UP		(1<<6)
#define BUTTON_DOWN		(1<<7)
#define BUTTON_R		(1<<8)
#define BUTTON_L		(1<<9)

#define KEY_DOWN_NOW(key)  (~(BUTTONS) & key)

#define BUTTONS *(volatile unsigned int *)0x4000130

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160

// DMA
typedef struct
{
	const volatile void *src;
	const volatile void *dst;
	unsigned int cnt;
} DMA_CONTROLLER;

#define DMA ((volatile DMA_CONTROLLER *) 0x040000B0)

#define DMA_CHANNEL_0 0
#define DMA_CHANNEL_1 1
#define DMA_CHANNEL_2 2
#define DMA_CHANNEL_3 3

#define DMA_DESTINATION_INCREMENT (0 << 21)
#define DMA_DESTINATION_DECREMENT (1 << 21)
#define DMA_DESTINATION_FIXED (2 << 21)
#define DMA_DESTINATION_RESET (3 << 21)

#define DMA_SOURCE_INCREMENT (0 << 23)
#define DMA_SOURCE_DECREMENT (1 << 23)
#define DMA_SOURCE_FIXED (2 << 23)

#define DMA_REPEAT (1 << 25)

#define DMA_16 (0 << 26)
#define DMA_32 (1 << 26)

#define DMA_NOW (0 << 28)
#define DMA_AT_VBLANK (1 << 28)
#define DMA_AT_HBLANK (2 << 28)
#define DMA_AT_REFRESH (3 << 28)

#define DMA_IRQ (1 << 30)
#define DMA_ON (1 << 31)

extern unsigned short *videoBuffer;

extern const unsigned char fontdata_6x8[12288];

void waitForVBlank();
void setPixel(int row, int col, u16 color);
void drawRect(int row, int col, int height, int width, u16 color);
void drawImage3(int r, int c, int width, int height, const u16* image);
void drawImageCentered(int r, int c, int width, int height, const u16* image);
void drawChar(int row, int col, char ch, u16 color);
void drawString(int row, int col, char *s, u16 color);
void drawStringCentered(int row, int col, char *s, u16 color);
void boundsCheck(int *var, int lo, int hi, int size);
int genRandInt(int n);
int testChance(int outcomes);
