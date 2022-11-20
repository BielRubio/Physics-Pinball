#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModuleSceneIntro.h"
#include "ModuleInput.h"
#include "ModuleTextures.h"
#include "ModuleAudio.h"
#include "ModulePhysics.h"

ModuleSceneIntro::ModuleSceneIntro(Application* app, bool start_enabled) : Module(app, start_enabled)
{

	// Initialise all the internal class variables, at least to NULL pointer
	ball = box = rick = NULL;
	ray_on = false;
	sensed = false;
}

ModuleSceneIntro::~ModuleSceneIntro()
{
	// You should do some memory cleaning here, if required
}

bool ModuleSceneIntro::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;

	// Set camera position
	App->renderer->camera.x = App->renderer->camera.y = 0;

	// Load textures
	char lookupTable[] = { "0123456789" };
	Font = App->renderer->Load("pinball/FontW.png", lookupTable, 1);
	ball = App->textures->Load("pinball/ball.png"); 
	flippers = App->textures->Load("pinball/flipper.png");
	bumpers = App->textures->Load("pinball/bouncers.png");
	smallBumpers = App->textures->Load("pinball/small_bumper.png");
	wall_bumpers = App->textures->Load("pinball/wall_bouncer.png");
	map = App->textures->Load("pinball/map.png");
	bg = App->textures->Load("pinball/bg.png");
	GOBG = App->textures->Load("pinball/lose_screen.png");
	kicker1 = App->textures->Load("pinball/kicker.png");
	
	bgOffset = 0;

	bonus_fx = App->audio->LoadFx("pinball/bonus.wav");
	boing_fx = App->audio->LoadFx("pinball/boing.wav");

	// Create a big red sensor on the bottom of the screen.
	// This sensor will not make other objects collide with it, but it can tell if it is "colliding" with something else
	lower_ground_sensor = App->physics->CreateRectangleSensor(SCREEN_WIDTH / 2, SCREEN_HEIGHT, SCREEN_WIDTH, 20);

	// Add this module (ModuleSceneIntro) as a listener for collisions with the sensor.
	// In ModulePhysics::PreUpdate(), we iterate over all sensors and (if colliding) we call the function ModuleSceneIntro::OnCollision()
	lower_ground_sensor->listener = this;
	lower_ground_sensor->type = COLLIDER::FALL;

	score = 0; 
	ballsCounter = 3; 
	comboCounter = 0; 

	CreateBoard(); 

	SpawnBall();

	App->audio->PlayMusic("pinball/music.ogg");

	return ret;
}

void ModuleSceneIntro::CreateBoard() {

	int FlipperL[20] = {
		9, 2,
		3, 5,
		1, 11,
		3, 17,
		8, 21,
		60, 19,
		62, 17,
		64, 12,
		62, 8,
		58, 6
	};

	int FlipperR[20] = {
		42, 2,
		49, 4,
		51, 9,
		51, 15,
		48, 20,
		42, 21,
		-5, 17,
		-8, 14,
		-8, 9,
		-4, 6,
	};

	int GameBoard[76] = {
		0, 0,
		512, 0,
		512, 768,
		288, 768,
		288, 736,
		396, 683,
		400, 676,
		400, 560,
		352, 513,
		352, 511,
		383, 448,
		400, 448,
		400, 368,
		432, 368,
		432, 608,
		464, 608,
		464, 132,
		458, 112,
		446, 91,
		433, 77,
		414, 63,
		401, 55,
		374, 48,
		125, 48,
		91, 61,
		68, 81,
		54, 109,
		48, 128,
		48, 448,
		66, 448,
		96, 511,
		96, 513,
		48, 560,
		48, 675,
		57, 688,
		160, 736,
		160, 768,
		0, 768
	};

	int boardWall[62]{
		400, 320,
		432, 320, 
		432, 139, 
		428, 125, 
		416, 106, 
		400, 91,
		385, 86,
		367, 80,
		306, 80, 
		285, 86, 
		276, 91, 
		264, 99, 
		255, 110,
		250, 117, 
		244, 131, 
		240, 146, 
		240, 164, 
		245, 172, 
		252, 176,
		260, 176, 
		267, 172, 
		272, 164, 
		272, 144,
		276, 133, 
		287, 120, 
		297, 115, 
		304, 112, 
		368, 112, 
		377, 115, 
		393, 129, 
		400, 144
	};

	int fWallLeft[24]{
		80, 598,  
		80, 663, 
		136, 689, 
		142, 689, 
		144, 686, 
		143, 684, 
		142, 675, 
		105, 655, 
		96, 646, 
		96, 598, 
		91, 592, 
		86, 592, 
	}; 

	int fWallRight[24]{ 
		307, 674, 
		349, 651, 
		352, 645, 
		352, 598, 
		357, 592, 
		363, 592, 
		368, 598, 
		368, 663, 
		313, 689, 
		308, 689, 
		305, 686, 
		305, 677, 
	}; 
	int vBumper[16]{
		0, 6,
		0, 59, 
		5, 64,
		11, 64, 
		16, 59, 
		16, 6,
		11, 0, 
		6, 0,
	};
	board = App->physics->CreateChain(0, 0, GameBoard, 76);
	board->body->SetType(b2_staticBody);
	
	wall = App->physics->CreateChain(0,0,boardWall,62); 
	wall->body->SetType(b2_staticBody); 

	flipperWallLeft = App->physics->CreateChain(0, 0, fWallLeft, 24); 
	flipperWallLeft->body->SetType(b2_staticBody); 

	flipperWallRight = App->physics->CreateChain(0, 0, fWallRight, 24);
	flipperWallRight->body->SetType(b2_staticBody);

	flipperLeftAnchor = App->physics->CreateCircle(137, 680, 6);
	flipperLeft = App->physics->CreateFlipper(1, 137, 680, FlipperL, 20, 11, 11, 20.0f, 20.0f, -0.15f, 0.15f, flipperLeftAnchor->body);
	flipperLeft->type = COLLIDER::FLIPPER; 

	flipperRightAnchor = App->physics->CreateCircle(311, 680, 6);
	flipperRight = App->physics->CreateFlipper(0, 311, 680, FlipperR, 20, 42, 11, 20.0f, -20.0f, -0.15f, 0.15f, flipperRightAnchor->body);
	flipperRight->type = COLLIDER::FLIPPER;

	circleBumper[0] = App->physics->CreateBumper(118, 224, 32); 
	circleBumper[1] = App->physics->CreateBumper(330, 224, 32);
	circleBumper[2] = App->physics->CreateBumper(224, 240, 32);
	circleBumper[3] = App->physics->CreateBumper(336, 112, 16);
	circleBumper[4] = App->physics->CreateBumper(368, 112, 16);
	circleBumper[5] = App->physics->CreateBumper(302, 112, 16);
	circleBumper[6] = App->physics->CreateBumper(224, 400, 16);
	circleBumper[7] = App->physics->CreateBumper(288, 368, 16);
	circleBumper[8] = App->physics->CreateBumper(160, 368, 16);

	wallBumper[0] = App->physics->CreateVerticalBumper(44, 384, vBumper, 16);
	wallBumper[1] = App->physics->CreateVerticalBumper(388, 384, vBumper, 16);

	kicker = App->physics->CreateRectangle(kickerX, kickerY, 31, 60); 
	kicker->body->SetGravityScale(0);
	kicker->body->SetLinearVelocity({ 0,0 });
	kicker->body->SetFixedRotation(true);
}

bool ModuleSceneIntro::CleanUp()
{
	LOG("Unloading Intro scene");

	return true;
}

update_status ModuleSceneIntro::Update()
{

	//Draw BG
	if (bgOffset >= 512) {
		bgOffset = 0;
	}
	else {
		bgOffset++;
	}
	App->renderer->Blit(bg, 0 - bgOffset, 0);

	// Draw map
	App->renderer->Blit(kicker1, kicker->GetPositionX(), kicker->GetPositionY());
	App->renderer->Blit(map,0,0);

	//Kicker
	App->renderer->BlitText(200, 12, Font, "0000");
	if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT && kicker->GetPositionY() <= 546 && kickerCharge == false && gameOver == false) {
		kicker->body->SetLinearVelocity({ 0,1 });
	}
	else {
		if (kickerCharge == false && kicker->GetPositionY() >= 546) {
			kicker->body->SetLinearVelocity({ 0,0 });
		}
		if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_IDLE) {
			kickerCharge = true;
		}
	}
	if (kickerCharge == true) {
		if (kicker->GetPositionY() >= 530 && kicker->GetPositionY() < 534) {
			kicker->body->SetLinearVelocity({ 0,-10 });
		}
		if (kicker->GetPositionY() >= 534 && kicker->GetPositionY() < 546) {
			kicker->body->SetLinearVelocity({ 0,-16 });
		}
		if (kicker->GetPositionY() >= 546) {
			kicker->body->SetLinearVelocity({ 0,-30 });
		}
	}
	if (kicker->GetPositionY() <= (kickerY-10) && kickerCharge == true) {
		kicker->body->SetLinearVelocity({ 0,0 });
		kicker->body->SetTransform({PIXEL_TO_METERS(448),PIXEL_TO_METERS(530) }, 0.0f);
		kickerCharge = false;
	}
	// If user presses 1, create a new circle object
	if(App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
	{
		circles.add(App->physics->CreateCircle(App->input->GetMouseX(), App->input->GetMouseY(), 12));

		// Add this module (ModuleSceneIntro) as a "listener" interested in collisions with circles.
		// If Box2D detects a collision with this last generated circle, it will automatically callback the function ModulePhysics::BeginContact()
		circles.getLast()->data->listener = this;
	}


	if (App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_DOWN)
	{
		flipperLeft->body->ApplyAngularImpulse(-30,true); 
	}

	if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_DOWN)
	{
		flipperRight->body->ApplyAngularImpulse(50, true);
	}

	//Debug keys
	if (App->input->GetKey(SDL_SCANCODE_F2) == KEY_DOWN)
	{
		gameOver = true;
		UpdateBall();
		ballsCounter = 0;

	}
	if (App->input->GetKey(SDL_SCANCODE_F3) == KEY_DOWN)
	{
		ballsCounter++;
	}
	if (App->input->GetKey(SDL_SCANCODE_F4) == KEY_DOWN)
	{
		ballsCounter--;
	}


	UpdateBall(); 
	//Draw flippers
	
	SDL_Rect r1 = { 0,0,59,16 };
	SDL_Rect r2 = { 59,0,59,16 };

	App->renderer->Blit(flippers, METERS_TO_PIXELS(flipperLeft->body->GetPosition().x), METERS_TO_PIXELS(flipperLeft->body->GetPosition().y + 5), &r1, 1.0f, flipperLeft->GetRotation(), PIXEL_TO_METERS(5), PIXEL_TO_METERS(5));
	App->renderer->Blit(flippers, METERS_TO_PIXELS(flipperRight->body->GetPosition().x), METERS_TO_PIXELS(flipperRight->body->GetPosition().y + 5), &r2, 1.0f, flipperRight->GetRotation(), PIXEL_TO_METERS(5), PIXEL_TO_METERS(5));

	//Draw bumpers

	for (int i = 0; i < 3; i++) {
		App->renderer->Blit(bumpers, circleBumper[i]->GetPositionX()+24, circleBumper[i]->GetPositionY()+24);

	}
	for (int i = 3; i < 9; i++) {
		App->renderer->Blit(smallBumpers, circleBumper[i]->GetPositionX() + 16, circleBumper[i]->GetPositionY() + 16);

	}

	//Draw wall bumpers
	for (int i = 0; i < 2; i++) {
		App->renderer->Blit(wall_bumpers, wallBumper[i]->GetPositionX(), wallBumper[i]->GetPositionY());

	}

	// Prepare for raycast ------------------------------------------------------
	
	// The target point of the raycast is the mouse current position (will change over game time)
	iPoint mouse;
	mouse.x = App->input->GetMouseX();
	mouse.y = App->input->GetMouseY();

	// Total distance of the raycast reference segment
	int ray_hit = ray.DistanceTo(mouse);

	// Declare a vector. We will draw the normal to the hit surface (if we hit something)
	fVector normal(0.0f, 0.0f);

	// All draw functions ------------------------------------------------------

	// Circles
	p2List_item<PhysBody*>* c = circles.getFirst();
	while(c != NULL)
	{
		int x, y;
		c->data->GetPosition(x, y);
		App->renderer->Blit(ball, x, y, NULL, 1.0f);
		c = c->next;
	}

	if (gameOver == true) {
		GameOver(); 
	}

	//Music

	

	// Keep playing
	return UPDATE_CONTINUE;
}

void ModuleSceneIntro::OnCollision(PhysBody* bodyA, PhysBody* bodyB)
{
	// Play Audio FX on every collision, regardless of who is colliding
	//App->audio->PlayFx(bonus_fx);

	switch (bodyB->type) {
	case COLLIDER::BUMPER:
		comboCounter++;
		score += 100;
		App->audio->PlayFx(boing_fx);
		if (comboCounter >= 3 && comboCounter < 6 ) {
			score += 100; 
		}
		else if (comboCounter >= 6 && comboCounter < 9) {
			score += 200; 
		}
		else if (comboCounter >= 9 && comboCounter < 12) {
			score += 400; 
		}
	case COLLIDER::FLIPPER:
		comboCounter = 0; 
	}

	// Do something else. You can also check which bodies are colliding (sensor? ball? player?)
}

void ModuleSceneIntro::SpawnBall() {

	circles.add(App->physics->CreateCircle(450, 500, 12));
	circles.getLast()->data->listener = this;
}

void ModuleSceneIntro::UpdateBall() {
	p2List_item<PhysBody*>* ball = circles.getFirst(); 

	if (ball->data->body->GetPosition().y > PIXEL_TO_METERS(768)) {
		ballsCounter--;
		if (ballsCounter <= 0) {
			gameOver = true; 
		}
		else {
			ball->data->body->SetLinearVelocity({ 0,0 });
			ball->data->body->SetTransform({ PIXEL_TO_METERS(450), PIXEL_TO_METERS(400) }, 0);
		}
	}
}

void ModuleSceneIntro::GameOver() {

	App->renderer->Blit(GOBG, 0, 0);
	if (App->input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN) {
		gameOver = false;
		ballsCounter = 3;
	}

}

void ModuleSceneIntro::UpdateScore() {

}

