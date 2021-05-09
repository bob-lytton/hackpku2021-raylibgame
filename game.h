#pragma once
#include "raylib.h"
#include <math.h>
#include <time.h>
#include <cstdio>
#include <cstdlib>
#include <vector>

using namespace std;

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
class Player {
public:
    Vector2 position;
    Vector2 speed;
    float acceleration;
    float rotation;
    Vector3 collider;
    Color color;
    float hp;
};

class Meteor {
public:
    Vector2 position;
    Vector2 speed;
    float radius;
    bool active;
    Color color;
};

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
const int screenWidth = 800;
const int screenHeight = 450;

int framesCounter = 0;
bool gameOver = false;
bool pause = false;

// NOTE: Defined triangle is isosceles with common angles of 70 degrees.
static float shipHeight = 0.0f;

vector<Player> players(2);
vector<Meteor> mediumMeteor;
vector<Meteor> smallMeteor;