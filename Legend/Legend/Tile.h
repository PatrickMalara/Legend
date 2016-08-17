#ifndef Tile_H
#define Tile_H

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <iostream>

//The tile
class Tile
{
public:
	//Initializes position and type
	Tile(int x, int y, int tileType);

	//Shows the tile
	void render(SDL_Rect& camera);

	//Get the tile type
	int getType();

	//Get the collision box
	SDL_Rect getBox();

	const int TILE_WIDTH = 80;
	const int TILE_HEIGHT = 80;

private:
	//The attributes of the tile
	SDL_Rect mBox;

	//The tile type
	int mType;
};

#endif /* Tile_H */