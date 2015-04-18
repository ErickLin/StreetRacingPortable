#include "game.h"

char strBuffer[16];
const int groundColor = COLOR(14, 11, 8);
const int roadColor = BLACK;
const int roadLineColor = LTGRAY;
int t;
int lives;
int score;

ROAD road = {0, 12, 160, 146, 2, 16, 2, (const u16*) img_road, (const u16*) img_road_line};
GAS gas[100];
int numGas;
PLAYER player = {0, 0, CAR_HEIGHT, CAR_WIDTH, FALSE, MAX_FUEL, 1, (const u16*) img_player, (const u16*) img_player_dead};
CAR_UP carsUp[100];
int numCarsUp;
CAR_DOWN carsDown[100];
int numCarsDown;
CAR_POLICE police = {TRUE, SCREEN_HEIGHT, 0, CAR_HEIGHT, CAR_WIDTH, 1, 2, 1, 0, 6, FALSE, (const u16*) img_police_0, (const u16*) img_police_1, (const u16*) img_police_dead};

void titleScreen()
{
    drawImage3(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, (const u16*) img_title);
    int keysPressed = 0;
    //title screen loop
	while (1)
    {
        //test button release for all buttons
        for (int i = 0; i < 10; i++)
        {
            if (KEY_DOWN_NOW(1 << i))
            {
                keysPressed |= (1 << i);
            }
            if ((keysPressed & (1 << i)) && !KEY_DOWN_NOW(1 << i))
            {
                playGame();
            }
        }
    }
}

void playGame()
{
    lives = MAX_LIVES;
    score = 0;
    startLevel();
    resetLevel();
    int resetPressed = FALSE;
    // game loop
	while (lives > 0) 
	{
        //test select button release
        if (KEY_DOWN_NOW(BUTTON_SELECT))
        {
            resetPressed = TRUE;
        }
        if (resetPressed && !KEY_DOWN_NOW(BUTTON_SELECT))
        {
            titleScreen();
        }
		waitForVBlank();
        erase();
        updateVars();
        checkOutsideScreen();
        generateObjs();
        draw();
	}
    gameOverScreen();
}

void gameOverScreen()
{
    drawImage3(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, (const u16*) img_game_over);
    sprintf(strBuffer, "Score: %d", score);
    drawStringCentered(SCREEN_HEIGHT / 2 + 14, SCREEN_WIDTH / 2, strBuffer, WHITE);
    int keysPressed = 0;
    //game over screen loop
	while (1)
    {
        //test button release for all buttons
        for (int i = 0; i < 10; i++)
        {
            if (KEY_DOWN_NOW(1 << i))
            {
                keysPressed |= (1 << i);
            }
            if ((keysPressed & (1 << i)) && !KEY_DOWN_NOW(1 << i))
            {
                titleScreen();
            }
        }
    }
}

void startLevel()
{
	DMA[3].src = &groundColor;
	DMA[3].dst = videoBuffer;
	DMA[3].cnt = SCREEN_WIDTH * SCREEN_HEIGHT | DMA_ON | DMA_SOURCE_FIXED;

    sprintf(strBuffer, "Score: ");
    drawString(16, 160, strBuffer, WHITE);
    sprintf(strBuffer, "Cars left: ");
    drawString(32, 160, strBuffer, WHITE);
    sprintf(strBuffer, "Petrol ");
    drawString(48, 160, strBuffer, WHITE);
}

void resetLevel()
{
    t = 0;
    road.row = 0;
    player.row = SCREEN_HEIGHT * 3 / 4 - CAR_HEIGHT;
    player.col = road.col + road.width / 2 - CAR_WIDTH / 2;
    player.dead = FALSE;
    player.fuel = MAX_FUEL;
    numGas = 0;
    numCarsUp = 0;
    numCarsDown = 0;
    police.exists = FALSE;
	drawRoad(road);
}

//replace drawn objects with background color
void erase()
{
    eraseRoadLines(road);
    //erase each gas object
    for (int i = 0; i < numGas; i++)
    {
        drawRect(gas[i].row, gas[i].col, gas[i].height, gas[i].width, roadColor);
    }
    //erase the player object
    drawRect(player.row, player.col, player.height, player.width, roadColor);
    //erase each car
    for (int i = 0; i < numCarsUp; i++)
    {
        drawRect(carsUp[i].row, carsUp[i].col, carsUp[i].height, carsUp[i].width, roadColor);
    }
    for (int i = 0; i < numCarsDown; i++)
    {
        drawRect(carsDown[i].row, carsDown[i].col, carsDown[i].height, carsDown[i].width, roadColor);
    }
    //erase the police car
    if (police.exists)
    {
        drawRect(police.row, police.col, police.height, police.width, roadColor);
    }
    //erase the previous score
    drawRect(16, 160 + 6 * 7, 8, 6 * 5, groundColor);
    //erase the previous number of lives
    drawRect(32, 160 + 6 * 11, 8, 6, groundColor);
    //drawRect(48, 160 + 6 * 7, 8, 6 * 3, groundColor);
}

//update all game-related variables
void updateVars()
{
    t = (t + 1) % 3600;
    road.row = (road.row + road.speed) % road.height;
    for (int i = 0; i < numGas; i++)
    {
        gas[i].row += road.speed;
    }
    if (player.row == SCREEN_HEIGHT)
    {
        player.dead = TRUE;
    }
    if (player.dead || player.fuel <= 0)
    {
        player.row += road.speed;
        if (player.row > SCREEN_HEIGHT + player.height) {
            lives--;
            resetLevel();
        }
    }
    else
    {
	    if (KEY_DOWN_NOW(BUTTON_RIGHT))
	    {
		    player.col++;
	    }
	    if (KEY_DOWN_NOW(BUTTON_LEFT))
	    {
		    player.col--;
	    }
	    if (KEY_DOWN_NOW(BUTTON_UP))
	    {
            player.row--;
	    }
	    if (KEY_DOWN_NOW(BUTTON_DOWN))
	    {
            player.row++;
	    }
        boundsCheck(&(player.row), 0, SCREEN_HEIGHT + CAR_HEIGHT, player.height);
        boundsCheck(&(player.col), road.col + 2, road.col + road.width - 2, player.width);
        if (t % player.fuelDecInterval == 0 && player.fuel > 0)
        {
            player.fuel--;
        }
    }
    for (int i = 0; i < numCarsUp; i++)
    {
        if (carsUp[i].dead)
        {
            carsUp[i].row += road.speed;
        }
        else
        {
            //move at the fractional rate vSpeedNum / vSpeedDenom
            if (t % carsUp[i].speedDenom == 0)
            {
                carsUp[i].row += carsUp[i].speedNum;
            }
        }
    }
    for (int i = 0; i < numCarsDown; i++)
    {
        if (carsDown[i].dead)
        {
            carsDown[i].row += road.speed;
        }
        else
        {
            //move at the fractional rate vSpeedNum / vSpeedDenom
            if (t % carsDown[i].speedDenom == 0)
            {
                carsDown[i].row += carsDown[i].speedNum;
            }
        }
    }
    if (police.exists)
    {
        if (police.dead)
        {
            police.row += road.speed;
        }
        else
        {
            //move at the fractional rate vSpeedNum / vSpeedDenom
            if (t % police.vSpeedDenom == 0)
            {
                police.row -= police.vSpeedNum;
            }
            //police follows player in terms of horizontal movement if no other cars in the way
            if (player.col <= police.col - police.hSpeed)
            {
                int canMove = TRUE;
                for (int i = 0; i < numCarsUp && canMove; i++) {
                    if (checkCollision(police.row, police.col - police.hSpeed, police.height, police.width
                            , carsUp[i].row, carsUp[i].col, carsUp[i].height, carsUp[i].width))
                    {
                        canMove = FALSE;
                    }
                }
                for (int i = 0; i < numCarsDown && canMove; i++) {
                    if (checkCollision(police.row, police.col - police.hSpeed, police.height, police.width
                            , carsDown[i].row, carsDown[i].col, carsDown[i].height, carsDown[i].width))
                    {
                        canMove = FALSE;
                    }
                }
                if (canMove)
                {
                    police.col -= police.hSpeed;
                }
            }
            if (player.col >= police.col + police.hSpeed)
            {
                int canMove = TRUE;
                for (int i = 0; i < numCarsUp && canMove; i++) {
                    if (checkCollision(police.row, police.col + police.hSpeed, police.height, police.width
                            , carsUp[i].row, carsUp[i].col, carsUp[i].height, carsUp[i].width))
                    {
                        canMove = FALSE;
                    }
                }
                for (int i = 0; i < numCarsDown && canMove; i++) {
                    if (checkCollision(police.row, police.col + police.hSpeed, police.height, police.width
                            , carsDown[i].row, carsDown[i].col, carsDown[i].height, carsDown[i].width))
                    {
                        canMove = FALSE;
                    }
                }
                if (canMove)
                {
                    police.col += police.hSpeed;
                }
            }
            //update animation frame
            if (t % police.updateInterval == 0)
            {
                police.frame = (police.frame + 1) % 2;
            }
        }
    }
    //collision checking
    for (int i = 0; i < numGas; i++)
    {
        //if the player is in contact with a gas container
        if (checkCollision(player.row, player.col, player.height, player.width,
            gas[i].row, gas[i].col, gas[i].height, gas[i].width))
        {
            player.fuel = MAX_FUEL;
            //remove the gas object
            //replace what was previously in index i with the last element and decrement size of array
            gas[i] = gas[(numGas--) - 1];
            i--;
        }
    }
    for (int i = 0; i < numCarsUp; i++)
    {
        if (checkCollision(player.row, player.col, player.height, player.width,
            carsUp[i].row, carsUp[i].col, carsUp[i].height, carsUp[i].width))
        {
            player.dead = TRUE;
            carsUp[i].dead = TRUE;
        }
    }
    for (int i = 0; i < numCarsDown; i++)
    {
        if (checkCollision(player.row, player.col, player.height, player.width,
            carsDown[i].row, carsDown[i].col, carsDown[i].height, carsDown[i].width))
        {
            player.dead = TRUE;
            carsDown[i].dead = TRUE;
        }
    }
    if (police.exists)
    {
        if (checkCollision(player.row, player.col, player.height, player.width,
                police.row, police.col, police.height, police.width))
        {
            player.dead = TRUE;
            police.dead = TRUE;
        }
        for (int i = 0; i < numCarsUp; i++)
        {
            if (checkCollision(police.row, police.col, police.height, police.width,
                carsUp[i].row, carsUp[i].col, carsUp[i].height, carsUp[i].width))
            {
                police.dead = TRUE;
                carsUp[i].dead = TRUE;
            }
        }
        for (int i = 0; i < numCarsDown; i++)
        {
            if (checkCollision(police.row, police.col, police.height, police.width,
                carsDown[i].row, carsDown[i].col, carsDown[i].height, carsDown[i].width))
            {
                police.dead = TRUE;
                carsDown[i].dead = TRUE;
            }
        }
    }
    score++;
}

//generate cars and gas containers based on RNG
void generateObjs()
{
    //generate gas container
    if (testChance(600 + score / 480 < 2000 ? 600 + score / 480 : 2000))
    {
        gas[numGas].row = -GAS_HEIGHT;
        gas[numGas].col = road.col + 2 + genRandInt(road.width - 4 - GAS_WIDTH);
        gas[numGas].height = GAS_HEIGHT;
        gas[numGas].width = GAS_WIDTH;
        gas[numGas].img = (const u16*) img_gas;
        numGas++;
    }
    //generate cars
    if (testChance((400 - score/800 > 1) ? 400 - score/800 : 1))
    {
        carsUp[numCarsUp].row = -CAR_HEIGHT;
        carsUp[numCarsUp].height = CAR_HEIGHT;
        carsUp[numCarsUp].width = CAR_WIDTH;
        carsUp[numCarsUp].dead = FALSE;
        int laneNum = genRandInt(6);
        if (laneNum == 0)
        {
            carsUp[numCarsUp].col = (road.col + 2) + (road.width - 4) / 2 + 4;
            carsUp[numCarsUp].speedNum = 1;
            carsUp[numCarsUp].speedDenom = 2;
        }
        if (laneNum >= 1 && laneNum <= 2)
        {
            carsUp[numCarsUp].col = (road.col + 2) + 2 * (road.width - 4) / 3 + 4;
            carsUp[numCarsUp].speedNum = 1;
            carsUp[numCarsUp].speedDenom = 1;
        }
        if (laneNum >= 3)
        {
            carsUp[numCarsUp].col = (road.col + 2) + 5 * (road.width - 4) / 6 + 4;
            carsUp[numCarsUp].speedNum = 3;
            carsUp[numCarsUp].speedDenom = 2;
        }
        int color = genRandInt(4);
        if (color == 0)
        {
            carsUp[numCarsUp].img = (const u16*) img_car_up_black;
            carsUp[numCarsUp].imgDead = (const u16*) img_car_up_black_dead;
        }
        if (color == 1)
        {
            carsUp[numCarsUp].img = (const u16*) img_car_up_green;
            carsUp[numCarsUp].imgDead = (const u16*) img_car_up_green_dead;
        }
        if (color == 2)
        {
            carsUp[numCarsUp].img = (const u16*) img_car_up_red;
            carsUp[numCarsUp].imgDead = (const u16*) img_car_up_red_dead;
        }
        if (color == 3)
        {
            carsUp[numCarsUp].img = (const u16*) img_car_up_yellow;
            carsUp[numCarsUp].imgDead = (const u16*) img_car_up_yellow_dead;
        }
        //only create the object if there is not already another car in its potential starting position
        int create = TRUE;
        if (police.exists)
        {
            if (checkCollision(carsUp[numCarsUp].row, carsUp[numCarsUp].col, carsUp[numCarsUp].height
                    , carsUp[numCarsUp].width, police.row, police.col, police.height, police.width))
            {
                create = FALSE;
            }
        }   
        for (int i = 0; i < numCarsUp && create; i++)
        {
            if (checkCollision(carsUp[numCarsUp].row, carsUp[numCarsUp].col, carsUp[numCarsUp].height
                    , carsUp[numCarsUp].width, carsUp[i].row, carsUp[i].col
                    , carsUp[i].height, carsUp[i].width))
            {
                create = FALSE;
            }
        }
        if (create)
        {
            numCarsUp++;
        }
    }
    if (testChance((140 - score/800 > 1) ? 140 - score/800 : 1))
    {
        carsDown[numCarsDown].row = -CAR_HEIGHT;
        carsDown[numCarsDown].height = CAR_HEIGHT;
        carsDown[numCarsDown].width = CAR_WIDTH;
        carsDown[numCarsDown].dead = FALSE;
        int laneNum = genRandInt(6);
        if (laneNum == 0)
        {
            carsDown[numCarsDown].col = (road.col + 2) + 4;
            carsDown[numCarsDown].speedNum = 5;
            carsDown[numCarsDown].speedDenom = 2;
        }
        if (laneNum >= 1 && laneNum <= 2)
        {
            carsDown[numCarsDown].col = (road.col + 2) + (road.width - 4) / 6 + 4;
            carsDown[numCarsDown].speedNum = 3;
            carsDown[numCarsDown].speedDenom = 1;
        }
        if (laneNum >= 3)
        {
            carsDown[numCarsDown].col = (road.col + 2) + (road.width - 4) / 3 + 4;
            carsDown[numCarsDown].speedNum = 7;
            carsDown[numCarsDown].speedDenom = 2;
        }
        int color = genRandInt(4);
        if (color == 0)
        {
            carsDown[numCarsDown].img = (const u16*) img_car_down_black;
            carsDown[numCarsDown].imgDead = (const u16*) img_car_down_black_dead;
        }
        if (color == 1)
        {
            carsDown[numCarsDown].img = (const u16*) img_car_down_green;
            carsDown[numCarsDown].imgDead = (const u16*) img_car_down_green_dead;
        }
        if (color == 2)
        {
            carsDown[numCarsDown].img = (const u16*) img_car_down_red;
            carsDown[numCarsDown].imgDead = (const u16*) img_car_down_red_dead;
        }
        if (color == 3)
        {
            carsDown[numCarsDown].img = (const u16*) img_car_down_yellow;
            carsDown[numCarsDown].imgDead = (const u16*) img_car_down_yellow_dead;
        }
        //only create the object if there is not already another car in its potential starting position
        int create = TRUE;
        if (police.exists)
        {
            if (checkCollision(carsDown[numCarsDown].row, carsDown[numCarsDown].col
                    , carsDown[numCarsDown].height, carsDown[numCarsDown].width
                    , police.row, police.col, police.height, police.width))
            {
                create = FALSE;
            }
        }   
        for (int i = 0; i < numCarsDown && create; i++)
        {
            if (checkCollision(carsDown[numCarsDown].row, carsDown[numCarsDown].col
                    , carsDown[numCarsDown].height, carsDown[numCarsDown].width, carsDown[i].row
                    , carsDown[i].col, carsDown[i].height, carsDown[i].width))
            {
                create = FALSE;
            }
        }
        if (create)
        {
            numCarsDown++;
        }
    }
    //generate police car
    if (!police.exists && testChance((1600 - score/1200 > 1) ? 1600 - score/1200 : 1))
    {
        police.exists = TRUE;
        police.row = SCREEN_HEIGHT;
        police.col = player.col;
        police.frame = 0;
        police.dead = FALSE;
    }
}

//remove objects outside screen
void checkOutsideScreen()
{
    for (int i = 0; i < numGas; i++)
    {
        if (gas[i].row > SCREEN_HEIGHT)
        {
            //remove the object
            //replace what was previously in index i with the last element and decrement size of array
            gas[i] = gas[(numGas--) - 1];
            i--;
        }
    }
    for (int i = 0; i < numCarsUp; i++)
    {
        if (carsUp[i].row > SCREEN_HEIGHT)
        {
            //remove the object
            //replace what was previously in index i with the last element and decrement size of array
            carsUp[i] = carsUp[(numCarsUp--) - 1];
            i--;
        }
    }
    for (int i = 0; i < numCarsDown; i++)
    {
        if (carsDown[i].row > SCREEN_HEIGHT)
        {
            //remove the object
            //replace what was previously in index i with the last element and decrement size of array
            carsDown[i] = carsDown[(numCarsDown--) - 1];
            i--;
        }
    }
    if (police.exists)
    {
        if ((police.row > SCREEN_HEIGHT && police.dead)
                || police.row < -police.height)
        {
            police.exists = FALSE;
        }
    }
}

//draw all objects that update position every frame
void draw()
{
    drawRoadLines(road);
    for (int i = 0; i < numGas; i++)
    {
        drawImage3(gas[i].row, gas[i].col, gas[i].height, gas[i].width, gas[i].img);
    }
    for (int i = 0; i < numCarsUp; i++)
    {
        if (carsUp[i].dead)
        {
            drawImage3(carsUp[i].row, carsUp[i].col, carsUp[i].height, carsUp[i].width, carsUp[i].imgDead);
        }
        else
        {
            drawImage3(carsUp[i].row, carsUp[i].col, carsUp[i].height, carsUp[i].width, carsUp[i].img);
        }
    }
    for (int i = 0; i < numCarsDown; i++)
    {
        if (carsDown[i].dead)
        {
            drawImage3(carsDown[i].row, carsDown[i].col, carsDown[i].height, carsDown[i].width
                    , carsDown[i].imgDead);
        }
        else
        {
            drawImage3(carsDown[i].row, carsDown[i].col, carsDown[i].height, carsDown[i].width
                    , carsDown[i].img);
        }
    }
    if (player.dead)
    {
        drawImage3(player.row, player.col, player.height, player.width, player.imgDead);
    }
    else 
    {
        drawImage3(player.row, player.col, player.height, player.width, player.img);
    }
    if (police.exists)
    {
        if (police.dead)
        {
            drawImage3(police.row, police.col, police.height, police.width, police.imgDead);
        }
        else 
        {
            if (police.frame == 0)
            {
                drawImage3(police.row, police.col, police.height, police.width, police.img0);
            }
            else
            {
                drawImage3(police.row, police.col, police.height, police.width, police.img1);
            }
        }
    }
	sprintf(strBuffer, "%d", score);
    drawString(16, 160 + 6 * 7, strBuffer, WHITE);
	sprintf(strBuffer, "%d", lives);
    drawString(32, 160 + 6 * 11, strBuffer, WHITE);
	//sprintf(strBuffer, "%d", player.fuel);
    //drawString(48, 160 + 6 * 7, strBuffer, WHITE);
    drawRect(48, 160 + 6 * 7, 8, 6 * 5, BLACK);
    if (6 * 5 * player.fuel / MAX_FUEL > 0)
    {
        int color;
        if (player.fuel >= 0.7 * MAX_FUEL)
        {
            color = GREEN;
        }
        else if (player.fuel >= 0.3 * MAX_FUEL)
        {
            color = YELLOW;
        }
        else
        {
            color = RED;
        }
        drawRect(48, 160 + 6 * 7, 8, (6 * 5 * player.fuel + MAX_FUEL / 2) / MAX_FUEL, color);
    }
}

//first 4 variables describe object 1, and last 4 variables describe object 2
int checkCollision(int row, int col, int height, int width
        , int rowOther, int colOther, int heightOther, int widthOther)
{
    if (row + height <= rowOther || row >= rowOther + heightOther
        || col + width <= colOther || col >= colOther + widthOther)
    {
        return 0;
    }
    return 1;
}

void drawRoad(ROAD road) {
    for (int row = 0; row < SCREEN_HEIGHT; row++)
    {
	    DMA[3].src = road.img + row * road.width;
	    DMA[3].dst = videoBuffer + OFFSET(row, road.col, SCREEN_WIDTH);
	    DMA[3].cnt = road.width | DMA_ON;
    }
}

void drawRoadLines(ROAD road) {
    DMA[3].src = road.imgLine;
    //draw long line
    for (int row = 0; row < SCREEN_HEIGHT; row++)
    {
	    //DMA[3].src = road.imgLineLong + row * road.lineWidth;
	    DMA[3].dst = videoBuffer + OFFSET(row, road.col + road.width / 2 - 1, SCREEN_WIDTH);
	    DMA[3].cnt = road.lineWidth | DMA_ON;
        /*
        setPixel(row, road.col + road.width / 2, roadLineColor);
        setPixel(row, road.col + road.width / 2 - 1, roadLineColor);
        */
    }
    //draw short lines
    for (int part = 0; part < 1; part++)
    {
        for (int i = 0; i < road.shortLineHeight; i++)
        {
	        //DMA[3].src = road.imgLineShort + i * road.lineWidth;
	        DMA[3].dst = videoBuffer + OFFSET(part * road.height / 1 + ((road.height / 1 - i) + road.row + road.height / 1) % (road.height / 1), road.col + (road.width - 2) / 6, SCREEN_WIDTH);
	        DMA[3].cnt = road.lineWidth | DMA_ON;
	        DMA[3].dst = videoBuffer + OFFSET(part * road.height / 1 + ((road.height / 1 - i) + road.row + road.height / 1) % (road.height / 1), road.col + (road.width - 2) / 3, SCREEN_WIDTH);
	        DMA[3].cnt = road.lineWidth | DMA_ON;
	        DMA[3].dst = videoBuffer + OFFSET(part * road.height / 1 + ((road.height / 1 - i) + road.row + road.height / 1) % (road.height / 1), road.col + (road.width - 2) * 2 / 3, SCREEN_WIDTH);
	        DMA[3].cnt = road.lineWidth | DMA_ON;
	        DMA[3].dst = videoBuffer + OFFSET(part * road.height / 1 + ((road.height / 1 - i) + road.row + road.height / 1) % (road.height / 1), road.col + (road.width - 2) * 5 / 6, SCREEN_WIDTH);
	        DMA[3].cnt = road.lineWidth | DMA_ON;
            /*
            setPixel(part * road.height / 1 + ((road.height / 1 - i) + road.row + road.height) % (road.height / 1), road.col + road.width / 6, roadLineColor);
            setPixel(part * road.height / 1 + ((road.height / 1 - i) + road.row + road.height) % (road.height / 1), road.col + road.width / 6 + 1, roadLineColor);
            setPixel(part * road.height / 1 + ((road.height / 1 - i) + road.row + road.height) % (road.height / 1), road.col + road.width / 3, roadLineColor);
            setPixel(part * road.height / 1 + ((road.height / 1 - i) + road.row + road.height) % (road.height / 1), road.col + road.width / 3 + 1, roadLineColor);
            setPixel(part * road.height / 1 + ((road.height / 1 - i) + road.row + road.height) % (road.height / 1), road.col + road.width * 2 / 3, roadLineColor);
            setPixel(part * road.height / 1 + ((road.height / 1 - i) + road.row + road.height) % (road.height / 1), road.col + road.width * 2 / 3 + 1, roadLineColor);
            setPixel(part * road.height / 1 + ((road.height / 1 - i) + road.row + road.height) % (road.height / 1), road.col + road.width * 5 / 6, roadLineColor);
            setPixel(part * road.height / 1 + ((road.height / 1 - i) + road.row + road.height) % (road.height / 1), road.col + road.width * 5 / 6 + 1, roadLineColor);
            */
        }
    }
}

void eraseRoadLines(ROAD road) {
	DMA[3].src = &roadColor;
    //erase short lines
    for (int part = 0; part < 1; part++)
    {
        for (int i = 0; i < road.shortLineHeight; i++)
        {
	        DMA[3].dst = videoBuffer + OFFSET(part * road.height / 1 + ((road.height / 1 - i) + road.row + road.height / 1) % (road.height / 1), road.col + (road.width - 2) / 6, SCREEN_WIDTH);
	        DMA[3].cnt = road.lineWidth | DMA_ON | DMA_SOURCE_FIXED;
	        DMA[3].dst = videoBuffer + OFFSET(part * road.height / 1 + ((road.height / 1 - i) + road.row + road.height / 1) % (road.height / 1), road.col + (road.width - 2) / 3, SCREEN_WIDTH);
	        DMA[3].cnt = road.lineWidth | DMA_ON | DMA_SOURCE_FIXED;
	        DMA[3].dst = videoBuffer + OFFSET(part * road.height / 1 + ((road.height / 1 - i) + road.row + road.height / 1) % (road.height / 1), road.col + (road.width - 2) * 2 / 3, SCREEN_WIDTH);
	        DMA[3].cnt = road.lineWidth | DMA_ON | DMA_SOURCE_FIXED;
	        DMA[3].dst = videoBuffer + OFFSET(part * road.height / 1 + ((road.height / 1 - i) + road.row + road.height / 1) % (road.height / 1), road.col + (road.width - 2) * 5 / 6, SCREEN_WIDTH);
	        DMA[3].cnt = road.lineWidth | DMA_ON | DMA_SOURCE_FIXED;
            /*
            setPixel(part * road.height / 1 + ((road.height / 1 - i) + (road.row - 2) + road.height) % (road.height / 1), road.col + road.width / 6, roadColor);
            setPixel(part * road.height / 1 + ((road.height / 1 - i) + (road.row - 2) + road.height) % (road.height / 1), road.col + road.width / 6 + 1, roadColor);
            setPixel(part * road.height / 1 + ((road.height / 1 - i) + (road.row - 2) + road.height) % (road.height / 1), road.col + road.width / 3, roadColor);
            setPixel(part * road.height / 1 + ((road.height / 1 - i) + (road.row - 2) + road.height) % (road.height / 1), road.col + road.width / 3 + 1, roadColor);
            setPixel(part * road.height / 1 + ((road.height / 1 - i) + (road.row - 2) + road.height) % (road.height / 1), road.col + road.width * 2 / 3, roadColor);
            setPixel(part * road.height * 2 / 2 + ((road.height / 1 - i) + (road.row - 2) + road.height) % (road.height / 1), road.col + road.width * 2 / 3 + 1, roadColor);
            setPixel(part * road.height / 1 + ((road.height / 1 - i) + (road.row - 2) + road.height) % (road.height / 1), road.col + road.width * 5 / 6, roadColor);
            setPixel(part * road.height / 1 + ((road.height / 1 - i) + (road.row - 2) + road.height) % (road.height / 1), road.col + road.width * 5 / 6 + 1, roadColor);
            */
        }
    }
}
