//=======================================================================
// `INCLUDES`
//=======================================================================

#include "includes.h"
#include "engine.h"


//=======================================================================
// `MAIN`
//=======================================================================

int main(int argc, char** argv){

    printf("%s", argv[0]);

    // Initialises graphics library utility toolkit
    glutInit(&argc, argv);

    // Initialise Double-Buffering
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

    // Setting up window size and initial position
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(WINDOW_INITIAL_POSITION_X, WINDOW_INITIAL_POSITION_Y);

    // Creating the viewable window with its name
    glutCreateWindow(WINDOW_NAME);

    // Enabling additional flags
    // glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH);

    // Initialise graphics library extension wrangler
    glewInit();

    // Initialise state of engine
    init();

    // Initialising link with core engine functions
    glutTimerFunc(0,timer,0);
    glutDisplayFunc(display);
    glutSpecialFunc(input);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    
    // Starting the initial glut loop
    glutMainLoop();

    // Exit code of 0 to indicate success
    return 0;
}
