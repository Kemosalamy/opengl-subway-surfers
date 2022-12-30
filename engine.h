#ifndef ENGINE_H
#define ENGINE_H

//=======================================================================
// `INCLUDES`
//=======================================================================

#include "includes.h"
#include "classes.h"

//=======================================================================
// `CONSTANTS`
//=======================================================================

constexpr int WINDOW_WIDTH = 544;
constexpr int WINDOW_HEIGHT = 960;
constexpr int WINDOW_INITIAL_POSITION_X = 600;
constexpr int WINDOW_INITIAL_POSITION_Y = 30;
constexpr char* WINDOW_NAME = "Subway Surfers";
constexpr char* AUDIO_BASE_DIRECTORY = "G:\\sounds";
constexpr double CAMERA_FIELD_OF_VIEW = 25.0;
constexpr double CAMERA_ASPECT_RATIO = (double)WINDOW_WIDTH / (double)WINDOW_HEIGHT;
constexpr double CAMERA_Z_NEAR = 0.001;
constexpr double CAMERA_Z_FAR = 100000000;
constexpr double OBSTACLE_RESPAWN_CAP = 10;
constexpr double INITIAL_OBSTACLE_DISTANCE = 30;
constexpr double LANE_WIDTH = 3;

const Vector3d INITIAL_CAMERA_POSITION(0.0, 4.1, 15.3);
const Vector3d INITIAL_CAMERA_TARGET(0.0, 1.9, 1.3);
const Vector3d FPS_CAMERA_POSITION(0.0, 2, 3.4);
const Vector3d FPS_CAMERA_TARGET(0.0, 2, -8);
const Vector3d INITIAL_PLAYER_POSITION(0.0, 0.0, 4.0);

//=======================================================================
// `FUNCTION DECLARATIONS`
//=======================================================================

void init();
void timer(int);
void display();
void subwayTimer();
void subwayDisplay();
void beachTimer();
void beachDisplay();
void setupSubwayCamera();
void setupSubwayLights();
void setupBeachCamera();
void setupBeachLights();
void input(int key, int x, int y);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void loadAssets();
void drawRunningPlayer();
void drawSurfingPlayer();
void playerJump();
void playerMove(bool left);
void switchCamera();
void switchStage();
void gameOver();
int random();
void printText();
void printText(int x, int y, char* string);

#endif