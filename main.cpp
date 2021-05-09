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
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
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
#define MAX_MEDIUM_METEORS  20
#define MAX_SMALL_METEORS   20

#define BOSS_BASE_SIZE      50.0f
#define BOSS_SPEED          3.0f
#define BOSS_MAX_HP         1000

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


typedef struct Boss {
    Vector2 position;
    Vector2 speed;
    float acceleration;
    float rotation;
    Vector3 collider;
    Color color;
    float hp;
} Boss;

// Meteors are emited by boss
typedef struct Meteor {
    Vector2 position;
    Vector2 speed;
    float radius;
    bool active;
    Color color;
} Meteor;

// Bullet are emited by player
typedef struct Bullet {
    Vector2 position;
    Vector2 speed;
    float radius;
    bool active;
    Color color;
} Bullet;



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
static vector<Boss>   boss(1);
static vector<Meteor> mediumMeteor;
static vector<Meteor> smallMeteor;
static vector<Bullet> bullets;

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
    srand(time(NULL));
    int posx, posy;
    int velx, vely;
    bool correctRange = false;

    pause = false;

    framesCounter = 0;

    shipHeight = (PLAYER_BASE_SIZE/2)/tanf(20*DEG2RAD);

    // Initialising player
    for (int i = 0; i < 2; i++) {
        players[i].position = (Vector2){screenWidth/2, screenHeight/2 - shipHeight/2};
        players[i].speed = (Vector2){0, 0};
        players[i].acceleration = 0;
        players[i].rotation = 0;
        players[i].collider = (Vector3){players[i].position.x + sin(players[i].rotation*DEG2RAD)*(shipHeight/2.5f), players[i].position.y - cos(players[i].rotation*DEG2RAD)*(shipHeight/2.5f), 12};
        players[i].hp = PLAYER_MAX_HP;
    }
    players[0].color = RED;
    players[1].color = BLUE;

    // Initialising boss
    for (int i = 0; i < boss.size(); i++ ) {
        boss[i].position = (Vector2){screenWidth/3, screenHeight/3 - shipHeight/4};
        boss[i].speed = (Vector2){0, 0};
        boss[i].acceleration = 0.1f;
        boss[i].rotation = 0;
        boss[i].collider = (Vector3){boss[i].position.x + sin(boss[i].rotation*DEG2RAD)*(shipHeight/2.5f), boss[i].position.y - cos(boss[i].rotation*DEG2RAD)*(shipHeight/2.5f), 12};
        boss[i].hp = BOSS_MAX_HP;
    }
    boss[0].color = DARKBLUE;

    // Initialising medium meteors
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

    // Initializing small meteors
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

            // Boss logic
            
            // number
            int bossNum = boss.size();

            // Rotation
            if (framesCounter % 300 == 0) {
                for (int i = 0; i < bossNum; i++) {
                    boss[i].rotation = rand() % 360 + 1 - 180;
                }
            }
            
            // Speed
            for (int i = 0; i < bossNum; i++) {
                boss[i].speed.x = sin(boss[i].rotation*DEG2RAD)*BOSS_SPEED;
                boss[i].speed.y = cos(boss[i].rotation*DEG2RAD)*BOSS_SPEED;
            }

            // Movement
            for (int i = 0; i < bossNum; i++) {
                boss[i].position.x += (boss[i].speed.x*boss[i].acceleration);
                boss[i].position.y -= (boss[i].speed.y*boss[i].acceleration);
            }

            // Wall behavior for boss
            for (int i = 0; i < bossNum; i++) {
                if (boss[i].position.x > screenWidth ) boss[i].position.x = screenWidth;
                else if (boss[i].position.x < -(shipHeight)) boss[i].position.x = 0;
                if (boss[i].position.y > (screenHeight )) boss[i].position.y = screenHeight;
                else if (boss[i].position.y < -(shipHeight)) boss[i].position.y = 0;
            }


            // Player logic

            // Rotation
            if (IsKeyDown(KEY_UP)) players[0].rotation = 0;
            if (IsKeyDown(KEY_DOWN)) players[0].rotation = 180;
            if (IsKeyDown(KEY_LEFT)) players[0].rotation = -90;
            if (IsKeyDown(KEY_RIGHT)) players[0].rotation = 90;
            
            if (IsKeyDown(KEY_W)) players[1].rotation = 0;
            if (IsKeyDown(KEY_S)) players[1].rotation = 180;
            if (IsKeyDown(KEY_A)) players[1].rotation = -90;
            if (IsKeyDown(KEY_D)) players[1].rotation = 90;
            
            int current_direction_1 = players[0].rotation;
            int current_direction_2 = players[1].rotation;

            // Speed
            for (int i = 0; i < 2; i++) {
                players[i].speed.x = sin(players[i].rotation*DEG2RAD)*PLAYER_SPEED;
                players[i].speed.y = cos(players[i].rotation*DEG2RAD)*PLAYER_SPEED;
            }

            // Controller
            if (current_direction_1 == 0) {
                if (IsKeyDown(KEY_UP))
                {
                    if (players[0].acceleration < 1) players[0].acceleration += 0.04f;
                }
                else
                {
                    if (players[0].acceleration > 0) players[0].acceleration -= 0.02f;
                    else if (players[0].acceleration < 0) players[0].acceleration = 0;
                }
            }
            if (current_direction_1 == 180) {
                if (IsKeyDown(KEY_DOWN))
                {
                    if (players[0].acceleration < 1) players[0].acceleration += 0.04f;
                }
                else
                {
                    if (players[0].acceleration > 0) players[0].acceleration -= 0.02f;
                    else if (players[0].acceleration < 0) players[0].acceleration = 0;
                }
            }
            if (current_direction_1 == -90) {
                if (IsKeyDown(KEY_LEFT))
                {
                    if (players[0].acceleration < 1) players[0].acceleration += 0.04f;
                }
                else
                {
                    if (players[0].acceleration > 0) players[0].acceleration -= 0.02f;
                    else if (players[0].acceleration < 0) players[0].acceleration = 0;
                }
            }
            if (current_direction_1 == 90) {
                if (IsKeyDown(KEY_RIGHT))
                {
                    if (players[0].acceleration < 1) players[0].acceleration += 0.04f;
                }
                else
                {
                    if (players[0].acceleration > 0) players[0].acceleration -= 0.02f;
                    else if (players[0].acceleration < 0) players[0].acceleration = 0;
                }
            }
            
            if (current_direction_2 == 0) {
                if (IsKeyDown(KEY_W))
                {
                    if (players[1].acceleration < 1) players[1].acceleration += 0.04f;
                }
                else
                {
                    if (players[1].acceleration > 0) players[1].acceleration -= 0.02f;
                    else if (players[1].acceleration < 0) players[1].acceleration = 0;
                }
            }
            if (current_direction_2 == 180) {
                if (IsKeyDown(KEY_S))
                {
                    if (players[1].acceleration < 1) players[1].acceleration += 0.04f;
                }
                else
                {
                    if (players[1].acceleration > 0) players[1].acceleration -= 0.02f;
                    else if (players[1].acceleration < 0) players[1].acceleration = 0;
                }
            }
            if (current_direction_2 == -90) {
                if (IsKeyDown(KEY_A))
                {
                    if (players[1].acceleration < 1) players[1].acceleration += 0.04f;
                }
                else
                {
                    if (players[1].acceleration > 0) players[1].acceleration -= 0.02f;
                    else if (players[1].acceleration < 0) players[1].acceleration = 0;
                }
            }
            if (current_direction_2 == 90) {
                if (IsKeyDown(KEY_D))
                {
                    if (players[1].acceleration < 1) players[1].acceleration += 0.04f;
                }
                else
                {
                    if (players[1].acceleration > 0) players[1].acceleration -= 0.02f;
                    else if (players[1].acceleration < 0) players[1].acceleration = 0;
                }
            }
    
            // Bullet Emission
            if (IsKeyPressed(KEY_ENTER)) {
                Bullet newBullet = Bullet();
                newBullet.active = true;
                newBullet.color = MAROON;
                newBullet.position = players[0].position;
                newBullet.radius = 5;
                int velx = GetRandomValue(-METEORS_SPEED, METEORS_SPEED);
                int vely = GetRandomValue(-METEORS_SPEED, METEORS_SPEED);
                newBullet.speed = (Vector2){static_cast<float>(velx), static_cast<float>(vely)};
                bullets.push_back(newBullet);
            }
            
            if (IsKeyPressed(KEY_SPACE)) {
                Bullet newBullet = Bullet();
                newBullet.active = true;
                newBullet.color = DARKBLUE;
                newBullet.position = players[1].position;
                newBullet.radius = 5;
                int velx = GetRandomValue(0, METEORS_SPEED);
                int vely = GetRandomValue(0, METEORS_SPEED);
                newBullet.speed = (Vector2){static_cast<float>(velx), static_cast<float>(vely)};
                bullets.push_back(newBullet);
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
            for (int i = 0; i < 2; i++) {
                players[i].collider = (Vector3){players[i].position.x + sin(players[i].rotation*DEG2RAD)*(shipHeight/2.5f), players[i].position.y - cos(players[i].rotation*DEG2RAD)*(shipHeight/2.5f), 12};
                vector<int> erasedMediumMeteorId;
                for (int a = 0; a < mediumMeteor.size(); ++a)
                {
                    if (CheckCollisionCircles((Vector2){players[i].collider.x, players[i].collider.y}, players[i].collider.z, mediumMeteor[a].position, mediumMeteor[a].radius) && mediumMeteor[a].active)
                     {
                         players[i].hp -= 10;
                         erasedMediumMeteorId.push_back(a);
                         if(players[i].hp <=0)gameOver = true;
                     
                     }
                }
                for (int j = (int)erasedMediumMeteorId.size() - 1; j >= 0; j--){
                    mediumMeteor.erase(mediumMeteor.begin() + erasedMediumMeteorId[j]);
                }
                
                vector<int> erasedSmallMeteorId;
                for (int a=0 ;a < smallMeteor.size(); ++a)
                {
                    if (CheckCollisionCircles((Vector2){players[i].collider.x, players[i].collider.y}, players[i].collider.z, smallMeteor[a].position, smallMeteor[a].radius) && smallMeteor[a].active)
                      {
                          players[i].hp -= 5;
                          erasedSmallMeteorId.push_back(a);
                          if(players[i].hp <=0)gameOver = true;
                     
                     }
                }
                for (int j = (int)erasedSmallMeteorId.size() - 1; j >= 0; j--){
                    smallMeteor.erase(smallMeteor.begin() + erasedSmallMeteorId[j]);
                }
            }
            
            
            // Collision Bullet to meteors

            // Meteor logic
            
            vector<int> erasedMediumMeteorId;
            for (int i=0; i< mediumMeteor.size(); i++)
            {
                if (mediumMeteor[i].active)
                {
                    // movement
                    mediumMeteor[i].position.x += mediumMeteor[i].speed.x;
                    mediumMeteor[i].position.y += mediumMeteor[i].speed.y;

                    // wall behaviour
                    if  (mediumMeteor[i].position.x > screenWidth + mediumMeteor[i].radius)
                        erasedMediumMeteorId.push_back(i);
                    else if (mediumMeteor[i].position.x < 0 - mediumMeteor[i].radius)
                        erasedMediumMeteorId.push_back(i);
                    else if (mediumMeteor[i].position.y > screenHeight + mediumMeteor[i].radius)
                        erasedMediumMeteorId.push_back(i);
                    else if (mediumMeteor[i].position.y < 0 - mediumMeteor[i].radius)
                        erasedMediumMeteorId.push_back(i);
                }
            }
            for (int i = (int)erasedMediumMeteorId.size() - 1; i >= 0; i--) {
                mediumMeteor.erase(mediumMeteor.begin() + erasedMediumMeteorId[i]);
            }
            
            vector<int> erasedSmallMeteorId;
            for (int i=0; i< smallMeteor.size(); ++i)
            {
                if (smallMeteor[i].active)
                {
                    // movement
                    smallMeteor[i].position.x += smallMeteor[i].speed.x;
                    smallMeteor[i].position.y += smallMeteor[i].speed.y;

                    // wall behaviour
                    if  (smallMeteor[i].position.x > screenWidth + smallMeteor[i].radius)
                        erasedSmallMeteorId.push_back(i);
                    else if (smallMeteor[i].position.x < 0 - smallMeteor[i].radius)
                        erasedSmallMeteorId.push_back(i);
                    else if (smallMeteor[i].position.y > screenHeight + smallMeteor[i].radius)
                        erasedSmallMeteorId.push_back(i);
                    else if (smallMeteor[i].position.y < 0 - smallMeteor[i].radius)
                        erasedSmallMeteorId.push_back(i);
                }
            }
            for (int i = (int)erasedSmallMeteorId.size() - 1; i >= 0; i--) {
                smallMeteor.erase(smallMeteor.begin() + erasedSmallMeteorId[i]);
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
            // Draw boss
            int bossNum = boss.size();
            for (int i = 0; i < bossNum; i++) {
                Vector2 v1 = { boss[i].position.x + sinf(boss[i].rotation*DEG2RAD)*(shipHeight), boss[i].position.y - cosf(boss[i].rotation*DEG2RAD)*(shipHeight) };
                Vector2 v2 = { boss[i].position.x - cosf(boss[i].rotation*DEG2RAD)*(BOSS_BASE_SIZE/2), boss[i].position.y - sinf(boss[i].rotation*DEG2RAD)*(PLAYER_BASE_SIZE/2) };
                Vector2 v3 = { boss[i].position.x + cosf(boss[i].rotation*DEG2RAD)*(BOSS_BASE_SIZE/2), boss[i].position.y + sinf(boss[i].rotation*DEG2RAD)*(PLAYER_BASE_SIZE/2) };
                DrawTriangle(v1, v2, v3, MAROON);
                DrawRectangle(boss[i].position.x-20, boss[i].position.y-20, boss[i].hp*3, 3, boss[i].color);
            }

            // Draw spaceship
            for (int i = 0; i < 2; i++) {
                Vector2 v1 = { players[i].position.x + sinf(players[i].rotation*DEG2RAD)*(shipHeight), players[i].position.y - cosf(players[i].rotation*DEG2RAD)*(shipHeight) };
                Vector2 v2 = { players[i].position.x - cosf(players[i].rotation*DEG2RAD)*(PLAYER_BASE_SIZE/2), players[i].position.y - sinf(players[i].rotation*DEG2RAD)*(PLAYER_BASE_SIZE/2) };
                Vector2 v3 = { players[i].position.x + cosf(players[i].rotation*DEG2RAD)*(PLAYER_BASE_SIZE/2), players[i].position.y + sinf(players[i].rotation*DEG2RAD)*(PLAYER_BASE_SIZE/2) };
                DrawTriangle(v1, v2, v3, players[i].color);
                DrawRectangle(players[i].position.x-20, players[i].position.y-20,players[i].hp*3, 3, players[i].color);
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
            
            // Draw bullet
            for (int i = 0;i< bullets.size(); i++)
            {
                if (bullets[i].active) DrawCircleV(bullets[i].position, bullets[i].radius, bullets[i].color);
                else DrawCircleV(mediumMeteor[i].position, mediumMeteor[i].radius, Fade(bullets[i].color, 0.3f));
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
