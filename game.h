#include <stdio.h>
#include "myLib.h"

#define MAX_FUEL 1000
#define MAX_LIVES 3

//used to convert other types into string/char array
extern char strBuffer[16];
extern const int roadColor;
extern const int groundColor;
extern const unsigned char img_title[38400];
extern const unsigned char img_game_over[38400];
extern const unsigned char img_road[23360];
extern const unsigned char img_road_line[2];
extern const unsigned char img_road_line_long[320];
extern const unsigned char img_road_line_short[32];
extern const unsigned char img_player[512];
extern const unsigned char img_player_dead[512];
extern const unsigned char img_gas[400];
extern const unsigned char img_car_down_black[512];
extern const unsigned char img_car_down_black_dead[512];
extern const unsigned char img_car_down_red[512];
extern const unsigned char img_car_down_red_dead[512];
extern const unsigned char img_car_down_green[512];
extern const unsigned char img_car_down_green_dead[512];
extern const unsigned char img_car_down_yellow[512];
extern const unsigned char img_car_down_yellow_dead[512];
extern const unsigned char img_car_up_black[512];
extern const unsigned char img_car_up_black_dead[512];
extern const unsigned char img_car_up_red[512];
extern const unsigned char img_car_up_red_dead[512];
extern const unsigned char img_car_up_green[512];
extern const unsigned char img_car_up_green_dead[512];
extern const unsigned char img_car_up_yellow[512];
extern const unsigned char img_car_up_yellow_dead[512];
extern const unsigned char img_police_0[512];
extern const unsigned char img_police_1[512];
extern const unsigned char img_police_dead[512];
//represents a cycle that is incremented every frame
extern int t;
//number of player cars remaining
extern int lives;
extern int score;

typedef struct {
    int row;
    int col;
    int height;
    int width;
    int lineWidth;
    int shortLineHeight;
    int speed;
    const u16* img;
    const u16* imgLine;
} ROAD;
extern ROAD road;

#define GAS_HEIGHT 16
#define GAS_WIDTH 25

typedef struct {
	int row;
	int col;
    int height;
    int width;
    const u16* img;
} GAS;
extern GAS gas[100];
extern int numGas;

#define CAR_HEIGHT 32
#define CAR_WIDTH 16

typedef struct {
	int row;
	int col;
    int height;
    int width;
    int dead;
    int fuel;
    int fuelDecInterval;
    const u16* img;
    const u16* imgDead;
} PLAYER;
extern PLAYER player;

typedef struct {
	int row;
	int col;
    int height;
    int width;
    //speed is stored as fractional representation
    int speedNum;
    int speedDenom;
    int dead;
    const u16* img;
    const u16* imgDead;
} CAR_UP;
extern CAR_UP carsUp[100];
extern int numCarsUp;

typedef struct {
	int row;
	int col;
    int height;
    int width;
    //speed is stored as fractional representation
    int speedNum;
    int speedDenom;
    int dead;
    const u16* img;
    const u16* imgDead;
} CAR_DOWN;
extern CAR_DOWN carsDown[100];
extern int numCarsDown;

typedef struct {
    int exists;
	int row;
	int col;
    int height;
    int width;
    //speed is stored as fractional representation
    int vSpeedNum;
    int vSpeedDenom;
    int hSpeed;
    int frame;
    int updateInterval;
    int dead;
    const u16* img0;
    const u16* img1;
    const u16* imgDead;
} CAR_POLICE;
extern CAR_POLICE police;

void titleScreen();
void playGame();
void gameOverScreen();
void startLevel();
void resetLevel();
//functions in game loop
void erase();
void updateVars();
void checkOutsideScreen();
void generateObjs();
void draw();
int checkCollision(int row, int col, int height, int width
        , int rowOther, int colOther, int heightOther, int widthOther);
void drawRoad(ROAD road);
void drawRoadLines(ROAD road);
void eraseRoadLines(ROAD road);
