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

	ball = App->textures->Load("pinball/ball.png"); 
	flippers = App->textures->Load("pinball/flippers.png");
	map = App->textures->Load("pinball/map.png");
	bg = App->textures->Load("pinball/bg.png");
	kicker1 = App->textures->Load("pinball/kicker.png");
	bgOffset = 0;

	bonus_fx = App->audio->LoadFx("pinball/bonus.wav");

	// Create a big red sensor on the bottom of the screen.
	// This sensor will not make other objects collide with it, but it can tell if it is "colliding" with something else
	lower_ground_sensor = App->physics->CreateRectangleSensor(SCREEN_WIDTH / 2, SCREEN_HEIGHT, SCREEN_WIDTH, 20);

	// Add this module (ModuleSceneIntro) as a listener for collisions with the sensor.
	// In ModulePhysics::PreUpdate(), we iterate over all sensors and (if colliding) we call the function ModuleSceneIntro::OnCollision()
	lower_ground_sensor->listener = this;

	CreateBoard(); 

	return ret;
}

void ModuleSceneIntro::CreateBoard() {

	int FlipperL[20] = {
		9, 2,
		3, 5,
		1, 11,
		3, 17,
		8, 21,
		46, 19,
		50, 17,
		52, 12,
		50, 8,
		46, 6
	};

	int FlipperR[20] = {
		42, 2,
		49, 4,
		51, 9,
		51, 15,
		48, 20,
		42, 21,
		5, 17,
		2, 14,
		2, 9,
		6, 6,
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

	board = App->physics->CreateChain(0, 0, GameBoard, 76);
	board->body->SetType(b2_staticBody);
	
	wall = App->physics->CreateChain(0,0,boardWall,62); 
	wall->body->SetType(b2_staticBody); 

	flipperWallLeft = App->physics->CreateChain(0, 0, fWallLeft, 24); 
	flipperWallLeft->body->SetType(b2_staticBody); 

	flipperWallRight = App->physics->CreateChain(0, 0, fWallRight, 24);
	flipperWallRight->body->SetType(b2_staticBody);

	flipperLeftAnchor = App->physics->CreateCircle(137, 680, 6);
	flipperLeft = App->physics->CreateFlipper(1, 112, 666, FlipperL, 20, 11, 11, 20.0f, 20.0f, -0.15f, 0.15f, flipperLeftAnchor->body);

	flipperRightAnchor = App->physics->CreateCircle(311, 680, 6);
	flipperRight = App->physics->CreateFlipper(0, 321, 666, FlipperR, 20, 42, 11, 20.0f, -20.0f, -0.15f, 0.15f, flipperRightAnchor->body);

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

	// If user presses SPACE, enable RayCast
	if(App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
	{
		// Enable raycast mode
		ray_on = !ray_on;

		// Origin point of the raycast is be the mouse current position now (will not change)
		ray.x = App->input->GetMouseX();
		ray.y = App->input->GetMouseY();
	}
	//Kicker
	if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT && kicker->GetPositionY() <= 546 && kickerCharge == false) {
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
			kicker->body->SetLinearVelocity({ 0,-20 });
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
		flipperLeft->body->ApplyAngularImpulse(-50,true); 
	}

	if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_DOWN)
	{
		flipperRight->body->ApplyAngularImpulse(50, true);
	}

	//Draw flippers
	
	SDL_Rect r1 = { 0,0,58,25 };
	SDL_Rect r2 = { 58,0,58,25 };

	App->renderer->Blit(flippers, flipperLeft->GetPositionX(), flipperLeft->GetPositionY(), &r1, 1.0f, -flipperLeft->GetRotation());
	App->renderer->Blit(flippers, flipperRight->GetPositionX(), flipperRight->GetPositionY(), &r2, 1.0f, flipperRight->GetRotation());

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

	// Raycasts -----------------
	if(ray_on == true)
	{
		// Compute the vector from the raycast origin up to the contact point (if we're hitting anything; otherwise this is the reference length)
		fVector destination(mouse.x-ray.x, mouse.y-ray.y);
		destination.Normalize();
		destination *= ray_hit;

		// Draw a line from origin to the hit point (or reference length if we are not hitting anything)
		App->renderer->DrawLine(ray.x, ray.y, ray.x + destination.x, ray.y + destination.y, 255, 255, 255);

		// If we are hitting something with the raycast, draw the normal vector to the contact point
		if(normal.x != 0.0f)
			App->renderer->DrawLine(ray.x + destination.x, ray.y + destination.y, ray.x + destination.x + normal.x * 25.0f, ray.y + destination.y + normal.y * 25.0f, 100, 255, 100);
	}

	// Keep playing
	return UPDATE_CONTINUE;
}

void ModuleSceneIntro::OnCollision(PhysBody* bodyA, PhysBody* bodyB)
{
	// Play Audio FX on every collision, regardless of who is colliding
	App->audio->PlayFx(bonus_fx);

	// Do something else. You can also check which bodies are colliding (sensor? ball? player?)
}

