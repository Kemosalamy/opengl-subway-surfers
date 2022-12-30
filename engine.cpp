//=======================================================================
// `INCLUDES`
//=======================================================================

#include "includes.h"
#include "classes.h"
#include "engine.h"

//=======================================================================
// `GENERAL VARIABLES`
//=======================================================================

// Main variables

// Gameplay variables
double game_speed = 1.0;
int game_score = 0;
int game_stage = 1;
double rewind_amount = 0.23;
int fps_view = 0;
bool camera_debug = false;
Audio audioManager;


// Camera object
Camera camera(INITIAL_CAMERA_POSITION, INITIAL_CAMERA_TARGET, { 0,1,0 });

// Player position state
GameObject player(INITIAL_PLAYER_POSITION);
string player_moving_state = "running";
Vector3d player_horizontal_target = INITIAL_PLAYER_POSITION;
double player_horizontal_easing = 0.22;

double initial_jump_height = 0.18;
double player_falling_gravity = 0.008;
double player_falling_velocity = 0;

// Randomisation object
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

// Model variables

Model_3DS model_rail;
Model_3DS model_subway;
Model_3DS model_barricade;
Model_3DS model_coin;
Model_3DS model_ship;
Model_3DS model_swim_ring;
Model_3DS model_house;

vector<Model_3DS> model_player_running(4);
Model_3DS model_player_running_left;
Model_3DS model_player_running_right;
Model_3DS model_player_running_jump;
Model_3DS model_player_rising_jump;
Model_3DS model_player_splat;

Model_3DS model_player_surfing;
Model_3DS model_player_surfing_left;
Model_3DS model_player_surfing_right;
Model_3DS model_player_surfing_jump;
vector<Model_3DS> model_player_surfing_splash(2);

// Texture variables

GLTexture tex_ground;
GLTexture tex_ocean;
GLTexture tex_sky;

//=======================================================================
// `SUBWAY VARIABLES`
//=======================================================================

double floor_uv_counter = 0;
double floor_vertex_limit = 30;
double floor_vertex_distance = 8;
deque<pair<Vector3d, Vector2d>> floor_vertices;

double rail_limit = 80;
double rail_distance = 2.5;
deque<Vector3d> rail_positions;

double house_limit = 80;
double house_distance = 7.5;
deque<Vector3d> house_positions;

// Player animation state

vector<int> running_animation_timings = { 8,5,8,5};
int running_animation_index = 0;
int running_animation_counter = 0;

// Obstacle spawning variables

deque<GameObject> subway_obstacles;
double subway_obstacle_distance = 15;
double subway_coin_distance = 2;
int subway_obstacle_count_limit = 20;

// Lighting control variables
Vector3d subway_light_pos = { 0,0,0 };
vector<double> subway_light_color = { 1.0,1.0,1.0 };

//=======================================================================
// `BEACH VARIABLES`
//=======================================================================

// Ocean surface variables

double ocean_uv_counter = 0;
double ocean_vertex_limit = 30;
double ocean_vertex_distance = 8;
deque<pair<Vector3d, Vector2d>> ocean_vertices;

// Ocean obstacles variables

deque<GameObject> ocean_obstacles;
double ocean_obstacle_distance = 15;
double ocean_coin_distance = 2;
int ocean_obstacle_count_limit = 30;

// Player animation state

double surfing_jump_angle = 0;
double surfing_jump_rotation = -0.7;

int splash_current = 0;
int splash_counter = 0;
int splash_counter_max = 12;

// Lighting control variables
Vector3d beach_light_pos = { 0,0,0.3 };
vector<double> beach_light_color = { 1.0,1.0,1.0 };



//=======================================================================
// `FUNCTION DEFINITIONS`
//=======================================================================

void init(){
    glClearColor(0.0, 0.0, 0.0, 0.0);
    loadAssets();
    audioManager.BasePath = AUDIO_BASE_DIRECTORY;
    audioManager.Play("theme.wav",0.5f,true);
}

void timer(int) {
    // Call the display function to render our objects every frame
    glutPostRedisplay();


    if (game_stage == 0)
        ;
    else if (game_stage == 1)
        subwayTimer();
    else
        beachTimer();

    // Call the timer function to run again after 1/60th of a second
    glutTimerFunc(1000.0 / 60.0, timer, 0);
}

void display() {
    // Clear the previous frames color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    string current_score = "Score: " + to_string(game_score);
    char current_score_chars[50];
    strcpy(current_score_chars, current_score.c_str());
    printText(WINDOW_WIDTH-200,WINDOW_HEIGHT-30, current_score_chars);

    if (game_stage == 0)
        ;
    else if (game_stage == 1)
        subwayDisplay();
    else
        beachDisplay();


    // Swap Buffers
    glutSwapBuffers();
}

void subwayTimer() {
    
    
    if (player_moving_state == "lost") {
        game_speed = 0;
        return;
    }

    // Add new floor vertices
    while ((floor_vertices.size() / 2) < floor_vertex_limit) {
        double last_distance = floor_vertices.empty() ? INITIAL_CAMERA_POSITION.z : floor_vertices.back().first.z;
        Vector3d left_vertex = Vector3d(-LANE_WIDTH * 2, 0, last_distance - floor_vertex_distance);
        Vector3d right_vertex = Vector3d(LANE_WIDTH * 2, 0, last_distance - floor_vertex_distance);
        floor_vertices.push_back({ left_vertex, Vector2d(0, floor_uv_counter) });
        floor_vertices.push_back({ right_vertex, Vector2d(3, floor_uv_counter) });
        floor_uv_counter++;
    }

    // Move the floor backwards
    for (auto& vertex : floor_vertices) {
        double rewind_distance = game_speed * rewind_amount;
        vertex.first.z += rewind_distance;
    }

    // Delete floor vertices which are out of sight
    if (!floor_vertices.empty()) {
        double behind_camera = camera.position.z + 2 * floor_vertex_distance;
        while (floor_vertices.front().first.z > behind_camera)
            floor_vertices.pop_front();
    }

    // Add new rails to deque
    while (rail_positions.size() < rail_limit) {
        double last_distance = rail_positions.empty() ? INITIAL_CAMERA_POSITION.z : rail_positions.back().z;
        rail_positions.push_back(Vector3d(0,0,last_distance-rail_distance));
    }

    // Move rails backwards
    for (auto& rail : rail_positions) {
        double rewind_distance = game_speed * rewind_amount;
        rail.z += rewind_distance;
    }

    // Delete out of view rails
    if (!rail_positions.empty()) {
        double behind_camera = camera.position.z + 2 * rail_distance;
        while (rail_positions.front().z > behind_camera)
            rail_positions.pop_front();
    }

    // Add new houses to deque
    while (house_positions.size() < house_limit) {
        double last_distance = house_positions.empty() ? INITIAL_CAMERA_POSITION.z : house_positions.back().z;
        house_positions.push_back(Vector3d(0, 0, last_distance - house_distance));
    }

    // Move house backwards
    for (auto& house : house_positions) {
        double rewind_distance = game_speed * rewind_amount;
        house.z += rewind_distance;
    }

    // Delete out of view houses
    if (!house_positions.empty()) {
        double behind_camera = camera.position.z + 2 * house_distance;
        while (house_positions.front().z > behind_camera)
            house_positions.pop_front();
    }

    // Add obstacles if the deque is not full
    while (subway_obstacles.size() < subway_obstacle_count_limit) {
        double last_position = subway_obstacles.empty() ? -INITIAL_CAMERA_POSITION.z : subway_obstacles.back().position.z;
        int obstacle_type = random() % 3;
        double obstacle_lane = -1 + (random() % 3);
        if (obstacle_type == 0) {
            // Coins
            GameObject first_coin(Vector3d(obstacle_lane * LANE_WIDTH, 0.3, last_position - subway_obstacle_distance),
                "coin");
            subway_obstacles.push_back(first_coin);
            for (int i = 0; i < 4; i++) {
                last_position = subway_obstacles.back().position.z;
                GameObject current_coin(Vector3d(obstacle_lane * LANE_WIDTH, 0.3, last_position - subway_coin_distance),
                    "coin");
                subway_obstacles.push_back(current_coin);
            }
            
        }
        else if (obstacle_type == 1) {
            // Subway
            GameObject subway(Vector3d(obstacle_lane * LANE_WIDTH, 0, last_position - subway_obstacle_distance),
                "subway");
            subway_obstacles.push_back(subway);
        }
        else {
            // Barricade
            GameObject barricade(Vector3d(obstacle_lane * LANE_WIDTH, 0, last_position - subway_obstacle_distance),
                "barricade");
            subway_obstacles.push_back(barricade);
        }

    }

    // Move obstacles backwards and check for collision
    deque<GameObject> new_obstacles;
    for (GameObject& obj : subway_obstacles) {
        double rewind_distance = game_speed * rewind_amount;
        double prev_position = obj.position.z;
        obj.position.z += rewind_distance;
        double width = 0.4;

        if (obj.object_type == "coin") {
            if (abs(player.position.x - obj.position.x) < 0.4 &&
                abs(player.position.y - obj.position.y) < 0.4 &&
                ((prev_position + width) <= player.position.z && player.position.z <= (obj.position.z + width))
                )
            {
                game_score++;
                audioManager.Play("coin.wav", 0.5f, false);
                continue;
            }
        }
        else if (obj.object_type == "subway") {
            width = 3;
            if (abs(player.position.x - obj.position.x) < LANE_WIDTH / 2 &&
                ((prev_position + width) <= player.position.z && player.position.z <= (obj.position.z + width))
                ) 
            {
                gameOver();
                return;
            }
        }
        else if (obj.object_type == "barricade") {
            if (abs(player.position.x - obj.position.x) < 0.4 &&
                abs(player.position.y - obj.position.y) < 0.4 &&
                ((prev_position + width) <= player.position.z && player.position.z <= (obj.position.z + width))
                ) 
            {
                gameOver();
                return;
            }
        }
        new_obstacles.push_back(obj);
    }

    subway_obstacles = new_obstacles;

    // Delete out of view obstacles
    if (!subway_obstacles.empty()) {
        double behind_camera = camera.position.z + 2 * subway_obstacle_distance;
        while (subway_obstacles.front().position.z > behind_camera)
            subway_obstacles.pop_front();
    }

    // Move player towards horizontal target
    double reposition_amount = (player_horizontal_target.x - player.position.x) * player_horizontal_easing;
    player.position.x += reposition_amount;

    if (!camera_debug) {
        // Move camera behind player
        camera.position.x = player.position.x;
        camera.target.x = player.position.x;

        // Move camera while jumping in first person
        if (fps_view) {
            camera.position.y = player.position.y + 2;
            camera.target.y = player.position.y + 2;
        }
    }

    // Move player down
    player.position.y += player_falling_velocity;
    player_falling_velocity -= player_falling_gravity;

    // Check for floor collision
    if (player.position.y < 0) {
        player.position.y = 0;
        player_falling_velocity = 0;
        player_moving_state = "running";
    }


    // Move light around scene
    double current_time = chrono::steady_clock::now().time_since_epoch().count();
    double light_height = (double)current_time / (double)10000000000;
    subway_light_pos.x = cos(light_height);
    subway_light_pos.y = sin(light_height);

    if (subway_light_pos.y < 0) {
        subway_light_color = { 0.0, 0.0, 0.0};
    }
    else {
        if (subway_light_pos.x > 0.5) {
            subway_light_color = { 0.970, 0.293, 0.0873 };
        }
        else if (subway_light_pos.x < -0.5) {
            subway_light_color = { 0.970, 0.293, 0.0873 };
            
        }
        else {
            subway_light_color = { 1.00, 0.987, 0.840 };
        }
        
    }

    //Increase game speed
    game_speed += 0.0005;

    camera.project();
}

void subwayDisplay() {
    setupSubwayCamera();
    setupSubwayLights();

    double distance_from_camera = 200;

    glDisable(GL_LIGHTING);
    glBindTexture(GL_TEXTURE_2D, tex_sky.texture[0]);

    if (subway_light_pos.y < 0)
        glColor3d(0.3, 0.3, 0.3);
    else
        glColor3d(1.0, 1.0, 1.0);
    glNormal3f(0, 0, 1);
    glBegin(GL_QUAD_STRIP);
    glTexCoord2f(0, 0);
    glVertex3d(-LANE_WIDTH * 20, 0, -distance_from_camera);
    glTexCoord2f(0, 1);
    glVertex3d(-LANE_WIDTH * 20, 50, -distance_from_camera);
    glTexCoord2f(2, 0);
    glVertex3d(LANE_WIDTH * 20, 0, -distance_from_camera);
    glTexCoord2f(2, 1);
    glVertex3d(LANE_WIDTH * 20, 50, -distance_from_camera);
    glEnd();
    glEnable(GL_LIGHTING);

    // Floor Rendering

    glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);
    glNormal3f(0, 1, 0);
    glBegin(GL_QUAD_STRIP);
    for (auto& value : floor_vertices) {
        glTexCoord2f(value.second.x, value.second.y);
        glVertex3d(value.first.x, value.first.y, value.first.z);
    }
    glEnd();

    // Rail Rendering

    for (auto rail : rail_positions) {
        for (int i = -1; i <= 1; i++) {
            glPushMatrix();
            glTranslated(rail.x + i*LANE_WIDTH, rail.y, rail.z);
            model_rail.Draw();
            glPopMatrix();
        }
    }

    // House Rendering


    for (auto house : house_positions) {
        const double horiz_dist = 7.5;
        for (int i = -1; i <= 1; i+=2) {
            glPushMatrix();
            glTranslated(-horiz_dist, house.y, house.z);
            glPushMatrix();
            glScaled(0.1, 0.2, 0.1);
            glRotated(0, 0, 1, 0);
            model_house.Draw();
            glPopMatrix();
            glPopMatrix();


            glPushMatrix();
            glTranslated(horiz_dist, house.y, house.z);
            glPushMatrix();
            glScaled(0.1, 0.2, 0.1);
            glRotated(0, 0, 1, 0);
            model_house.Draw();
            glPopMatrix();
            glPopMatrix();

        }
    }

    // Player Rendering

    drawRunningPlayer();

    // Obstacle Rendering

    for (GameObject obj : subway_obstacles) {
        if(obj.object_type=="coin")
            obj.render(model_coin);
        else if(obj.object_type == "subway")
            obj.render(model_subway);
        else if(obj.object_type == "barricade")
            obj.render(model_barricade);
    }

}

void beachTimer() {

    if (player_moving_state == "lost") {
        game_speed = 0;
        return;
    }


    // Add new ocean vertices
    while ((ocean_vertices.size() / 2) < ocean_vertex_limit) {
        double last_distance = ocean_vertices.empty() ? INITIAL_CAMERA_POSITION.z : ocean_vertices.back().first.z;
        ocean_vertices.push_back({ Vector3d(-LANE_WIDTH * 20, 0, last_distance - ocean_vertex_distance), Vector2d(0, ocean_uv_counter) });
        ocean_vertices.push_back({ Vector3d(LANE_WIDTH * 20, 0, last_distance - ocean_vertex_distance), Vector2d(20, ocean_uv_counter) });
        ocean_uv_counter++;
    }

    // Move the ocean backwards
    for (auto& vertex : ocean_vertices) {
        double rewind_distance = game_speed * rewind_amount;
        vertex.first.z += rewind_distance;
    }

    // Delete ocean vertices which are out of sight
    if (!ocean_vertices.empty()) {
        double behind_camera = camera.position.z + 2 * ocean_vertex_distance;
        while (ocean_vertices.front().first.z > behind_camera)
            ocean_vertices.pop_front();
    }

    // Add obstacles if the deque is not full
    while (ocean_obstacles.size() < ocean_obstacle_count_limit) {
        double last_position = ocean_obstacles.empty() ? -INITIAL_CAMERA_POSITION.z : ocean_obstacles.back().position.z;
        int obstacle_type = random() % 3;
        double obstacle_lane = -1 + (random() % 3);
        if (obstacle_type == 0) {
            // Coins
            GameObject first_coin(Vector3d(obstacle_lane * LANE_WIDTH, 0.3, last_position - ocean_obstacle_distance),
                "coin");
            ocean_obstacles.push_back(first_coin);
            for (int i = 0; i < 4; i++) {
                last_position = ocean_obstacles.back().position.z;
                GameObject current_coin(Vector3d(obstacle_lane * LANE_WIDTH, 0.3, last_position - ocean_coin_distance),
                    "coin");
                ocean_obstacles.push_back(current_coin);
            }

        }
        else if (obstacle_type == 1) {
            // Ship
            GameObject ship(Vector3d(obstacle_lane * LANE_WIDTH, 0, last_position - ocean_obstacle_distance),
                "ship");
            ocean_obstacles.push_back(ship);
        }
        else {
            // Swim Ring
            GameObject swim_ring(Vector3d(obstacle_lane * LANE_WIDTH, 0, last_position - ocean_obstacle_distance),
                "swim_ring");
            ocean_obstacles.push_back(swim_ring);
        }

    }

    // Move obstacles backwards and check for collision
    deque<GameObject> new_obstacles;
    for (GameObject& obj : ocean_obstacles) {
        double rewind_distance = game_speed * rewind_amount;
        double prev_position = obj.position.z;
        obj.position.z += rewind_distance;
        double width = 0.4;

        if (obj.object_type == "coin") {
            if (abs(player.position.x - obj.position.x) < 0.4 &&
                abs(player.position.y - obj.position.y) < 0.4 &&
                ((prev_position + width) <= player.position.z && player.position.z <= (obj.position.z + width))
                )
            {
                game_score++;
                audioManager.Play("coin.wav", 0.5f, false);
                continue;
            }
        }
        else if (obj.object_type == "ship") {
            width = 4;
            if (abs(player.position.x - obj.position.x) < LANE_WIDTH / 2 &&
                ((prev_position - width) <= player.position.z && player.position.z <= (obj.position.z + width))
                )
            {
                gameOver();
                return;
            }
        }
        else if(obj.object_type == "swim_ring") {
            width = 2;
            if (abs(player.position.x - obj.position.x) < 0.4 &&
                abs(player.position.y - obj.position.y) < 0.4 &&
                ((prev_position - width) <= player.position.z && player.position.z <= (obj.position.z + width))
                )
            {
                gameOver();
                return;
            }
        }
        new_obstacles.push_back(obj);
    }

    ocean_obstacles = new_obstacles;

    // Delete out of view obstacles
    if (!ocean_obstacles.empty()) {
        double behind_camera = camera.position.z + 2 * ocean_obstacle_distance;
        while (ocean_obstacles.front().position.z > behind_camera)
            ocean_obstacles.pop_front();
    }


    // Move player towards horizontal target
    double reposition_amount = (player_horizontal_target.x - player.position.x) * player_horizontal_easing;
    player.position.x += reposition_amount;

    if (!camera_debug) {
        // Move camera behind player
        camera.position.x = player.position.x;
        camera.target.x = player.position.x;

        // Move camera while jumping in first person
        if (fps_view) {
            camera.position.y = player.position.y + 2;
            camera.target.y = player.position.y + 2;
        }
    }

    // Move player down
    player.position.y += player_falling_velocity;
    player_falling_velocity -= player_falling_gravity;

    // Check for floor collision
    if (player.position.y < 0) {
        player.position.y = 0;
        player_falling_velocity = 0;
        player_moving_state = "running";
    }

    // Move light around scene
    double current_time = chrono::steady_clock::now().time_since_epoch().count();
    double light_height = (double)current_time / (double)10000000000;
    beach_light_pos.x = cos(light_height);
    beach_light_pos.y = sin(light_height);

    if (beach_light_pos.y < 0) {
        beach_light_color = { 0.155, 0.426, 0.620 };
    }
    else {
        beach_light_color = { 1,1,1 };
    }

    //Increase game speed
    game_speed += 0.001;

}

void beachDisplay() {
    setupBeachCamera();
    setupBeachLights();

    

    // Sky Rendering

    double distance_from_camera = 200;

    glDisable(GL_LIGHTING);
    glBindTexture(GL_TEXTURE_2D, tex_sky.texture[0]);

    if(beach_light_pos.y < 0)
        glColor3d(0.155, 0.426, 0.620);
    else
        glColor3d(1.0, 1.0, 1.0);
    glNormal3f(0, 0, 1);
    glBegin(GL_QUAD_STRIP);
    glTexCoord2f(0, 0);
    glVertex3d(-LANE_WIDTH*20, 0, -distance_from_camera);
    glTexCoord2f(0, 1);
    glVertex3d(-LANE_WIDTH * 20, 50, -distance_from_camera);
    glTexCoord2f(2, 0);
    glVertex3d(LANE_WIDTH * 20, 0, -distance_from_camera);
    glTexCoord2f(2, 1);
    glVertex3d(LANE_WIDTH * 20, 50, -distance_from_camera);
    glEnd();
    glEnable(GL_LIGHTING);

    // Ocean Rendering

    glBindTexture(GL_TEXTURE_2D, tex_ocean.texture[0]);
    glNormal3f(0, 1, 0);

    if (beach_light_pos.y < 0) {
        glDisable(GL_LIGHTING);
        glColor3d(0.4, 0.55, 0.4);
    }
    else {

        glColor3d(1.0, 1.0, 1.0);
    }

    glBegin(GL_QUAD_STRIP);
    for (auto& value : ocean_vertices) {
        glTexCoord2f(value.second.x, value.second.y);
        glVertex3d(value.first.x, value.first.y, value.first.z);
    }
    glEnd();


    glEnable(GL_LIGHTING);

    // Player Rendering

    drawSurfingPlayer();


    // Obstacle Rendering

    for (GameObject obj : ocean_obstacles) {
        if (obj.object_type == "coin")
            obj.render(model_coin);
        else if (obj.object_type == "ship")
            obj.render(model_ship);
        else if (obj.object_type == "swim_ring")
            obj.render(model_swim_ring);
    }

    camera.project();
}



void setupSubwayCamera() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	
    gluPerspective(CAMERA_FIELD_OF_VIEW, CAMERA_ASPECT_RATIO, CAMERA_Z_NEAR, CAMERA_Z_FAR);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    
    camera.project();
}

void setupSubwayLights(){
    // Light 0
    GLfloat lightPosition[] = { subway_light_pos.x, subway_light_pos.y, subway_light_pos.z, 0.0 };
    GLfloat lightIntensity[] = { 4, 4, 4, 4 };
    GLfloat lightColor[] = { subway_light_color[0], subway_light_color[1], subway_light_color[2] };
    glMaterialfv(GL_FRONT, GL_SPECULAR, lightColor);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_INTENSITY, lightIntensity);

    // Light 1
    float diffuse_amount = 1.3;
    if (subway_light_pos.y > 0)
        diffuse_amount = 0;
    GLfloat l1Diffuse[] = { diffuse_amount, diffuse_amount, diffuse_amount, diffuse_amount };
    GLfloat l1Ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    GLfloat l1Position[] = { 0.0f, 0.0f, 20.0f, 0.0f };
    GLfloat l1Direction[] = { 0.0, 0.0, -1.0 };
    glLightfv(GL_LIGHT1, GL_DIFFUSE, l1Diffuse);
    glLightfv(GL_LIGHT1, GL_AMBIENT, l1Ambient);
    glLightfv(GL_LIGHT1, GL_POSITION, l1Position);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0);
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 90.0);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, l1Direction);
    
}

void setupBeach() {

    
}

void setupBeachCamera() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(CAMERA_FIELD_OF_VIEW, CAMERA_ASPECT_RATIO, CAMERA_Z_NEAR, CAMERA_Z_FAR);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    camera.project();
}
void setupBeachLights() {
    // Light 0
    GLfloat lightPosition[] = { beach_light_pos.x, beach_light_pos.y, beach_light_pos.z, 0.0 };
    GLfloat lightIntensity[] = { 1, 1, 1, 1 };
    GLfloat lightColor[] = { beach_light_color[0], beach_light_color[1], beach_light_color[2] };
    glMaterialfv(GL_FRONT, GL_SPECULAR, lightColor);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_INTENSITY, lightIntensity);

    // Light 1
    GLfloat l1Diffuse[] = { 0.4f, 0.4f, 0.4f, 0.4f };
    GLfloat l1Ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    GLfloat l1Position[] = { 0.0f, 0.0f, 1.0f, 0.0f };
    GLfloat l1Direction[] = { 0.0, -1.0, 0.0 };
    glLightfv(GL_LIGHT1, GL_DIFFUSE, l1Diffuse);
    glLightfv(GL_LIGHT1, GL_AMBIENT, l1Ambient);
    glLightfv(GL_LIGHT1, GL_POSITION, l1Position);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0);
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 90.0);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, l1Direction);
}

void input(int key, int x, int y) {
    switch (key) {

    case GLUT_KEY_UP:
        playerJump();
        break;
    case GLUT_KEY_DOWN:
        player_falling_velocity = -0.2;
        break;
    case GLUT_KEY_LEFT:
        playerMove(true);
        break;
    case GLUT_KEY_RIGHT:
        playerMove(false);
        break;

    }
}

void keyboard(unsigned char key, int x, int y) {

    switch (key) {
        // Escape character
    case 'q':
        camera.position.x += 0.1;
        break;
    case 'a':
        camera.position.x -= 0.1;
        break;
    case 'w':
        camera.position.y += 0.1;
        break;
    case 's':
        camera.position.y -= 0.1;
        break;
    case 'e':
        camera.position.z += 0.1;
        break;
    case 'd':
        camera.position.z -= 0.1;
        break;
    case 'r':
        camera.target.x += 0.1;
        break;
    case 'f':
        camera.target.x -= 0.1;
        break;
    case 't':
        camera.target.y += 0.1;
        break;
    case 'g':
        camera.target.y -= 0.1;
        break;
    case 'y':
        camera.target.z += 0.1;
        break;
    case 'h':
        camera.target.z -= 0.1;
        break;

    case 'c':
        camera_debug ^= 1;
        break;
    case 'b':
        switchStage();
        break;
    case 'v':
        switchCamera();
        break;
    }
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        playerJump();
    }
}

void loadAssets() {
    tex_ground.Load("textures/ground_dirt.bmp");
    tex_ocean.Load("textures/ocean_texture.tga");
    tex_sky.Load("textures/sky_texture.tga");

    model_rail.Load("models/rail/rail.3ds");
    model_subway.Load("models/subway/subway.3ds");
    model_barricade.Load("models/barricade/barricade.3ds");
    model_coin.Load("models/coin/coin.3ds");
    model_ship.Load("models/ship/ship.3ds");
    model_swim_ring.Load("models/swim_ring/swim_ring.3ds");
    model_house.Load("models/house/house.3ds");

    for (int i = 0; i < 4; i++) {
        string file_path = "models/player_running/player_running_" + to_string(i+1) + ".3ds";
        char* char_path = strdup(file_path.c_str());
        model_player_running[i].Load(char_path);
    }
    model_player_running_jump.Load("models/player_running/player_running_jump.3ds");
    model_player_rising_jump.Load("models/player_running/player_rising_jump.3ds");
    model_player_running_left.Load("models/player_running/player_running_left.3ds");
    model_player_running_right.Load("models/player_running/player_running_right.3ds");
    model_player_splat.Load("models/player_running/player_splat.3ds");

    model_player_surfing.Load("models/player_surfing/player_surfing.3ds");
    model_player_surfing_jump.Load("models/player_surfing/player_surfing_jump.3ds");
    model_player_surfing_left.Load("models/player_surfing/player_surfing_left.3ds");
    model_player_surfing_right.Load("models/player_surfing/player_surfing_right.3ds");
    
    
    for (int i = 0; i < 2; i++) {
        string file_path = "models/player_surfing/player_surfing_splash_" + to_string(i + 1) + ".3ds";
        char* char_path = strdup(file_path.c_str());
        model_player_surfing_splash[i].Load(char_path);
    }

}

void drawRunningPlayer() {
    if (player_moving_state == "lost") {
        // Draw splat here
        
            glPushMatrix();
        glTranslated(player.position.x+0.1, player.position.y+3.5, player.position.z+7);
        glPushMatrix();
        glScaled(0.15, 0.15, 0.15);
        glRotated(120, 0, 0, 1);
        model_player_splat.Draw();
        glPopMatrix();
        glPopMatrix();
        return;
    }

    if (player_moving_state == "jumping") {
        glPushMatrix();
            glTranslated(player.position.x, player.position.y, player.position.z);
            glPushMatrix();
                glScaled(0.15, 0.15, 0.15);
                glRotated(180, 0, 1, 0);
                if(player_falling_velocity>0.1)
                    model_player_rising_jump.Draw();
                else
                    model_player_running_jump.Draw();
            glPopMatrix();
        glPopMatrix();
        return;
    }

    glPushMatrix();
        glTranslated(player.position.x, player.position.y, player.position.z);
        glPushMatrix();
            glScaled(0.15, 0.15, 0.15);
            glRotated(180, 0, 1, 0);    
            if (abs(player.position.x - player_horizontal_target.x) > 0.3) {
                if(player.position.x> player_horizontal_target.x)
                    model_player_running_left.Draw();
                else
                    model_player_running_right.Draw();
            }
            else
                model_player_running[running_animation_index].Draw();
        glPopMatrix();
    glPopMatrix();

    running_animation_counter++;
    if (running_animation_counter == running_animation_timings[running_animation_index]) {
        running_animation_counter = 0;
        running_animation_index = (running_animation_index + 1) % 4;
    }

}

void drawSurfingPlayer() {
    if (player_moving_state == "lost") {
        // Draw splat here

        glPushMatrix();
        glTranslated(player.position.x + 0.1, player.position.y + 3.5, player.position.z + 7);
        glPushMatrix();
        glScaled(0.15, 0.15, 0.15);
        glRotated(120, 0, 0, 1);
        model_player_splat.Draw();
        glPopMatrix();
        glPopMatrix();
        return;
    }
    if (player_moving_state == "jumping") {
        glPushMatrix();
        glTranslated(player.position.x, player.position.y, player.position.z);
        glPushMatrix();
        glScaled(0.15, 0.15, 0.15);
        glRotated(180, 0, 1, 0);
        glRotated(surfing_jump_angle, 1, 0, 0);

            model_player_surfing_jump.Draw();
        glPopMatrix();
        glPopMatrix();
        surfing_jump_angle += surfing_jump_rotation;
        return;
    }

    glPushMatrix();
    glTranslated(player.position.x, player.position.y, player.position.z);
    glPushMatrix();
    glScaled(0.15, 0.15, 0.15);
    glRotated(180, 0, 1, 0);
    if (abs(player.position.x - player_horizontal_target.x) > 0.3) {
        if (player.position.x > player_horizontal_target.x)
            model_player_surfing_left.Draw();
        else
            model_player_surfing_right.Draw();
    }
    else
        model_player_surfing.Draw();
    glPopMatrix();
    glPopMatrix();


    // Splash Rendering
    glColor3d(1.0, 1.0, 1.0);
    glPushMatrix();
    glTranslated(player.position.x, player.position.y, player.position.z);
    glPushMatrix();
    if (abs(player.position.x - player_horizontal_target.x) > 0.3) {
        if (player.position.x > player_horizontal_target.x)
            glRotated(210, 0, 1, 0);
        else
            glRotated(150, 0, 1, 0);
    }
    else
        glRotated(180, 0, 1, 0);
    
    glScaled(0.15, 0.15, 0.15);
    model_player_surfing_splash[splash_current].Draw();
    glPopMatrix();
    glPopMatrix();


    splash_counter++;
    if (splash_counter == splash_counter_max) {
        splash_current ^= 1;
        splash_counter = 0;
    }

}

void playerJump() {
    if (player_moving_state == "lost")
        return;
    surfing_jump_angle = 0;
    if (player_moving_state == "jumping")
        return;
    player_moving_state = "jumping";
    player_falling_velocity += initial_jump_height;
    if (player_moving_state != "lost") {
        audioManager.Play("whoosh.wav", 0.5f, false);
    }
}

void playerMove(bool left) {
    if (player_moving_state == "lost")
        return;
    double new_x = player_horizontal_target.x;
    
    if (left)
        new_x = max(-LANE_WIDTH, new_x - LANE_WIDTH);
    else
        new_x = min(LANE_WIDTH, new_x + LANE_WIDTH);

    player_horizontal_target.x = new_x;

    if (player_moving_state != "lost") {
        audioManager.Play("whoosh.wav", 0.5f, false);
    }
}

void switchCamera() {
    if (player_moving_state == "lost")
        return;

    fps_view ^= 1;

    if (!fps_view) {
        camera.position = INITIAL_CAMERA_POSITION;
        camera.target = INITIAL_CAMERA_TARGET;
    }
    else {
        camera.position = FPS_CAMERA_POSITION;
        camera.target = FPS_CAMERA_TARGET;
    }
}

void switchStage() {
    if (player_moving_state == "lost")
        return;

    game_stage = game_stage == 1 ? 2 : 1;
}

void gameOver() {
    if (fps_view)
        switchCamera();
    game_speed = 0.0;
    player_moving_state = "lost";
    audioManager.Play("crash.wav", 0.5f, false);
}

int random() {
    return rng() % 10000;
}

void printText(int x, int y, char* string){

    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);
    glWindowPos2i(x, y);
    int len, i;
    len = (int)strlen(string);
    for (i = 0; i < len; i++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
    }

    glEnable(GL_TEXTURE_2D);
}