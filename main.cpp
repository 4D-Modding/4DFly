// its code is a bit outdated but who cares lol

//#define DEBUG_CONSOLE

#define MOD_NAME "4DFly"
#define MOD_VER "0.7"

#include <Windows.h>
#include <cstdio>
#include <4dm.h>
using namespace fdm;

// lerp definition ig
#define lerpF(a, b, ratio) (a * (1.f - ratio) + b * ratio)

// variables
bool flyEnabled = false;
float yVel = 0.0f;

void(__thiscall* Player_update)(Player* self, GLFWwindow* window, World* world, double dt);
void __fastcall Player_update_H(Player* self, GLFWwindow* window, World* world, double dt) 
{
	if (flyEnabled)
	{
		// Fly UP if SPACE is pressed and fly DOWN in SHIFT is pressed
		if (self->keys.space && !self->keys.shift) yVel = lerpF(yVel, (self->keys.ctrl ? 15.0f : 12.0f), 10.0f * dt);
		else if (self->keys.shift && !self->keys.space) yVel = lerpF(yVel, (self->keys.ctrl ? -15.0f : -12.0f), 10.0f * dt);
		else yVel = lerpF(yVel, 0.0f, 10.0f * dt);

		// Change velocity of player
		self->deltaVel = glm::vec4{ 0.0f };
		self->vel = glm::vec4{ self->vel.x, yVel, self->vel.z, self->vel.w };
	}
	else yVel = self->vel.y;
	Player_update(self, window, world, dt);
}
bool(__thiscall* Player_keyInput)(Player* self, GLFWwindow* window, int key, int scancode, int action, int mods);
bool __fastcall Player_keyInput_H(Player* self, GLFWwindow* window, int key, int scancode, int action, int mods) 
{
	// Switch `flyEnabled` when F press
	if (key == GLFW_KEY_F && action == GLFW_PRESS)
		flyEnabled = !flyEnabled;
	return Player_keyInput(self, window, key, scancode, action, mods);
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
	Hook(reinterpret_cast<void*>(base + idaOffsetFix(0x7EB40)), reinterpret_cast<void*>(&Player_update_H), reinterpret_cast<void**>(&Player_update));
	// Hook to the Player::keyInput function to add F button
	Hook(reinterpret_cast<void*>(base + idaOffsetFix(0x81880)), reinterpret_cast<void*>(&Player_keyInput_H), reinterpret_cast<void**>(&Player_keyInput));

	EnableHook(0);
	return true;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD _reason, LPVOID lpReserved)
{
	if (_reason == DLL_PROCESS_ATTACH)
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Main_Thread, hModule, 0, NULL);
	
	return TRUE;
}