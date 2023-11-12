// its code is a bit outdated but who cares lol

//#define DEBUG_CONSOLE

#define MOD_NAME "4DFly"
#define MOD_VER "1.05"

#include <Windows.h>
#include <cstdio>
#include <4dm.h>
using namespace fdm;

#include "4DKeyBinds.h"

// lerp definition ig
#define lerpF(a, b, ratio) (a * (1.f - ratio) + b * ratio)

// variables
bool flyEnabled = false;
float yVel = 0.0f;

void(__thiscall* Player_updatePos)(Player* self, World* world, double dt);
void __fastcall Player_updatePos_H(Player* self, World* world, double dt)
{
	if (flyEnabled)
	{
		// Fly UP if SPACE is pressed and fly DOWN in SHIFT is pressed
		if (self->keys.space && !self->keys.shift) yVel = lerpF(yVel, (self->keys.ctrl ? 20.0f : 15.0f), 10.0f * dt);
		else if (self->keys.shift && !self->keys.space && !self->touchingGround) yVel = lerpF(yVel, (self->keys.ctrl ? -20.0f : -15.0f), 10.0f * dt);
		else yVel = lerpF(yVel, 0.0f, 10.0f * dt);

		if (yVel <= 0.01f && yVel >= -0.01f)
			yVel = 0.f;

		// Change velocity of player
		self->vel = glm::vec4{ self->vel.x, yVel, self->vel.z, self->vel.w };
	}
	else yVel = self->vel.y;

	const float d = self->pos.y + self->vel.y * (float)dt;

	Player_updatePos(self, world, dt);

	// this anti-gravity shit i made is dumb but it works and i dont give a shit :troll:
	if (flyEnabled && !(self->keys.space || self->keys.shift) && (yVel <= 0.01f && yVel >= -0.01f))
		self->pos.y -= self->pos.y - d;
}
bool(__thiscall* Player_keyInput)(Player* self, GLFWwindow* window, World* world, int key, int scancode, int action, int mods);
bool __fastcall Player_keyInput_H(Player* self, GLFWwindow* window, World* world, int key, int scancode, int action, int mods) 
{
	if(!KeyBinds::IsLoaded()) // if no 4DKeyBinds mod
	{
		// Switch `flyEnabled` when F press
		if (key == GLFW_KEY_F && action == GLFW_PRESS)
			flyEnabled = !flyEnabled;
	}
	
	return Player_keyInput(self, window, world, key, scancode, action, mods);
}

void toggleFlyCallback(GLFWwindow* window, int action, int mods)
{
	if(action == GLFW_PRESS)
		flyEnabled = !flyEnabled;
}

DWORD WINAPI Main_Thread(void* hModule)
{
	// create console window if DEBUG_CONSOLE is defined
#ifdef DEBUG_CONSOLE
	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
#endif

	// Hook to the Player::update function
	Hook(reinterpret_cast<void*>(FUNC_PLAYER_UPDATEPOS), reinterpret_cast<void*>(&Player_updatePos_H), reinterpret_cast<void**>(&Player_updatePos));
	// Hook to the Player::keyInput function to add F button
	Hook(reinterpret_cast<void*>(FUNC_PLAYER_KEYINPUT), reinterpret_cast<void*>(&Player_keyInput_H), reinterpret_cast<void**>(&Player_keyInput));

	EnableHook(0);

	KeyBinds::addBind("Toggle Fly", glfw::Keys::F, KeyBindsScope::PLAYER, toggleFlyCallback);

	return true;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD _reason, LPVOID lpReserved)
{
	if (_reason == DLL_PROCESS_ATTACH)
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Main_Thread, hModule, 0, NULL);
	
	return TRUE;
}