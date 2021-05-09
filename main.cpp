/*******************************************************************************************
*
*   raylib - sample game: asteroids survival
*
*   Sample game developed by Ian Eito, Albert Martos and Ramon Santamaria
*
*   This game has been created using raylib v1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include <math.h>
#include<vector>
using namespace std;

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Some Defines
//----------------------------------------------------------------------------------
#define PLAYER_BASE_SIZE    20.0f
#define PLAYER_SPEED        6.0f
#define PLAYER_MAX_SHOOTS   10
#define PLAYER_MAX_HP 50

#define METEORS_SPEED       2
#define MAX_MEDIUM_METEORS  3
#define MAX_SMALL_METEORS   3

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct Player {
    Vector2 position;
    Vector2 speed;
    float acceleration;
    float rotation;
    Vector3 collider;
    Color color;
    float hp;
} Player;

typedef struct Meteor {
    Vector2 position;
    Vector2 speed;
    float radius;
    bool active;
    Color color;
} Meteor;

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
static const int screenWidth = 800;
static const int screenHeight = 450;

static int framesCounter = 0;
static bool gameOver = false;
static bool pause = false;

// NOTE: Defined triangle is isosceles with common angles of 70 degrees.
static float shipHeight = 0.0f;

static vector<Player> players(2);
static vector<Meteor> mediumMeteor;
static vector<Meteor> smallMeteor;

//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
static void InitGame(void);         // Initialize game
static void UpdateGame(void);       // Update game (one frame)
static void DrawGame(void);         // Draw game (one frame)
static void UnloadGame(void);       // Unload game
static void UpdateDrawFrame(void);  // Update and Draw (one frame)

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization (Note windowTitle is unused on Android)
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "sample game: asteroids survival");

    InitGame();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update and Draw
        //----------------------------------------------------------------------------------
        UpdateDrawFrame();
        //----------------------------------------------------------------------------------
    }
#endif
    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadGame();         // Unload loaded data (textures, sounds, models...)
    
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//------------------------------------------------------------------------------------
// Module Functions Definitions (local)
//------------------------------------------------------------------------------------

// Initialize game variables
void InitGame(void)
{
    int posx, posy;
    int velx, vely;
    bool correctRange = false;

    pause = false;

    framesCounter = 0;

    shipHeight = (PLAYER_BASE_SIZE/2)/tanf(20*DEG2RAD);

    // Initialization player
    for (int i = 0; i < 2; i++) {
        players[i].position = (Vector2){screenWidth/2, screenHeight/2 - shipHeight/2};
        players[i].speed = (Vector2){0, 0};
        players[i].acceleration = 0;
        players[i].rotation = 0;
        players[i].collider = (Vector3){players[i].position.x + sin(players[i].rotation*DEG2RAD)*(shipHeight/2.5f), players[i].position.y - cos(players[i].rotation*DEG2RAD)*(shipHeight/2.5f), 12};
        players[i].hp = PLAYER_MAX_HP;
    }
    players[0].color = LIGHTGRAY;
    players[1].color = DARKGRAY;
    
    for (int i = 0; i < MAX_MEDIUM_METEORS; i++)
    {
        posx = GetRandomValue(0, screenWidth);

        while(!correctRange)
        {
            if (posx > screenWidth/2 - 150 && posx < screenWidth/2 + 150) posx = GetRandomValue(0, screenWidth);
            else correctRange = true;
        }

        correctRange = false;

        posy = GetRandomValue(0, screenHeight);

        while(!correctRange)
        {
            if (posy > screenHeight/2 - 150 && posy < screenHeight/2 + 150)  posy = GetRandomValue(0, screenHeight);
            else correctRange = true;
        }

        correctRange = false;
        velx = GetRandomValue(-METEORS_SPEED, METEORS_SPEED);
        vely = GetRandomValue(-METEORS_SPEED, METEORS_SPEED);

        while(!correctRange)
        {
            if (velx == 0 && vely == 0)
            {
                velx = GetRandomValue(-METEORS_SPEED, METEORS_SPEED);
                vely = GetRandomValue(-METEORS_SPEED, METEORS_SPEED);
            }
            else correctRange = true;
        }
        mediumMeteor.push_back(Meteor());
        mediumMeteor.back().position = (Vector2){static_cast<float>(posx), static_cast<float>(posy)};
        mediumMeteor.back().speed = (Vector2){static_cast<float>(velx), static_cast<float>(vely)};
        mediumMeteor.back().radius = 20;
        mediumMeteor.back().active = true;
        mediumMeteor.back().color = GREEN;
    }

    for (int i = 0; i < MAX_SMALL_METEORS; i++)
    {
        posx = GetRandomValue(0, screenWidth);

        while(!correctRange)
        {
            if (posx > screenWidth/2 - 150 && posx < screenWidth/2 + 150) posx = GetRandomValue(0, screenWidth);
            else correctRange = true;
        }

        correctRange = false;

        posy = GetRandomValue(0, screenHeight);

        while(!correctRange)
        {
            if (posy > screenHeight/2 - 150 && posy < screenHeight/2 + 150)  posy = GetRandomValue(0, screenHeight);
            else correctRange = true;
        }

        correctRange = false;
        velx = GetRandomValue(-METEORS_SPEED, METEORS_SPEED);
        vely = GetRandomValue(-METEORS_SPEED, METEORS_SPEED);

        while(!correctRange)
        {
            if (velx == 0 && vely == 0)
            {
                velx = GetRandomValue(-METEORS_SPEED, METEORS_SPEED);
                vely = GetRandomValue(-METEORS_SPEED, METEORS_SPEED);
            }
            else correctRange = true;
        }
        smallMeteor.push_back(Meteor());
        smallMeteor.back().position = (Vector2){static_cast<float>(posx), static_cast<float>(posy)};
        smallMeteor.back().speed = (Vector2){static_cast<float>(velx), static_cast<float>(vely)};
        smallMeteor.back().radius = 10;
        smallMeteor.back().active = true;
        smallMeteor.back().color = YELLOW;
    }
}

// Update game (one frame)
void UpdateGame(void)
{
    if (!gameOver)
    {
        if (IsKeyPressed('P')) pause = !pause;

        if (!pause)
        {
            framesCounter++;

            // Player logic

            // Rotation
            if (IsKeyDown(KEY_LEFT)) players[0].rotation -= 5;
            if (IsKeyDown(KEY_RIGHT)) players[0].rotation += 5;
            
            if (IsKeyDown(KEY_A)) players[1].rotation -= 5;
            if (IsKeyDown(KEY_D)) players[1].rotation += 5;

            // Speed
            for (int i = 0; i < 2; i++) {
                players[i].speed.x = sin(players[i].rotation*DEG2RAD)*PLAYER_SPEED;
                players[i].speed.y = cos(players[i].rotation*DEG2RAD)*PLAYER_SPEED;
            }

            // Controller
            if (IsKeyDown(KEY_UP))
            {
                if (players[0].acceleration < 1) players[0].acceleration += 0.04f;
            }
            else
            {
                if (players[0].acceleration > 0) players[0].acceleration -= 0.02f;
                else if (players[0].acceleration < 0) players[0].acceleration = 0;
            }
            if (IsKeyDown(KEY_DOWN))
            {
                if (players[0].acceleration > 0) players[0].acceleration -= 0.04f;
                else if (players[0].acceleration < 0) players[0].acceleration = 0;
            }
            
            if (IsKeyDown(KEY_W))
            {
                if (players[1].acceleration < 1) players[1].acceleration += 0.04f;
            }
            else
            {
                if (players[1].acceleration > 0) players[1].acceleration -= 0.02f;
                else if (players[1].acceleration < 0) players[1].acceleration = 0;
            }
            if (IsKeyDown(KEY_S))
            {
                if (players[1].acceleration > 0) players[1].acceleration -= 0.04f;
                else if (players[1].acceleration < 0) players[1].acceleration = 0;
            }

            // Movement
            for (int i = 0; i < 2; i++) {
                players[i].position.x += (players[i].speed.x*players[i].acceleration);
                players[i].position.y -= (players[i].speed.y*players[i].acceleration);

            }
            
            // Wall behaviour for player
            for (int i = 0; i < 2; i++) {
                if (players[i].position.x > screenWidth ) players[i].position.x = screenWidth;
                else if (players[i].position.x < -(shipHeight)) players[i].position.x = 0;
                if (players[i].position.y > (screenHeight )) players[i].position.y = screenHeight;
                else if (players[i].position.y < -(shipHeight)) players[i].position.y = 0;
            }
            

            // Collision Player to meteors
            int ml = int(mediumMeteor.size());
            int sl = int(smallMeteor.size());
            for (int i = 0; i < 2; i++) {
                players[i].collider = (Vector3){players[i].position.x + sin(players[i].rotation*DEG2RAD)*(shipHeight/2.5f), players[i].position.y - cos(players[i].rotation*DEG2RAD)*(shipHeight/2.5f), 12};
                for (int a = 0; a < ml; ++a)
                {
                    if (CheckCollisionCircles((Vector2){players[i].collider.x, players[i].collider.y}, players[i].collider.z, mediumMeteor[a].position, mediumMeteor[a].radius) && mediumMeteor[a].active)
                     {
                       players[i].hp -= 10;
                       vector<Meteor>::iterator it = mediumMeteor.begin() + a;
                       mediumMeteor.erase(it);
                       if(players[i].hp <=0)gameOver = true;
                     
                     }
                }
            }
            
            for (int a=0 ;a < sl; ++a)
            {
                for (int i = 0; i < 2; i++){
                    if (CheckCollisionCircles((Vector2){players[i].collider.x, players[i].collider.y}, players[i].collider.z, smallMeteor[a].position, smallMeteor[a].radius) && smallMeteor[a].active)
                      {
                       players[i].hp -= 5;
                        vector<Meteor>::iterator it = smallMeteor.begin() + a;
                       smallMeteor.erase(it);
                       if(players[i].hp <=0)gameOver = true;
                     
                     }
                }
            }

            // Meteor logic

            for (int i=0; i< ml; i++)
            {
                if (mediumMeteor[i].active)
                {
                    // movement
                    mediumMeteor[i].position.x += mediumMeteor[i].speed.x;
                    mediumMeteor[i].position.y += mediumMeteor[i].speed.y;

                    // wall behaviour
                    if  (mediumMeteor[i].position.x > screenWidth + mediumMeteor[i].radius) mediumMeteor[i].position.x = -(mediumMeteor[i].radius);
                    else if (mediumMeteor[i].position.x < 0 - mediumMeteor[i].radius) mediumMeteor[i].position.x = screenWidth + mediumMeteor[i].radius;
                    if (mediumMeteor[i].position.y > screenHeight + mediumMeteor[i].radius) mediumMeteor[i].position.y = -(mediumMeteor[i].radius);
                    else if (mediumMeteor[i].position.y < 0 - mediumMeteor[i].radius) mediumMeteor[i].position.y = screenHeight + mediumMeteor[i].radius;
                }
            }

            for (int i=0; i< sl; ++i)
            {
                if (smallMeteor[i].active)
                {
                    // movement
                    smallMeteor[i].position.x += smallMeteor[i].speed.x;
                    smallMeteor[i].position.y += smallMeteor[i].speed.y;

                    // wall behaviour
                    if  (smallMeteor[i].position.x > screenWidth + smallMeteor[i].radius) smallMeteor[i].position.x = -(smallMeteor[i].radius);
                    else if (smallMeteor[i].position.x < 0 - smallMeteor[i].radius) smallMeteor[i].position.x = screenWidth + smallMeteor[i].radius;
                    if (smallMeteor[i].position.y > screenHeight + smallMeteor[i].radius) smallMeteor[i].position.y = -(smallMeteor[i].radius);
                    else if (smallMeteor[i].position.y < 0 - smallMeteor[i].radius) smallMeteor[i].position.y = screenHeight + smallMeteor[i].radius;
                }
            }
        }
    }
    else
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            InitGame();
            gameOver = false;
        }
    }
}

// Draw game (one frame)
void DrawGame(void)
{
    BeginDrawing();

        ClearBackground(RAYWHITE);
        
        if (!gameOver)
        {
            // Draw spaceship
            for (int i = 0; i < 2; i++) {
                Vector2 v1 = { players[i].position.x + sinf(players[i].rotation*DEG2RAD)*(shipHeight), players[i].position.y - cosf(players[i].rotation*DEG2RAD)*(shipHeight) };
                Vector2 v2 = { players[i].position.x - cosf(players[i].rotation*DEG2RAD)*(PLAYER_BASE_SIZE/2), players[i].position.y - sinf(players[i].rotation*DEG2RAD)*(PLAYER_BASE_SIZE/2) };
                Vector2 v3 = { players[i].position.x + cosf(players[i].rotation*DEG2RAD)*(PLAYER_BASE_SIZE/2), players[i].position.y + sinf(players[i].rotation*DEG2RAD)*(PLAYER_BASE_SIZE/2) };
                DrawTriangle(v1, v2, v3, MAROON);
                DrawRectangle(players[i].position.x-20, players[i].position.y-20,players[i].hp*3, 3, RED);
            }

            // Draw meteor
            for (int i = 0;i< mediumMeteor.size(); i++)
            {
                if (mediumMeteor[i].active) DrawCircleV(mediumMeteor[i].position, mediumMeteor[i].radius, GRAY);
                else DrawCircleV(mediumMeteor[i].position, mediumMeteor[i].radius, Fade(LIGHTGRAY, 0.3f));
            }

            for (int i = 0;i< smallMeteor.size(); i++)
            {
                if (smallMeteor[i].active) DrawCircleV(smallMeteor[i].position, smallMeteor[i].radius, DARKGRAY);
                else DrawCircleV(smallMeteor[i].position, smallMeteor[i].radius, Fade(LIGHTGRAY, 0.3f));
            }

            DrawText(TextFormat("TIME: %.02f", (float)framesCounter/60), 10, 10, 20, BLACK);

            if (pause) DrawText("GAME PAUSED", screenWidth/2 - MeasureText("GAME PAUSED", 40)/2, screenHeight/2 - 40, 40, GRAY);
        }
        else DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, GRAY);

    EndDrawing();
    //----------------------------------------------------------------------------------
}

// Unload game variables
void UnloadGame(void)
{
    // TODO: Unload all dynamic loaded data (textures, sounds, models...)
}

// Update and Draw (one frame)
void UpdateDrawFrame(void)
{
    UpdateGame();
    DrawGame();
}
