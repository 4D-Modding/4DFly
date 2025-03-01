//#define DEBUG_CONSOLE

#include <4dm.h>
using namespace fdm;

initDLL

#include "4DKeyBinds.h"

float deltaRatio(float ratio, double dt, double targetDelta)
{
	const double rDelta = dt / (1.0 / (1.0 / targetDelta));
	const double s = 1.0 - ratio;

	return (float)(1.0 - pow(s, rDelta));
}

float deltaRatio(float ratio, double dt)
{
	return deltaRatio(ratio, dt, 1.0 / 100.0);
}

float lerp(float a, float b, float ratio, bool clampRatio = true)
{
	if (clampRatio)
		ratio = glm::clamp(ratio, 0.f, 1.f);
	return a + (b - a) * ratio;
}

float ilerp(float a, float b, float ratio, double dt, bool clampRatio = true)
{
	return lerp(a, b, deltaRatio(ratio, dt), clampRatio);
}


// variables
bool flyEnabled = false;
float yVel = 0.0f;

$hook(void, Player, update, World* world, double dt, EntityPlayer* entityPlayer)
{
	// check if `self` is the local player. if not then dont do any shit
	if (self != &fdm::StateGame::instanceObj->player) return;
	if (flyEnabled)
	{
		float yVelT = 0;
		float speed = self->keys.ctrl ? 20.0f : 15.0f;
		if (!self->touchingGround && self->keys.shift)
			yVelT -= speed;
		if (self->keys.space)
			yVelT += speed;
		yVel = ilerp(yVel, yVelT, 0.1f, dt);
	}
	original(self, world, dt, entityPlayer);
}

$hook(void, Player, updatePos, World* world, double dt)
{
	if (flyEnabled)
	{
		self->deltaVel.y = 0;
		self->vel.y = yVel;
	}
	else
		yVel = self->vel.y;

	original(self, world, dt);
}

void toggleFlyCallback(GLFWwindow* window, int action, int mods)
{
	if(action == GLFW_PRESS)
		flyEnabled = !flyEnabled;
}

$hook(bool, Player, keyInput, GLFWwindow* window, World* world, int key, int scancode, int action, int mods)
{
	if(!KeyBinds::isLoaded()) // if no 4DKeyBinds mod
	{
		// Switch `flyEnabled` when F press
		if (key == GLFW_KEY_F)
			toggleFlyCallback(window, action, mods);
	}
	
	return original(self, window, world, key, scancode, action, mods);
}

$exec
{
	KeyBinds::addBind("4D-Fly", "Toggle Fly", glfw::Keys::F, KeyBindsScope::PLAYER, toggleFlyCallback);
}
