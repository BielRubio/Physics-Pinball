#pragma once
#include "Module.h"
#include "p2List.h"
#include "p2Point.h"
#include "Globals.h"

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

	PhysBody* kicker;
	int kickerX = 448, kickerY = 530;
	bool kickerCharge = false;

	b2RevoluteJoint* flipper_joints[2]; 
	// Lower ground sensor (will kill all objects passig through it)
	PhysBody* lower_ground_sensor;
	bool sensed;

	// Textures
	SDL_Texture* ball;
	SDL_Texture* box;
	SDL_Texture* rick;

	SDL_Texture* map;
	SDL_Texture* bg;
	SDL_Texture* kicker1;
	int bgOffset;

	// FX
	uint bonus_fx;

	// Raycast
	p2Point<int> ray;
	bool ray_on;
};
