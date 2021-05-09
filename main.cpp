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
#include<algorithm>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <random>
#include <unordered_set>
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
#define MAX_METEORS  40

#define BULLET_SPEED    2

#define BOSS_BASE_SIZE      50.0f
#define BOSS_SPEED          3.0f
#define BOSS_MAX_HP         200

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
    int damage;
    Color color;
} Bullet;


//------define by yun
Rectangle frameRec1;
Rectangle frameRec2;
Rectangle frameRec_boss;
int frame_count = 0;
int framesSpeed = 8;
int currentFrame = 0;
float frame_w ;
float frame_h;
float frame_boss_w;
float frame_boss_h;



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
static vector<Boss>   bosses(1);
static vector<Meteor> meteors;
static vector<Bullet> bullets;

//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
static void InitGame(void);         // Initialize game
static void UpdateGame(void);       // Update game (one frame)
static void DrawGame(Texture2D player_model,Texture2D boss_move_model);         // Draw game (one frame)
static void UnloadGame(void);       // Unload game
static void UpdateDrawFrame(Texture2D player_model,Texture2D boss_move_model);  // Update and Draw (one frame)

int getRotationDirection(int rotation) {
    if (rotation >= -30 && rotation <= 30) return 0;   // UP
    else if (rotation > 30 && rotation < 150) return 3; // RIGHT
    else if ((rotation >= 150 && rotation <= 180) || (rotation <= -150 && rotation >= -179)) return 2; // DOWN
    else return 1;  // LEFT
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization (Note windowTitle is unused on Android)
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "sample game: asteroids survival");
    //-----------------------------------------------
    //Texture
    //---------------------------------------------


    Texture2D player_model = LoadTexture("./texture/player.png");
    Texture2D boss_move_model = LoadTexture("./texture/boss/golem-walk.png");
    Texture2D boss_atk_model = LoadTexture("./texture/boss/golem-atk.png");
    frameRec1 = { 0.0f, 0.0f, (float)player_model.width/4, (float)player_model.height/4 };
    frameRec2 = { 0.0f, 0.0f, (float)player_model.width/4, (float)player_model.height/4 };
    frameRec_boss = { 0.0f, 0.0f, (float)boss_move_model.width/7, (float)boss_move_model.height/4 };
    frame_w = (float)player_model.width/4;
    frame_h = (float)player_model.height/4;
    frame_boss_w = (float)boss_move_model.width/7;
    frame_boss_h = (float)boss_move_model.height/4;



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
        UpdateDrawFrame(player_model,boss_move_model);
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
    srand((unsigned int)time(NULL));
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
    for (int i = 0; i < bosses.size(); i++ ) {
        bosses[i].position = (Vector2){screenWidth/3, screenHeight/3 - shipHeight/4};
        bosses[i].speed = (Vector2){0, 0};
        bosses[i].acceleration = 0.1f;
        bosses[i].rotation = 0;
        bosses[i].collider = (Vector3){bosses[i].position.x + sin(bosses[i].rotation*DEG2RAD)*(shipHeight/2.5f), bosses[i].position.y - cos(bosses[i].rotation*DEG2RAD)*(shipHeight/2.5f), 12};
        bosses[i].hp = BOSS_MAX_HP;
    }
    bosses[0].color = DARKBLUE;

    // Initialising meteors
    default_random_engine randEng;
    bernoulli_distribution bernoulliDistri;
    for (int i = 0; i < MAX_METEORS; i++)
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
        meteors.push_back(Meteor());
        meteors.back().position = (Vector2){static_cast<float>(posx), static_cast<float>(posy)};
        meteors.back().speed = (Vector2){static_cast<float>(velx), static_cast<float>(vely)};
        meteors.back().active = true;
        
        if (bernoulliDistri(randEng)) {
            meteors.back().radius = 20;
            meteors.back().color = GREEN;
        }
        else {
            meteors.back().radius = 10;
            meteors.back().color = YELLOW;
        }
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
            int bossNum = (int) bosses.size();

            // Rotation
            if (framesCounter % 300 == 0) {
                for (int i = 0; i < bossNum; i++) {
                    bosses[i].rotation = rand() % 360 + 1 - 180;
                    int dir = getRotationDirection(bosses[i].rotation);
                    frameRec_boss.y = dir*frame_boss_h;
                }
            }
            
            // Speed
            for (int i = 0; i < bossNum; i++) {
                bosses[i].speed.x = sin(bosses[i].rotation*DEG2RAD)*BOSS_SPEED;
                bosses[i].speed.y = cos(bosses[i].rotation*DEG2RAD)*BOSS_SPEED;
            }

            // Movement
            for (int i = 0; i < bossNum; i++) {
                bosses[i].position.x += (bosses[i].speed.x*bosses[i].acceleration);
                bosses[i].position.y -= (bosses[i].speed.y*bosses[i].acceleration);
            }

            // Wall behavior for boss
            for (int i = 0; i < bossNum; i++) {
                if (bosses[i].position.x > screenWidth ) bosses[i].position.x = screenWidth;
                else if (bosses[i].position.x < -(shipHeight)) bosses[i].position.x = 0;
                if (bosses[i].position.y > (screenHeight )) bosses[i].position.y = screenHeight;
                else if (bosses[i].position.y < -(shipHeight)) bosses[i].position.y = 0;
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
                    frameRec1.y = 3*frame_h;//edit by yun
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
                    frameRec1.y = 0*frame_h;//edit by yun
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
                    frameRec1.y = 1*frame_h;//edit by yun
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
                    frameRec1.y = 2*frame_h;//edit by yun
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
                    frameRec2.y = 3*frame_h;//edit by yun
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
                    frameRec2.y = 0*frame_h;//edit by yun
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
                    frameRec2.y = 1*frame_h;//edit by yun
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
                    frameRec2.y = 2*frame_h;//edit by yun
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
                newBullet.damage = 10;
                newBullet.speed = (Vector2){sin((players[0].rotation + 180)*DEG2RAD)*BULLET_SPEED, cos((players[0].rotation + 180)*DEG2RAD)*BULLET_SPEED};
                bullets.push_back(newBullet);
            }
            
            if (IsKeyPressed(KEY_SPACE)) {
                Bullet newBullet = Bullet();
                newBullet.active = true;
                newBullet.color = DARKBLUE;
                newBullet.position = players[1].position;
                newBullet.radius = 5;
                newBullet.damage = 10;
                newBullet.speed = (Vector2){sin((players[1].rotation + 180)*DEG2RAD)*BULLET_SPEED, cos((players[1].rotation + 180)*DEG2RAD)*BULLET_SPEED};
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
            
            vector<int> erasedMeteorId;
            unordered_set<int> erasedMeteorIdSet;
            vector<int> erasedBulletId;
            vector<int> erasedBossId;
            unordered_set<int> erasedBossIdSet;
            
            
            // Meteor logic
            erasedMeteorId.clear();
            erasedMeteorIdSet.clear();
            for (int i=0; i< meteors.size(); i++)
            {
                if (meteors[i].active)
                {
                    // movement
                    meteors[i].position.x += meteors[i].speed.x;
                    meteors[i].position.y += meteors[i].speed.y;

                    // wall behaviour
                    if  (meteors[i].position.x > screenWidth + meteors[i].radius)
                        erasedMeteorId.push_back(i);
                    else if (meteors[i].position.x < 0 - meteors[i].radius)
                        erasedMeteorId.push_back(i);
                    else if (meteors[i].position.y > screenHeight + meteors[i].radius)
                        erasedMeteorId.push_back(i);
                    else if (meteors[i].position.y < 0 - meteors[i].radius)
                        erasedMeteorId.push_back(i);
                }
            }
            for (int i = (int)erasedMeteorId.size() - 1; i >= 0; i--) {
                meteors.erase(meteors.begin() + erasedMeteorId[i]);
            }
            
            
            // Bullet logic
            erasedBulletId.clear();
            for (int i=0; i< bullets.size(); i++)
            {
                if (bullets[i].active)
                {
                    // movement
                    bullets[i].position.x += bullets[i].speed.x;
                    bullets[i].position.y += bullets[i].speed.y;

                    // wall behaviour
                    if  (bullets[i].position.x > screenWidth + bullets[i].radius)
                        erasedBulletId.push_back(i);
                    else if (bullets[i].position.x < 0 - bullets[i].radius)
                        erasedBulletId.push_back(i);
                    else if (bullets[i].position.y > screenHeight +  bullets[i].radius)
                        erasedBulletId.push_back(i);
                    else if (bullets[i].position.y < 0 - bullets[i].radius)
                        erasedBulletId.push_back(i);
                }
            }
            for (int i = (int)erasedBulletId.size() - 1; i >= 0; i--) {
                bullets.erase(bullets.begin() + erasedBulletId[i]);
            }
            
            // Collision Logic
            // Collision Player to meteors
            for (int i = 0; i < 2; i++) {
                players[i].collider = (Vector3){players[i].position.x + sin(players[i].rotation*DEG2RAD)*(shipHeight/2.5f), players[i].position.y - cos(players[i].rotation*DEG2RAD)*(shipHeight/2.5f), 12};
                erasedMeteorId.clear();
                for (int a = 0; a < meteors.size(); ++a)
                {
                    if (CheckCollisionCircles((Vector2){players[i].collider.x, players[i].collider.y}, players[i].collider.z, meteors[a].position, meteors[a].radius) && meteors[a].active)
                     {
                         players[i].hp -= 10;
                         erasedMeteorId.push_back(a);
                         if(players[i].hp <=0)gameOver = true;
                     }
                }
                for (int j = (int)erasedMeteorId.size() - 1; j >= 0; j--){
                    meteors.erase(meteors.begin() + erasedMeteorId[j]);
                }
            }
            
            // Collision Bullet to meteors
            erasedMeteorId.clear();
            erasedMeteorIdSet.clear();
            erasedBulletId.clear();
            for (int b_id = 0; b_id < bullets.size(); b_id++) {
                for (int m_id = 0; m_id < meteors.size(); m_id++) {
                    if (erasedMeteorIdSet.find(m_id) != erasedMeteorIdSet.end())
                        continue;
                    if (CheckCollisionCircles(bullets[b_id].position, bullets[b_id].radius, meteors[m_id].position, meteors[m_id].radius) && bullets[b_id].active && meteors[m_id].active) {
                        erasedMeteorId.push_back(m_id);
                        erasedMeteorIdSet.insert(m_id);
                        erasedBulletId.push_back(b_id);
                        break;
                    }
                }
            }
            sort(erasedBulletId.begin(), erasedBulletId.end());
            sort(erasedMeteorId.begin(), erasedMeteorId.end());
            for (int i = (int)erasedMeteorId.size() - 1; i >= 0; i--) {
                meteors.erase(meteors.begin() + erasedMeteorId[i]);
            }
            for (int i = (int)erasedBulletId.size() - 1; i >= 0; i--) {
                bullets.erase(bullets.begin() + erasedBulletId[i]);
            }
            
            // Collision Bullet to boss
            erasedBulletId.clear();
            erasedBossId.clear();
            erasedBossIdSet.clear();
            for (int i = 0; i < bosses.size(); i++) {
                bosses[i].collider = (Vector3){bosses[i].position.x, bosses[i].position.y, 20};
            }
            for (int bulletId = 0; bulletId < bullets.size(); bulletId++) {
                for (int bossId = 0; bossId < bosses.size(); bossId++) {
                    if (bosses[bossId].hp <= 0) continue;
                    if (CheckCollisionCircles((Vector2){bosses[bossId].collider.x, bosses[bossId].collider.y}, bosses[bossId].collider.z, bullets[bulletId].position, bullets[bulletId].radius) && bullets[bulletId].active)
                     {
                         bosses[bossId].hp -= bullets[bulletId].damage;
                         erasedBulletId.push_back(bulletId);
                         if (bosses[bossId].hp <= 0) {
                             erasedBossId.push_back(bossId);
                         }
                         break;
                     }
                }
            }
            sort(erasedBulletId.begin(), erasedBulletId.end());
            sort(erasedBossId.begin(), erasedBossId.end());
            for (int i = (int)erasedBulletId.size() - 1; i >= 0; i--) {
                bullets.erase(bullets.begin() + erasedBulletId[i]);
            }
            for (int i = (int)erasedBossId.size() - 1; i >= 0; i--) {
                bosses.erase(bosses.begin() + erasedBossId[i]);
            }
            if (bosses.size() == 0) {
                gameOver = true;
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
void DrawGame(Texture2D player_model, Texture2D boss_move_model)
{
    BeginDrawing();

        ClearBackground(RAYWHITE);
        
        if (!gameOver)
        {
            //----------------------------------------------------------------------------------draw by yun
            
            frame_count++;

            if (frame_count>= (60/framesSpeed))
            {
                frame_count = 0;
                currentFrame++;

                if (currentFrame > 3) currentFrame = 0;

                frameRec1.x = (float)currentFrame*frame_w;
                frameRec2.x = (float)currentFrame*frame_w;
                frameRec_boss.x = (float)currentFrame*frame_boss_w;
            }

            // Draw boss
            int bossNum = (int) bosses.size();
            for (int i = 0; i < bossNum; i++) {
                Vector2 v1 = { bosses[i].position.x + sinf(bosses[i].rotation*DEG2RAD)*(shipHeight), bosses[i].position.y - cosf(bosses[i].rotation*DEG2RAD)*(shipHeight) };
                Vector2 v2 = { bosses[i].position.x - cosf(bosses[i].rotation*DEG2RAD)*(BOSS_BASE_SIZE/2), bosses[i].position.y - sinf(bosses[i].rotation*DEG2RAD)*(PLAYER_BASE_SIZE/2) };
                Vector2 v3 = { bosses[i].position.x + cosf(bosses[i].rotation*DEG2RAD)*(BOSS_BASE_SIZE/2), bosses[i].position.y + sinf(bosses[i].rotation*DEG2RAD)*(PLAYER_BASE_SIZE/2) };
                //DrawTriangle(v1, v2, v3, bosses[i].color);
                DrawTextureRec(boss_move_model, frameRec_boss, bosses[0].position, WHITE);  // Draw part of the texture ,edit by yun
                DrawRectangle(bosses[i].position.x-20, bosses[i].position.y-20, bosses[i].hp*3, 3, bosses[i].color);
            }

            

            // Draw spaceship
            for (int i = 0; i < 2; i++) {
                Vector2 v1 = { players[i].position.x + sinf(players[i].rotation*DEG2RAD)*(shipHeight), players[i].position.y - cosf(players[i].rotation*DEG2RAD)*(shipHeight) };
                Vector2 v2 = { players[i].position.x - cosf(players[i].rotation*DEG2RAD)*(PLAYER_BASE_SIZE/2), players[i].position.y - sinf(players[i].rotation*DEG2RAD)*(PLAYER_BASE_SIZE/2) };
                Vector2 v3 = { players[i].position.x + cosf(players[i].rotation*DEG2RAD)*(PLAYER_BASE_SIZE/2), players[i].position.y + sinf(players[i].rotation*DEG2RAD)*(PLAYER_BASE_SIZE/2) };
                //DrawTriangle(v1, v2, v3, players[i].color);
                if(i==0)DrawTextureRec(player_model, frameRec1, players[i].position, WHITE);  // Draw part of the texture ,edit by yun
                if(i==1)DrawTextureRec(player_model, frameRec2, players[i].position, WHITE);  // Draw part of the texture
                DrawRectangle(players[i].position.x-20, players[i].position.y-20,players[i].hp*3, 3, players[i].color);
            }

            // Draw meteor
            for (int i = 0;i< meteors.size(); i++)
            {
                if (meteors[i].radius == 20) {
                    if (meteors[i].active) DrawCircleV(meteors[i].position, meteors[i].radius, GRAY);
                    else DrawCircleV(meteors[i].position, meteors[i].radius, Fade(LIGHTGRAY, 0.3f));
                }
                else {
                    if (meteors[i].active) DrawCircleV(meteors[i].position, meteors[i].radius, DARKGRAY);
                    else DrawCircleV(meteors[i].position, meteors[i].radius, Fade(LIGHTGRAY, 0.3f));
                }
                
            }

            
            // Draw bullet
            for (int i = 0;i< bullets.size(); i++)
            {
                if (bullets[i].active) DrawCircleV(bullets[i].position, bullets[i].radius, bullets[i].color);
                else DrawCircleV(bullets[i].position, bullets[i].radius, Fade(bullets[i].color, 0.3f));
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
void UpdateDrawFrame(Texture2D player_model, Texture2D boss_move_model)
{
    UpdateGame();
    DrawGame(player_model,boss_move_model);
}
