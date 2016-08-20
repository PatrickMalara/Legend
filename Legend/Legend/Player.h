#ifndef Player_H
#define Player_H

#include <SDL.h>
#include "Tile.h"
#include "Weapon.h"

//The Player that will move around on the screen
class Player
{
public:
	//The dimensions of the Player
	static const int Player_WIDTH = 64;
	static const int Player_HEIGHT = 64;

	//Maximum axis velocity of the Player
	static const int Player_VEL = 8;	//SHOULD BE 4 IN REALITY, BUT FOR TESTING I PUT 8

	//Initializes the variables
	Player();

	//Takes key presses and adjusts the Player's velocity
	void handleEvent(SDL_Event& e);

	//Moves the Player
	void move(Tile *tiles[], Player& Player);

	//Shows the Player on the screen
	void render(int frame, int camX, int camY);

	Weapon weapon;

	int getPosX();
	int getPosY();
	void setPosX(int amt);
	void setPosY(int amt);

	int getCurrentHealth();
	int getHealth();
	int getLevel();
	int getXp();
	void setXp(int amt);
	int setLevel(int amt);
	void levelUp();
	void setCurrentHealth(int amt);

	SDL_Rect collider;

private:
	//The X and Y offsets of the Player
	int mPosX, mPosY;

	//The velocity of the Player
	int mVelX, mVelY;

	int currentHealth;
	int health;
	int level;
	int xp;
};

#endif /* Player_H */