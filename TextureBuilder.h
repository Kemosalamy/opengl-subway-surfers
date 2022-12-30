#include <stdio.h>
#include "glew.h"
#include "glaux.h"

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "glaux.lib")

void loadPPM(GLuint* textureID, char* strFileName, int width, int height, int wrap);
void loadBMP(GLuint* textureID, char* strFileName, int wrap);