#include "game.h"

int main()
{
    //enable mode 3 display
	REG_DISPCTL = MODE3 | BG2_ENABLE;
    //start the game
    titleScreen();
}
