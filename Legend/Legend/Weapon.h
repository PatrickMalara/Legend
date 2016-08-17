#ifndef Weapon_H
#define Weapon_H

#include <SDL.h>

class Weapon
{
public:
	Weapon();

	//Shows the Player on the screen
	void render();
	SDL_Rect clip;

	std::string name;
	int damage;
};
# endif /* Weapon_H*/