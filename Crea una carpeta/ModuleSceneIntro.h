#pragma once
#include "Module.h"
#include "p2List.h"
#include "p2Point.h"
#include "Globals.h"
#include <iostream>
#include <sstream>
using namespace std;


class PhysBody;
class b2RevoluteJoint;

class ModuleSceneIntro : public Module
{
public:

	// Constructors & Destructors
	ModuleSceneIntro(Application* app, bool start_enabled = true);
	~ModuleSceneIntro();

	// Main module steps
	bool Start();
	update_status Update();
	bool CleanUp();
	void OnCollision(PhysBody* bodyA, PhysBody* bodyB);
	void CreateBoard(); 
	void SpawnBall();
	void UpdateBall(); 
	void GameOver(); 
	void UpdateScore(); 

public:

	// Lists of physics objects
	p2List<PhysBody*> circles;
	PhysBody* board;
	PhysBody* wall; 
	PhysBody* flipperWallLeft;
	PhysBody* flipperWallRight;


	PhysBody* flipperLeft; 
	PhysBody* flipperLeftAnchor;
	PhysBody* flipperRight;
	PhysBody* flipperRightAnchor;

	PhysBody* circleBumper[40]; 
	PhysBody* wallBumper[2]; 

	PhysBody* kicker;
	int kickerX = 448, kickerY = 530;
	bool kickerCharge = false;

	b2RevoluteJoint* flipper_joints[2]; 
	// Lower ground sensor (will kill all objects passig through it)
	PhysBody* lower_ground_sensor;
	bool sensed;

	// Textures
	SDL_Texture* ball;
	SDL_Texture* flippers;
	SDL_Texture* bumpers;
	SDL_Texture* smallBumpers;
	SDL_Texture* wall_bumpers;
	SDL_Texture* box;
	SDL_Texture* rick;

	SDL_Texture* map;
	SDL_Texture* GOBG;
	SDL_Texture* bg;
	SDL_Texture* kicker1;
	int Font;
	int bgOffset;


	// FX
	uint bonus_fx;
	uint boing_fx;

	// Raycast
	p2Point<int> ray;
	bool ray_on;

	//Score
	const char* scoreS;
	char ScoreR[256];
	char ScoreP[256];
	char ScoreH[256];
	char BallsC[256];
	int score = 50; 
	int previousScore; 
	int highScore;  
	int ballsCounter; 

	int comboCounter; 

	bool gameOver; 
};
