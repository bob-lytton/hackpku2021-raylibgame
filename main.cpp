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
#include <unordered_map>
using namespace std;

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Some Defines
//----------------------------------------------------------------------------------
#define PLAYER_BASE_SIZE    20.0f
#define PLAYER_SPEED        3.0f
#define PLAYER_MAX_SHOOTS   10
#define PLAYER_MAX_HP       50

#define MAX_ENV_METEORS     0
#define METEORS_SPEED       2.0f

#define PLAYER_BULLET_SPEED 5.0f
#define BOSS_BULLET_SPEED   3.0f

#define BOSS_BASE_SIZE      50.0f
#define BOSS_SPEED          0.5f
#define BOSS_MAX_HP         250

#define DIR_UP              0
#define DIR_LEFT            1
#define DIR_DOWN            2
#define DIR_RIGHT           3

//------define by yun
vector<Rectangle> frameRec;
Rectangle frameRec_boss;
Rectangle frameRec_bossatk;
int frame_count = 0;
int framesSpeed = 6;
int currentFrame = 0;
int currentFrame_boss = 0;
float frame_w ;
float frame_h;
float frame_boss_w;
float frame_boss_h;
float frame_bossatk_w;
float frame_bossatk_h;

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
static const int screenWidth = 800;
static const int screenHeight = 800;

static int framesCounter = 0;
static bool gameOver = false;
static bool pause = false;

// NOTE: Defined triangle is isosceles with common angles of 70 degrees.
static float shipHeight = 0.0f;

//------------------------------------------------------------------------------------
// Help Functions Declaration
//------------------------------------------------------------------------------------
float getDistance(float x1, float y1, float x2, float y2);
int getRotationDirection(int rotation);

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
class Player {
public:
    int id;
    Vector2 position;
    Vector2 speed;
    float acceleration;
    float rotation;
    Rectangle collider;
    Color color;
    float hp;
    int curDirection;
    vector<int> dirFrame;
    unordered_map<int, int> keyMap;

    void init(int keyMapSchema, float x, float y) {
        initKeyMap(keyMapSchema);
        id = keyMapSchema;
        dirFrame = vector<int>{3, 1, 0, 2};

        position = (Vector2){x, y};
        speed = (Vector2){0, 0};
        acceleration = 0;
        rotation = 0;
        collider = (Rectangle){position.x-12, position.y-21, 24, 42};
        hp = PLAYER_MAX_HP;
    }

    void updateRotation() {
        if (IsKeyDown(keyMap[DIR_UP])) { rotation = 0; curDirection = DIR_UP; }
        if (IsKeyDown(keyMap[DIR_DOWN])) { rotation = 180; curDirection = DIR_DOWN; }
        if (IsKeyDown(keyMap[DIR_LEFT])) { rotation = -90; curDirection = DIR_LEFT; }
        if (IsKeyDown(keyMap[DIR_RIGHT])) { rotation = 90; curDirection = DIR_RIGHT; }
    }

    void updateSpeed() {
        speed.x = sin(rotation * DEG2RAD) * PLAYER_SPEED;
        speed.y = cos(rotation * DEG2RAD) * PLAYER_SPEED;
    }

    void walkCtrl(int dir) {
        if (curDirection == dir) {
            if (IsKeyDown(keyMap[dir])) {
                if (acceleration < 1)
                    acceleration = min(acceleration + 0.04f, 1.0f);
                frameRec[id].y = dirFrame[dir] * frame_h;//edit by yun
            }
            else {
                acceleration = max(0.0f, acceleration - 0.02f);
            }
        }
    }

    void updatePosition() {
        position.x += speed.x * acceleration;
        position.y -= speed.y * acceleration;
    }

    void updateColliderPosition() {
        collider.x = position.x - 12;
        collider.y = position.y - 25;
    }

    void printSpeed() {
        printf("player id: %d, speed: (%f, %f), acceleration: %f\n", id, speed.x, speed.y, acceleration);
    }

private:
    void initKeyMap(int schema) {
        if (schema == 0) {
            // up, down, left, right
            keyMap[DIR_UP] = KEY_UP;
            keyMap[DIR_DOWN] = KEY_DOWN;
            keyMap[DIR_LEFT] = KEY_LEFT;
            keyMap[DIR_RIGHT] = KEY_RIGHT;
        } else if (schema == 1) {
            // w, a, s, d
            keyMap[DIR_UP] = KEY_W;
            keyMap[DIR_DOWN] = KEY_S;
            keyMap[DIR_LEFT] = KEY_A;
            keyMap[DIR_RIGHT] = KEY_D;
        }
    }

};


class Boss {
public:
    Vector2 position;
    Vector2 speed;
    float acceleration;
    float rotation;
    Rectangle collider;
    Color color;
    float hp;
    bool inAttack;

    void init() {
        position = (Vector2){screenWidth / 2, screenHeight / 3.5};
        speed = (Vector2){0, 0};
        acceleration = 1.0f;
        rotation = 180;
        collider = (Rectangle){position.x - 24, position.y - 38, 48, 76};
        hp = BOSS_MAX_HP;
    }

    void updateRotation(float playerPosx, float playerPosy) {
        // if going out of the map
        if (!insideBorder()) {
            rotation += 180;    // reverse direction
            rotation += rand() % 21 - 10;   // add a small turbulence
        } else {
            if (getDistance(position.x, position.y, playerPosx, playerPosy) < 15.0) {
                rotation = rand() % 360;
            }
            else {
                // go straight to the player
                float dx = playerPosx - position.x;
                float dy = playerPosy - position.y;
                // printf("player pos: (%f, %f), boss pos: (%f, %f), dist: (%f, %f)\n", playerPosx, playerPosy, position.x, position.y, dx, dy); TODO: debug
                rotation = atan2(dx, -dy) * RAD2DEG;
                printRotation();
            }
        }
        // printRotation();    // TODO: debug
    }

    void updateSpeed() {
        speed.x = sin(rotation * DEG2RAD) * BOSS_SPEED;
        speed.y = cos(rotation * DEG2RAD) * BOSS_SPEED;
    }

    void updatePosition() {
        position.x += speed.x * acceleration;
        position.y -= speed.y * acceleration;
    }

    void updateColliderPosition() {
        collider.x = position.x - 24;
        collider.y = position.y - 38;
    }

private:
    bool insideBorder() {
        float x = position.x;
        float y = position.y;
        return x > 0 && x < screenWidth && y > 0 && y < screenHeight;
    }

    void printRotation() {
        printf("boss rotation: %f\n", rotation);
    }
};

// Meteors are emited by boss
class Meteor {
public:
    Vector2 position;
    Vector2 speed;
    float radius;
    bool active;
    Color color;

    Meteor() {}
    Meteor(int posx, int posy, int velx, int vely) {
        position = (Vector2){static_cast<float>(posx), static_cast<float>(posy)};
        speed = (Vector2){static_cast<float>(velx), static_cast<float>(vely)};
        active = true;
    }

};

// Bullet are emited by player or boss
class Bullet {
public:
    Vector2 position;
    Vector2 speed;
    float radius;
    bool active;
    int damage;
    Color color;
};

static vector<Player> players(2);
static vector<Boss>   bosses(1);
static vector<Meteor> meteors;
static vector<Bullet> playerBullets;
static vector<Bullet> bossBullets;

//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
static void InitGame(void);         // Initialize game
static void UpdateGame(Sound playerwav,Sound bosswav);       // Update game (one frame)
static void DrawGame(Texture2D player_model,Texture2D boss_move_model, Texture2D boss_atk_model,Texture2D bgTexture);         // Draw game (one frame)
static void UnloadGame(void);       // Unload game
static void UpdateDrawFrame(Texture2D player_model,Texture2D boss_move_model, Texture2D boss_atk_model,Texture2D bgTexture,Sound playerwav,Sound bosswav);  // Update and Draw (one frame)

//------------------------------------------------------------------------------------
// Help Functions
//------------------------------------------------------------------------------------
float getDistance(float x1, float y1, float x2, float y2) {
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}

int getRotationDirection(int rotation) {
    if (rotation >= -30 && rotation <= 30) return DIR_UP;   // UP
    else if (rotation > 30 && rotation < 150) return DIR_RIGHT; // RIGHT
    else if ((rotation >= 150 && rotation <= 180) || (rotation <= -150 && rotation >= -179)) return DIR_DOWN; // DOWN
    else return DIR_LEFT;  // LEFT
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization (Note windowTitle is unused on Android)
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "Beat the boss!");

    //-----------------------------------------------
    //Texture
    //---------------------------------------------
    Texture2D player_model = LoadTexture("./texture/player.png");
    Texture2D boss_move_model = LoadTexture("./texture/boss/golem-walk.png");
    Texture2D boss_atk_model = LoadTexture("./texture/boss/golem-atk.png");
    Image bgImage = LoadImage("texture/TileableWall.png");     // Loaded in CPU memory (RAM)
    Texture2D bgTexture = LoadTextureFromImage(bgImage);  
    InitAudioDevice();      // Initialize audio device

    Sound playerwav = LoadSound("texture/radio/player.wav");
    Sound bosswav = LoadSound("texture/radio/boss.wav");
    UnloadImage(bgImage);

    for (int i = 0; i < 2; i++) {
        frameRec.push_back({ 0.0f, 0.0f, (float)player_model.width/4, (float)player_model.height/4 });
    }
    frameRec_boss = { 0.0f, 0.0f, (float)boss_move_model.width/7, (float)boss_move_model.height/4 };
    frameRec_bossatk = { 0.0f, 0.0f, (float)boss_atk_model.width/7, (float)boss_atk_model.height/4 };
    frame_w = (float)player_model.width/4;
    frame_h = (float)player_model.height/4;
    frame_boss_w = (float)boss_move_model.width/7;
    frame_boss_h = (float)boss_move_model.height/4;
    frame_bossatk_w = (float)boss_atk_model.width/7;
    frame_bossatk_h = (float)boss_atk_model.height/4;

    InitGame();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update and Draw
        UpdateDrawFrame(player_model,boss_move_model,boss_atk_model,bgTexture,playerwav,bosswav);
    }
#endif
    // De-Initialization
    UnloadGame();         // Unload loaded data (textures, sounds, models...)
    UnloadTexture(bgTexture);
    UnloadSound(playerwav);     // Unload sound data
    UnloadSound(bosswav);     // Unload sound data

    CloseAudioDevice();
    CloseWindow();        // Close window and OpenGL context
    
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
    players[0].init(0, (int)(screenWidth * 0.75), (int)(screenHeight * 0.75));
    players[0].color = RED;
    players[1].init(1, (int)(screenWidth * 0.25), (int)(screenHeight * 0.75));
    players[1].color = BLUE;

    // Initialising boss
    bosses.clear();
    bosses.push_back(Boss());
    for (int i = 0; i < bosses.size(); i++ ) {
        bosses[i].init();
    }
    bosses[0].color = DARKBLUE;

    // Initialising meteors
    default_random_engine randEng;
    bernoulli_distribution bernoulliDistri;
    for (int i = 0; i < MAX_ENV_METEORS; i++)
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
        meteors.push_back(Meteor(posx, posy, velx, vely));
        
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
void UpdateGame(Sound playerwav,Sound bosswav)
{
    if (!gameOver)
    {
        if (IsKeyPressed('P')) pause = !pause;

        if (!pause)
        {
            framesCounter++;

            // #########  Boss logic begin #########
            
            // number
            int playerNum = (int) players.size();
            int bossNum = (int) bosses.size();

            // TODO: boss movement logic
            // Rotation
            if (framesCounter % 300 == 0) {
                for (int i = 0; i < bossNum; i++) {
                    int p = rand() % 2; // player target
                    bosses[i].updateRotation(players[p].position.x, players[p].position.y);
                    int dir = getRotationDirection(bosses[i].rotation);
                    frameRec_bossatk.y = dir*frame_bossatk_h;
                    frameRec_boss.y = dir*frame_boss_h;
                    printf("boss attack frame %f, %f\n", frame_bossatk_h, frame_boss_h);
                }
            }
            
            // Speed
            for (int i = 0; i < bossNum; i++) {
                bosses[i].updateSpeed();
            }

            // Movement
            for (int i = 0; i < bossNum; i++) {
                bosses[i].updatePosition();
            }

            // Wall behavior for boss
            for (int i = 0; i < bossNum; i++) {
                if (bosses[i].position.x > screenWidth)
                    bosses[i].position.x = screenWidth;
                else if (bosses[i].position.x < 0)
                    bosses[i].position.x = 0;
                if (bosses[i].position.y > screenHeight)
                    bosses[i].position.y = screenHeight;
                else if (bosses[i].position.y < 0)
                    bosses[i].position.y = 0;
            }
            
            // boss emit meteor
            if(framesCounter % 300 >= 0 && framesCounter % 300 < 70){ //70 out of every 300 frames are attack frames ,edit by yun
                default_random_engine randEng;
                bernoulli_distribution bernoulliDistri;
                for (int b = 0; b < bosses.size(); b++) {
                    if (framesCounter % 50 == 0) {
                        int target = 0;
                        if (framesCounter % 100 == 0) {
                            target = 0;
                        }
                        else {
                            target = 1;
                        }
                        if (players[target].hp <= 0) target = 1 - target;
                        // velocity direction
                        players[target].printSpeed();
                        
                        float velx = (players[target].position.x - bosses[b].position.x);
                        float vely = (players[target].position.y - bosses[b].position.y);
                        
                        // the larger the distance, the faster the speed
                        float s = sqrt(pow(velx, 2) + pow(vely, 2));
                        velx = velx / s * METEORS_SPEED;
                        vely = vely / s * METEORS_SPEED;
                        // edit by yun, add the second attack model
                        PlaySound(bosswav); 
                        if(bosses[b].hp <BOSS_MAX_HP/3){
                            

                            for(float tx = -4; tx <= 4; tx += 1){
                                
                                float ty = sqrt(16 - pow(tx,2));
                                //printf("tx:%f , ty:%f\n", tx,ty);
                                meteors.push_back(Meteor(bosses[b].position.x, bosses[b].position.y, tx, ty));
                                meteors.back().radius = 10;
                                meteors.back().color = YELLOW;
                                meteors.push_back(Meteor(bosses[b].position.x, bosses[b].position.y, tx, -ty));
                                meteors.back().radius = 10;
                                meteors.back().color = YELLOW;
                            }
                        }
                        else{
                            meteors.push_back(Meteor(bosses[b].position.x, bosses[b].position.y, velx, vely));
                            
                            if (framesCounter % 200 == 0) {
                                meteors.back().radius = 20;
                                meteors.back().color = YELLOW;
                            }
                            else {
                                meteors.back().radius = 10;
                                meteors.back().color = YELLOW;
                            }
                        }


                    }
                }
            }

            
            
            
            // #########  Boss logic end #########

            // #########  Player logic Begin #########
            
            // Rotation
            for (int i = 0; i < playerNum; i++) {
                players[i].updateRotation();
            }
            
            // Speed
            for (int i = 0; i < 2; i++) {
                players[i].updateSpeed();
            }

            // Controller
            for (int i = 0; i < playerNum; i++) {
                for (int dir = 0; dir < 4; dir++) {
                    players[i].walkCtrl(dir);
                }
            }
            
            // Movement
            for (int i = 0; i < 2; i++) {
                players[i].updatePosition();
            }
            
            // Wall behaviour for player
            for (int i = 0; i < 2; i++) {
                if (players[i].position.x > screenWidth ) players[i].position.x = screenWidth;
                else if (players[i].position.x < -(shipHeight)) players[i].position.x = 0;
                if (players[i].position.y > (screenHeight )) players[i].position.y = screenHeight;
                else if (players[i].position.y < -(shipHeight)) players[i].position.y = 0;
            }

            // #########  Player logic end #########
    
            vector<int> toEraseMeteorId;
            unordered_set<int> toEraseMeteorIdSet;
            vector<int> toEraseBulletId;
            vector<int> toEraseBossId;
            unordered_set<int> toEraseBossIdSet;
            
            // #########  Bullet logic begin #########
            // Bullet Emission
            if (IsKeyPressed(KEY_ENTER)) {
                Bullet newBullet = Bullet();
                newBullet.active = true;
                newBullet.color = MAROON;
                newBullet.position = players[0].position;
                newBullet.radius = 5;
                newBullet.damage = 10;
                newBullet.speed = (Vector2){sin((players[0].rotation + 0)*DEG2RAD)*PLAYER_BULLET_SPEED, cos((players[0].rotation + 180)*DEG2RAD)*PLAYER_BULLET_SPEED};
                playerBullets.push_back(newBullet);
                PlaySound(playerwav);
            }
            
            if (IsKeyPressed(KEY_SPACE)) {
                Bullet newBullet = Bullet();
                newBullet.active = true;
                newBullet.color = DARKBLUE;
                newBullet.position = players[1].position;
                newBullet.radius = 5;
                newBullet.damage = 10;
                newBullet.speed = (Vector2){sin((players[1].rotation + 0)*DEG2RAD)*PLAYER_BULLET_SPEED, cos((players[1].rotation + 180)*DEG2RAD)*PLAYER_BULLET_SPEED};
                playerBullets.push_back(newBullet);
                PlaySound(playerwav);
            }
            
            toEraseBulletId.clear();
            for (int i=0; i< playerBullets.size(); i++)
            {
                if (playerBullets[i].active)
                {
                    // movement
                    playerBullets[i].position.x += playerBullets[i].speed.x;
                    playerBullets[i].position.y += playerBullets[i].speed.y;

                    // wall behaviour
                    if  (playerBullets[i].position.x > screenWidth + playerBullets[i].radius)
                        toEraseBulletId.push_back(i);
                    else if (playerBullets[i].position.x < 0 - playerBullets[i].radius)
                        toEraseBulletId.push_back(i);
                    else if (playerBullets[i].position.y > screenHeight +  playerBullets[i].radius)
                        toEraseBulletId.push_back(i);
                    else if (playerBullets[i].position.y < 0 - playerBullets[i].radius)
                        toEraseBulletId.push_back(i);
                }
            }
            for (int i = (int)toEraseBulletId.size() - 1; i >= 0; i--) {
                playerBullets.erase(playerBullets.begin() + toEraseBulletId[i]);
            }
            // #########  Bullet logic end #########
                        
            
            
            // #########  Meteor logic begin #########
            toEraseMeteorId.clear();
            toEraseMeteorIdSet.clear();
            for (int i=0; i< meteors.size(); i++)
            {
                if (meteors[i].active)
                {
                    // movement
                    meteors[i].position.x += meteors[i].speed.x;
                    meteors[i].position.y += meteors[i].speed.y;

                    // wall behaviour
                    if  (meteors[i].position.x > screenWidth + meteors[i].radius)
                        toEraseMeteorId.push_back(i);
                    else if (meteors[i].position.x < 0 - meteors[i].radius)
                        toEraseMeteorId.push_back(i);
                    else if (meteors[i].position.y > screenHeight + meteors[i].radius)
                        toEraseMeteorId.push_back(i);
                    else if (meteors[i].position.y < 0 - meteors[i].radius)
                        toEraseMeteorId.push_back(i);
                }
            }
            for (int i = (int)toEraseMeteorId.size() - 1; i >= 0; i--) {
                meteors.erase(meteors.begin() + toEraseMeteorId[i]);
            }
            
            // #########  Meteor logic end #########
            
            
            // #########  Collision logic begin #########
            // Collision Player to meteors
            for (int i = 0; i < 2; i++) {
                if (players[i].hp <= 0) continue;
                players[i].updateColliderPosition();
                toEraseMeteorId.clear();
                for (int a = 0; a < meteors.size(); ++a)
                {
                    if (CheckCollisionCircleRec(meteors[a].position, meteors[a].radius, players[i].collider) && meteors[a].active)
                     {
                         players[i].hp -= 10;
                         toEraseMeteorId.push_back(a);
                     }
                }
                for (int j = (int)toEraseMeteorId.size() - 1; j >= 0; j--){
                    meteors.erase(meteors.begin() + toEraseMeteorId[j]);
                }
            }
            if (players[0].hp <= 0 && players[1].hp <= 0) gameOver = true;
            
            // Collision Bullet to meteors
            toEraseMeteorId.clear();
            toEraseMeteorIdSet.clear();
            toEraseBulletId.clear();
            for (int b_id = 0; b_id < playerBullets.size(); b_id++) {
                for (int m_id = 0; m_id < meteors.size(); m_id++) {
                    if (toEraseMeteorIdSet.find(m_id) != toEraseMeteorIdSet.end())
                        continue;
                    if (CheckCollisionCircles(playerBullets[b_id].position, playerBullets[b_id].radius, meteors[m_id].position, meteors[m_id].radius) && playerBullets[b_id].active && meteors[m_id].active) {
                        toEraseMeteorId.push_back(m_id);
                        toEraseMeteorIdSet.insert(m_id);
                        toEraseBulletId.push_back(b_id);
                        break;
                    }
                }
            }
            sort(toEraseBulletId.begin(), toEraseBulletId.end());
            sort(toEraseMeteorId.begin(), toEraseMeteorId.end());
            for (int i = (int)toEraseMeteorId.size() - 1; i >= 0; i--) {
                meteors.erase(meteors.begin() + toEraseMeteorId[i]);
            }
            for (int i = (int)toEraseBulletId.size() - 1; i >= 0; i--) {
                playerBullets.erase(playerBullets.begin() + toEraseBulletId[i]);
            }
            
            // Collision Bullet to boss
            toEraseBulletId.clear();
            toEraseBossId.clear();
            toEraseBossIdSet.clear();
            for (int i = 0; i < bosses.size(); i++) {
                bosses[i].updateColliderPosition();
            }
            for (int bulletId = 0; bulletId < playerBullets.size(); bulletId++) {
                for (int bossId = 0; bossId < bosses.size(); bossId++) {
                    if (bosses[bossId].hp <= 0) continue;
                    if (CheckCollisionCircleRec( playerBullets[bulletId].position, playerBullets[bulletId].radius, bosses[bossId].collider) && playerBullets[bulletId].active)
                    {
                        bosses[bossId].hp -= playerBullets[bulletId].damage;
                        toEraseBulletId.push_back(bulletId);
                        if (bosses[bossId].hp <= 0) {
                            toEraseBossId.push_back(bossId);
                        }
                        break;
                    }
                }
            }
            sort(toEraseBulletId.begin(), toEraseBulletId.end());
            sort(toEraseBossId.begin(), toEraseBossId.end());
            for (int i = (int)toEraseBulletId.size() - 1; i >= 0; i--) {
                playerBullets.erase(playerBullets.begin() + toEraseBulletId[i]);
            }
            for (int i = (int)toEraseBossId.size() - 1; i >= 0; i--) {
                bosses.erase(bosses.begin() + toEraseBossId[i]);
            }
            if (bosses.size() == 0) {
                gameOver = true;
            }
            
            // Collision Player to boss
            for (int i = 0; i < 2; i++) {
                if (players[i].hp <= 0) continue;
                players[i].updateColliderPosition();
                toEraseMeteorId.clear();
                for (int j = 0; j < bosses.size(); j++) {
                    if (CheckCollisionRecs(players[i].collider, bosses[j].collider) && bosses[j].hp > 0)
                    {
                        players[i].hp -= 5;
                        // player bounce away when hit by boss
                        players[i].position.x -= players[i].speed.x*5;
                        players[i].position.y -= players[i].speed.y*5;
                        players[i].acceleration = 0;
                        break;
                    }
                }
            }
            if (players[0].hp <= 0 && players[1].hp <= 0) gameOver = true;
            
            // #########  Collision logic end #########
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
void DrawGame(Texture2D player_model, Texture2D boss_move_model, Texture2D boss_atk_model ,Texture2D bgTexture)
{
    BeginDrawing();

        ClearBackground(RAYWHITE);
        DrawTexture(bgTexture, 0 , 0 , WHITE);
        if (!gameOver)
        {
            //----------------------------------------------------------------------------------draw by yun
            
            frame_count++;

            if (frame_count>= (60/framesSpeed))
            {
                frame_count = 0;
                currentFrame++;
                currentFrame_boss ++;
                if (currentFrame > 3) currentFrame = 0;
                if (currentFrame_boss > 6) currentFrame_boss = 0;
                frameRec_boss.x = (float)currentFrame_boss*frame_boss_w;
                int curr_fx = int((framesCounter%300)/7);
                frameRec_bossatk.x = (float)curr_fx*frame_bossatk_w;
                for (int i = 0; i < 2; i++) {
                    frameRec[i].x = (float)currentFrame*frame_w;
                }
            }

            // Draw boss
            int bossNum = (int) bosses.size();
            for (int i = 0; i < bossNum; i++) {
                Vector2 tmp = { bosses[i].position.x-43, bosses[i].position.y-45};
                Vector2 tmp2 = { bosses[i].position.x-43, bosses[i].position.y-90};
                if(framesCounter%300>=0 &&framesCounter%300<70){
                    DrawTextureRec(boss_atk_model, frameRec_bossatk, tmp2, WHITE);  // Draw part of the texture ,edit by yun
                }
                else{
                    DrawTextureRec(boss_move_model, frameRec_boss, tmp, WHITE);  // Draw part of the texture ,edit by yun
                }
                DrawRectangle(10, 10, bosses[i].hp*3, 30, RED);
            }

            

            // Draw player
            for (int i = 0; i < 2; i++) {
                if (players[i].hp <= 0) continue;
                Vector2 tmp = { players[i].position.x-16, players[i].position.y-28};
                DrawTextureRec(player_model, frameRec[i], tmp, WHITE);  // Draw part of the texture ,edit by yun
                DrawRectangle(players[i].position.x-30, players[i].position.y-40,players[i].hp*3, 3, players[i].color);
            }

            // Draw meteor
            for (int i = 0;i< meteors.size(); i++)
            {

                    if (meteors[i].active){
                        DrawCircleV(meteors[i].position, meteors[i].radius+4, RED);
                        DrawCircleV(meteors[i].position, meteors[i].radius, meteors[i].color);
                        
                    }
                    else DrawCircleV(meteors[i].position, meteors[i].radius, Fade(LIGHTGRAY, 0.3f));

                
            }

            
            // Draw bullet
            for (int i = 0;i< playerBullets.size(); i++)
            {
                if (playerBullets[i].active) DrawCircleV(playerBullets[i].position, playerBullets[i].radius, playerBullets[i].color);
                else DrawCircleV(playerBullets[i].position, playerBullets[i].radius, Fade(playerBullets[i].color, 0.3f));
            }

            DrawText(TextFormat("TIME: %.02f", (float)framesCounter/60), 10, 10, 20, BLACK);

            if (pause) DrawText("GAME PAUSED", screenWidth/2 - MeasureText("GAME PAUSED", 40)/2, screenHeight/2 - 40, 40, GRAY);
        }
        else {
            if (bosses.size() == 0) {
                DrawText("SUCCESS! PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("SUCCESS! PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, GRAY);
            }
            else {
                DrawText("FAIL! PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("FAIL! PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, GRAY);
            }
            
        }

    EndDrawing();
    //----------------------------------------------------------------------------------
}

// Unload game variables
void UnloadGame(void)
{
    // TODO: Unload all dynamic loaded data (textures, sounds, models...)
}

// Update and Draw (one frame)
void UpdateDrawFrame(Texture2D player_model, Texture2D boss_move_model, Texture2D boss_atk_model,Texture2D bgTexture,Sound playerwav,Sound bosswav)
{
    UpdateGame(playerwav, bosswav);
    DrawGame(player_model,boss_move_model,boss_atk_model,bgTexture);
}
