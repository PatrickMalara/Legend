
// Using SDL, SDL_image, standard IO, strings, and file streams
#include "LTexture.h"
#include "LTimer.h"
#include "Player.h"

//Screen dimension constants
const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 800;

//The dimensions of the level
const int LEVEL_WIDTH = 5760;
const int LEVEL_HEIGHT = 6080;

int dir = 0;
bool fighting = false;
bool isOpeningChest = false;
bool inDungen = false;
int fightTime = 1;

//AREAS
enum class AREA{
	FOREST1,
	FOREST2,
	FOREST3,
	FOREST4,
	FORESTDUNGEN1
};

AREA AreaState = AREA::FOREST1;

//Tile constants
const int TOTAL_TILES = 5472; // 36, 38
const int TOTAL_TILE_SPRITES = 12;
const int TOTAL_ENEMIES = 5;
const int TILE_WIDTH = 80;
const int TILE_HEIGHT = 80;

//The different tile sprites
const int TILE_RED = 0;
const int TILE_GREEN = 1;
const int TILE_BLUE = 2;
const int TILE_CENTER = 3;
const int TILE_TOP = 4;
const int TILE_TOPRIGHT = 5;
const int TILE_RIGHT = 6;
const int TILE_BOTTOMRIGHT = 7;
const int TILE_BOTTOM = 8;
const int TILE_BOTTOMLEFT = 9;
const int TILE_LEFT = 10;
const int TILE_TOPLEFT = 11;
const int TILE_STAIRS = 12;

//Analog joystick dead zone
const int JOYSTICK_DEAD_ZONE = 8000;

//Normalized direction
int xDir = 0;
int yDir = 0;

//Enemies entity class
class Enemy{
public:
	SDL_Rect collider;

	Enemy();

	void render(int camX, int camY, SDL_Rect hitBox);

	LTimer atkTimer;
	int atkTime;

	int getPosX();
	int getPosY();
	int getCurrentHealth();
	int getHealth();
	void setCurrentHealth(int amt);

	int imageID = 0;
	int damage;

private:
	int currentHealth;
	int health;
	int ePosX, ePosY;
	int eVelX, eVelY;
};

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia(Tile* tiles[] );

//Frees media and shuts down SDL
void close(Tile* tiles[]);

bool checkCollision(SDL_Rect a, SDL_Rect b);

//Checks collision box against set of tiles
bool touchesWall(SDL_Rect box, Tile* tiles[], Player& Player);

//Sets tiles from tile map
bool setTiles(Tile *tiles[]);
void loadForestArea2(Tile *tiles[]);

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
static SDL_Renderer* gRenderer = NULL;

//The level tiles
Tile* tileSet[TOTAL_TILES];

//Globally used font
TTF_Font *gFont = NULL;

//Rendered texture
LTexture gTextTexture;

//Scene textures
LTexture gPlayerTexture;
LTexture gWeaponTexture;
LTexture gSwordTexture;
LTexture gTileTexture;
LTexture gEnemyPortrait;
LTexture gFightingBar;

SDL_Rect TileClips[TOTAL_TILE_SPRITES];
SDL_Rect EnemyPortraitsClips[TOTAL_ENEMIES];

const unsigned int SWORD_ANIMATION_FRAMES = 1;
SDL_Rect SwordClipDown[2];
SDL_Rect SwordClipLeft[1];
SDL_Rect SwordClipUp[1];
SDL_Rect SwordClipRight[1];

SDL_Rect BackgroundClip[4];

const unsigned int WALKING_ANIMATION_FRAMES = 6;
SDL_Rect SpriteClipsDown[WALKING_ANIMATION_FRAMES];
SDL_Rect SpriteClipsLeft[WALKING_ANIMATION_FRAMES];
SDL_Rect SpriteClipsUp[WALKING_ANIMATION_FRAMES];
SDL_Rect SpriteClipsRight[WALKING_ANIMATION_FRAMES];

LTimer timer;

//Game Controller 1 handler
SDL_Joystick* gGameController = NULL;

void Tile::render(SDL_Rect& camera)
{
	//If the tile is on screen
	if (checkCollision(camera, mBox))
	{
		//Show the tile
		gTileTexture.render(mBox.x - camera.x, mBox.y - camera.y, gRenderer, &TileClips[mType]);
	}
}

Weapon::Weapon(){
	clip.w = 100;
	clip.h = 100;
	clip.x = 0;
	clip.y = 0;
	damage = 0;
}

void Weapon::render(int xPos, int yPos){
	SDL_Rect* currentClip = &clip;
	gWeaponTexture.render(xPos, yPos, gRenderer, currentClip);
}

Player::Player()
{
	//Initialize the offsets
	mPosX = 0;
	mPosY = 0;

	//Initialize the velocity
	mVelX = 0;
	mVelY = 0;

	//Initialize the Level
	level = 1;

	xp = 0;

	//Initialize the health
	health = 100;
	currentHealth = health;
	
	//Initialize the collision box
	collider.x = 0;
	collider.y = 0;
	collider.w = Player_WIDTH;
	collider.h = Player_HEIGHT;
}

void Player::handleEvent(SDL_Event& e)
{
	if (e.type == SDL_JOYAXISMOTION)
	{
		//Motion on controller 0
		if (e.jaxis.which == 0)
		{
			//X axis motion
			if (e.jaxis.axis == 0)
			{
				//Left of dead zone
				if (e.jaxis.value < -JOYSTICK_DEAD_ZONE)
				{
					mVelX -= Player_VEL; dir = 4;
				}
				//Right of dead zone
				else if (e.jaxis.value > JOYSTICK_DEAD_ZONE)
				{
					mVelX += Player_VEL; dir = 2;
				}
				else
				{
					mVelX += Player_VEL;
					mVelX -= Player_VEL;

				}
			}//Y axis motion
			else if (e.jaxis.axis == 1)
			{
				//Below of dead zone
				if (e.jaxis.value < -JOYSTICK_DEAD_ZONE)
				{
					mVelY -= Player_VEL; dir = 1;
				}
				//Above of dead zone
				else if (e.jaxis.value > JOYSTICK_DEAD_ZONE)
				{
					mVelY += Player_VEL; dir = 3;
				}
				else
				{
					mVelY += Player_VEL;
					mVelY -= Player_VEL;
					
				}
			}
		}
	}
	//If a key was pressed
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
		case SDLK_UP: mVelY -= Player_VEL; dir = 1; break;
		case SDLK_DOWN: mVelY += Player_VEL; dir = 3; break;
		case SDLK_LEFT: mVelX -= Player_VEL; dir = 4;  break;
		case SDLK_RIGHT: mVelX += Player_VEL; dir = 2; break;

		case SDLK_1: mPosX = 80; mPosY = 1840; collider.x = mPosX; collider.y = mPosY; break;
		case SDLK_f: loadForestArea2(tileSet); break;
		case SDLK_2: fighting = !fighting; break;
		case SDLK_3: weapon.clip.y += 100; break;
		case SDLK_4: weapon.clip.y -= 100; break;
		}
	}
	//If a key was released
	else if (e.type == SDL_KEYUP && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
		case SDLK_UP:    mVelY += Player_VEL; break;
		case SDLK_DOWN:  mVelY -= Player_VEL; break;
		case SDLK_LEFT:  mVelX += Player_VEL; break;
		case SDLK_RIGHT: mVelX -= Player_VEL; break;
		}
	}
}

void Player::move(Tile *tiles[], Player& Player)
{
	//Move the Player left or right
	collider.x += mVelX;
	mPosX += mVelX;

	//If the Player went too far to the left or right or touched a wall
	if ((collider.x < 0) || (collider.x + Player_WIDTH > LEVEL_WIDTH) || touchesWall(collider, tiles, Player))
	{
		//move back
		collider.x -= mVelX;
		mPosX -= mVelX;
	}

	//Move the Player up or down
	collider.y += mVelY;
	mPosY += mVelY;

	//If the Player went too far up or down or touched a wall
	if ((collider.y < 0) || (collider.y + Player_HEIGHT > LEVEL_HEIGHT) || touchesWall(collider, tiles, Player))
	{
		//move back
		collider.y -= mVelY;
		mPosY -= mVelY;
	}
}

void Player::render(int frame, int camPosX, int camPosY) {
	//Render current frame
	if (dir == 1){
		SDL_Rect* currentClip = &SpriteClipsUp[frame / 6];
		gPlayerTexture.render(mPosX - camPosX, mPosY - camPosY, gRenderer, currentClip);
	}
	else if (dir == 2){
		SDL_Rect* currentClip = &SpriteClipsRight[frame / 6];
		gPlayerTexture.render(mPosX - camPosX, mPosY - camPosY, gRenderer, currentClip);
	}
	else if (dir == 3){
		SDL_Rect* currentClip = &SpriteClipsDown[frame / 6];
		gPlayerTexture.render(mPosX - camPosX, mPosY - camPosY, gRenderer, currentClip);
	}
	else if (dir == 4) {
		SDL_Rect* currentClip = &SpriteClipsLeft[frame / 6];
		gPlayerTexture.render(mPosX - camPosX, mPosY - camPosY, gRenderer, currentClip);
	}
}

int Player::getPosX(){ return mPosX; }
int Player::getPosY(){ return mPosY; }
void Player::setPosX(int amt){ mPosX = amt; collider.x = amt; }
void Player::setPosY(int amt){ mPosY = amt; collider.y = amt; }
int Player::getHealth(){ return health; }
int Player::getCurrentHealth(){ return currentHealth; }
void Player::setCurrentHealth(int amt){ currentHealth = amt; }
void Player::levelUp(){ level++; }
int Player::getLevel(){ return level; }
int Player::getXp(){ return xp; }
void Player::setXp(int amt){ xp = amt; }


Enemy::Enemy(){
	health = 100;
	currentHealth = health;
	ePosX = 124;
	ePosY = 124;
	eVelX = 3;
	eVelY = 0;
	collider = {ePosX, ePosY, 1000, 600};
	damage = 1;
	atkTime = 1;
}

void Enemy::render(int camX, int camY, SDL_Rect hitBox) {

	SDL_Rect enemyP;
	enemyP.h = gEnemyPortrait.getHeight();
	enemyP.w = gEnemyPortrait.getWidth();
	enemyP.x = 0;
	enemyP.y = 0;
	SDL_Rect* currentClip = &EnemyPortraitsClips[imageID];

	gEnemyPortrait.render(0, 0, gRenderer, currentClip);
}

int Enemy::getPosX(){ return ePosX; }
int Enemy::getPosY(){ return ePosY; }
int Enemy::getHealth(){ return health; }
int Enemy::getCurrentHealth(){ return currentHealth; }
void Enemy::setCurrentHealth(int amt){ currentHealth = amt; }

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		//Check for joysticks
		if (SDL_NumJoysticks() < 1)
		{
			printf("Warning: No joysticks connected!\n");
		}
		else
		{
			//Load joystick
			gGameController = SDL_JoystickOpen(0);
			if (gGameController == NULL)
			{
				printf("Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError());
			}
		}

		//Create window
		gWindow = SDL_CreateWindow("Eos", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}

				//Initialize SDL_ttf
				if (TTF_Init() == -1)
				{
					printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadMedia(Tile* tiles[]) {
	//Load Player texture
	if (!gPlayerTexture.loadFromFile("Zelda.png", gRenderer)) {
		printf("Failed to load Player texture!\n");
		return false; }
	//Load tile texture
	if (!gTileTexture.loadFromFile("tiles.png", gRenderer))
	{
		printf("Failed to load tile set texture!\n");
		return false;
	}
	//Load the enemy Portaits
	if (!gEnemyPortrait.loadFromFile("enemyPortrait.png", gRenderer)){
		printf("Failed to load enemy Portrait texture!\n");
		return false;
	}
	//Load the enemy Portaits
	if (!gWeaponTexture.loadFromFile("weapons.png", gRenderer)){
		printf("Failed to load Weapon texture!\n");
		return false;
	}
	//Load the enemy Portaits
	if (!gFightingBar.loadFromFile("FightBarUI.png", gRenderer)){
		printf("Failed to load enemy Portrait texture!\n");
		return false;
	}
	else
	{
		EnemyPortraitsClips[0].x = 0;
		EnemyPortraitsClips[0].y = 0;
		EnemyPortraitsClips[0].w = 1000;
		EnemyPortraitsClips[0].h = 600;

		EnemyPortraitsClips[1].x = 0;
		EnemyPortraitsClips[1].y = 600;
		EnemyPortraitsClips[1].w = 1000;
		EnemyPortraitsClips[1].h = 600;

		EnemyPortraitsClips[2].x = 0;
		EnemyPortraitsClips[2].y = 1200;
		EnemyPortraitsClips[2].w = 1000;
		EnemyPortraitsClips[2].h = 600;

		EnemyPortraitsClips[3].x = 0;
		EnemyPortraitsClips[3].y = 1800;
		EnemyPortraitsClips[3].w = 1000;
		EnemyPortraitsClips[3].h = 600;

		EnemyPortraitsClips[4].x = 0;
		EnemyPortraitsClips[4].y = 2400;
		EnemyPortraitsClips[4].w = 1000;
		EnemyPortraitsClips[4].h = 600;
	}

	//Open the font
	gFont = TTF_OpenFont("SDS_8x8.ttf", 18);
	if (gFont == NULL)
	{
		printf("Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError());
		return false;
	}
	else
	{
		//Render text
		SDL_Color textColor = { 255, 255, 255 };
		if (!gTextTexture.loadFromRenderedText("The quick brown fox jumps over the lazy dog", textColor, gRenderer, gFont))
		{
			printf("Failed to render text texture!\n");
			return false;
		}
	}

	//Load tile map
	if (!setTiles(tiles))
	{
		printf("Failed to load tile set!\n");
		return false;
	}
	else {
		
		//Set sprite clips
		SpriteClipsDown[0].x = 0;
		SpriteClipsDown[0].y = 0;
		SpriteClipsDown[0].w = 64;
		SpriteClipsDown[0].h = 70;
		
		SpriteClipsDown[1].x = 0;
		SpriteClipsDown[1].y = 0;
		SpriteClipsDown[1].w = 64;
		SpriteClipsDown[1].h = 70;
		
		SpriteClipsDown[2].x = 0;
		SpriteClipsDown[2].y = 0;
		SpriteClipsDown[2].w = 64;
		SpriteClipsDown[2].h = 70;
		
		SpriteClipsDown[3].x = 0;
		SpriteClipsDown[3].y = 120;
		SpriteClipsDown[3].w = 64;
		SpriteClipsDown[3].h = 70;
		
		SpriteClipsDown[4].x = 0;
		SpriteClipsDown[4].y = 120;
		SpriteClipsDown[4].w = 64;
		SpriteClipsDown[4].h = 70;
		
		SpriteClipsDown[5].x = 0;
		SpriteClipsDown[5].y = 120;
		SpriteClipsDown[5].w = 64;
		SpriteClipsDown[5].h = 70;
		//---------------------------|
		SpriteClipsLeft[0].x = 120;
		SpriteClipsLeft[0].y = 0;
		SpriteClipsLeft[0].w = 64;
		SpriteClipsLeft[0].h = 70;
			
		SpriteClipsLeft[1].x = 120;
		SpriteClipsLeft[1].y = 0;
		SpriteClipsLeft[1].w = 64;
		SpriteClipsLeft[1].h = 70;
		
		SpriteClipsLeft[2].x = 120;
		SpriteClipsLeft[2].y = 0;
		SpriteClipsLeft[2].w = 64;
		SpriteClipsLeft[2].h = 70;
		
		SpriteClipsLeft[3].x = 120;
		SpriteClipsLeft[3].y = 120;
		SpriteClipsLeft[3].w = 64;
		SpriteClipsLeft[3].h = 70;
		
		SpriteClipsLeft[4].x = 120;
		SpriteClipsLeft[4].y = 120;
		SpriteClipsLeft[4].w = 64;
		SpriteClipsLeft[4].h = 70;
		
		SpriteClipsLeft[5].x = 120;
		SpriteClipsLeft[5].y = 120;
		SpriteClipsLeft[5].w = 64;
		SpriteClipsLeft[5].h = 70; 
		//---------------------------|

		SpriteClipsUp[0].x = 240;
		SpriteClipsUp[0].y = 0;
		SpriteClipsUp[0].w = 64;
		SpriteClipsUp[0].h = 70;

		SpriteClipsUp[1].x = 240;
		SpriteClipsUp[1].y = 0;
		SpriteClipsUp[1].w = 64;
		SpriteClipsUp[1].h = 70;

		SpriteClipsUp[2].x = 240;
		SpriteClipsUp[2].y = 0;
		SpriteClipsUp[2].w = 64;
		SpriteClipsUp[2].h = 70;

		SpriteClipsUp[3].x = 240;
		SpriteClipsUp[3].y = 120;
		SpriteClipsUp[3].w = 64;
		SpriteClipsUp[3].h = 70;

		SpriteClipsUp[4].x = 240;
		SpriteClipsUp[4].y = 120;
		SpriteClipsUp[4].w = 64;
		SpriteClipsUp[4].h = 70;

		SpriteClipsUp[5].x = 240;
		SpriteClipsUp[5].y = 120;
		SpriteClipsUp[5].w = 64;
		SpriteClipsUp[5].h = 70;
		//---------------------------|

		SpriteClipsRight[0].x = 360;
		SpriteClipsRight[0].y = 0;
		SpriteClipsRight[0].w = 64;
		SpriteClipsRight[0].h = 70;

		SpriteClipsRight[1].x = 360;
		SpriteClipsRight[1].y = 0;
		SpriteClipsRight[1].w = 64;
		SpriteClipsRight[1].h = 70;

		SpriteClipsRight[2].x = 360;
		SpriteClipsRight[2].y = 0;
		SpriteClipsRight[2].w = 64;
		SpriteClipsRight[2].h = 70;

		SpriteClipsRight[3].x = 360;
		SpriteClipsRight[3].y = 240;
		SpriteClipsRight[3].w = 64;
		SpriteClipsRight[3].h = 70;

		SpriteClipsRight[4].x = 360;
		SpriteClipsRight[4].y = 240;
		SpriteClipsRight[4].w = 64;
		SpriteClipsRight[4].h = 70;

		SpriteClipsRight[5].x = 360;
		SpriteClipsRight[5].y = 240;
		SpriteClipsRight[5].w = 64;
		SpriteClipsRight[5].h = 70;
		//=======================================
		//=======================================
		SwordClipDown[0].x = 0;
		SwordClipDown[0].y = 345;
		SwordClipDown[0].w = 64;
		SwordClipDown[0].h = 70;

		SwordClipRight[0].x = 345;
		SwordClipRight[0].y = 350;
		SwordClipRight[0].w = 64;
		SwordClipRight[0].h = 70;

		SwordClipUp[0].x = 245;
		SwordClipUp[0].y = 345;
		SwordClipUp[0].w = 70;
		SwordClipUp[0].h = 70;

		SwordClipLeft[0].x = 111;
		SwordClipLeft[0].y = 345;
		SwordClipLeft[0].w = 64;
		SwordClipLeft[0].h = 70;

	}

	return true; 
}

void close(Tile* tiles[]) {

	//Close game controller
	SDL_JoystickClose(gGameController);
	gGameController = NULL;

	//Deallocate tiles
	for (int i = 0; i < TOTAL_TILES; ++i)
	{
		if (tiles[i] == NULL)
		{
			delete tiles[i];
			tiles[i] = NULL;
		}
	}
	//Free loaded images
	gPlayerTexture.free();
	gTileTexture.free();
	gSwordTexture.free();

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit(); 
}

bool checkCollision(SDL_Rect a, SDL_Rect b) {
	//The sides of the rectangles
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;

	//Calculate the sides of rect A
	leftA = a.x;
	rightA = a.x + a.w;
	topA = a.y;
	bottomA = a.y + a.h;

	//Calculate the sides of rect B
	leftB = b.x;
	rightB = b.x + b.w;
	topB = b.y;
	bottomB = b.y + b.h;

	//If any of the sides from A are outside of B
	if (bottomA <= topB)
		return false;
	if (topA >= bottomB)
		return false;
	if (rightA <= leftB)
		return false;
	if (leftA >= rightB)
		return false;
	//If none of the sides from A are outside B
	return true; }

//Checks collision box against set of tiles
bool touchesWall(SDL_Rect box, Tile* tiles[], Player& Player){
	//Go through the tiles
	for (int i = 0; i < TOTAL_TILES; ++i)
	{
		
		//If the tile is stairs
		if (tiles[i]->getType() == TILE_BLUE){
			//If the collision box touches the wall tile
			if (checkCollision(box, tiles[i]->getBox()))
			{
				inDungen = !inDungen;
				if (inDungen){
					Player.setPosX(1920);
					Player.setPosY(2160);
					fightTime = rand() % 10 + 1;
					std::cout << fightTime << std::endl;
					timer.start();
				}
				else if (!inDungen){
					Player.setPosX(1840);
					Player.setPosY(481);
					timer.stop();
				}
				return false;
			}
		}

		//If the tile is stairs
		if (tiles[i]->getType() == TILE_TOPRIGHT){
			//If the collision box touches the wall tile
			if (checkCollision(box, tiles[i]->getBox()))
			{
				inDungen = !inDungen;
				if (inDungen){
					Player.setPosX(80);
					Player.setPosY(2240);
					fightTime = rand() % 10 + 1;
					std::cout << fightTime << std::endl;
					timer.start();
				}
				else if (!inDungen){
					Player.setPosX(800);
					Player.setPosY(561);
					timer.stop();
				}
				return false;
			}
		}

		//If the tile is a wall type tile
		if ((tiles[i]->getType() >= TILE_CENTER) && (tiles[i]->getType() <= TILE_TOPLEFT))
		{
			//If the collision box touches the wall tile
			if (checkCollision(box, tiles[i]->getBox()))
			{
				return true;
			}
		}

		
	}

	//If no wall tiles were touched
	return false;
}

bool setTiles(Tile* tiles[])
{
	//Success flag
	bool tilesLoaded = true;

	//The tile offsets
	int x = 0, y = 0;

	//Open the map
	std::ifstream map("lazy - Copy.map");

		//Initialize the tiles
		for (int i = 0; i < TOTAL_TILES; ++i)
		{
			//Determines what kind of tile will be made
			int tileType = -1;

			//Read tile from map file
			map >> tileType;

			//If the was a problem in reading the map
			if (map.fail())
			{
				//Stop loading map
				printf("Error loading map: Unexpected end of file!\n");
				tilesLoaded = false;
				break;
			}

			//If the number is a valid tile number
			if ((tileType >= 0) && (tileType < TOTAL_TILE_SPRITES))
			{
				tiles[i] = new Tile(x, y, tileType);
			}
			//If we don't recognize the tile type
			else
			{
				//Stop loading map
				printf("Error loading map: Invalid tile type at %d!\n", i);
				tilesLoaded = false;
				break;
			}

			//Move to next tile spot
			x += TILE_WIDTH;

			//If we've gone too far
			if (x >= LEVEL_WIDTH)
			{
				//Move back
				x = 0;

				//Move to the next row
				y += TILE_HEIGHT;
			}
		}

	//Clip the sprite sheet
	if (tilesLoaded)
	{
		TileClips[TILE_RED].x = 0;
		TileClips[TILE_RED].y = 0;
		TileClips[TILE_RED].w = TILE_WIDTH;
		TileClips[TILE_RED].h = TILE_HEIGHT;

		TileClips[TILE_GREEN].x = 0;
		TileClips[TILE_GREEN].y = 80;
		TileClips[TILE_GREEN].w = TILE_WIDTH;
		TileClips[TILE_GREEN].h = TILE_HEIGHT;

		TileClips[TILE_BLUE].x = 0;
		TileClips[TILE_BLUE].y = 160;
		TileClips[TILE_BLUE].w = TILE_WIDTH;
		TileClips[TILE_BLUE].h = TILE_HEIGHT;

		TileClips[TILE_TOPLEFT].x = 80;
		TileClips[TILE_TOPLEFT].y = 0;
		TileClips[TILE_TOPLEFT].w = TILE_WIDTH;
		TileClips[TILE_TOPLEFT].h = TILE_HEIGHT;

		TileClips[TILE_LEFT].x = 80;
		TileClips[TILE_LEFT].y = 80;
		TileClips[TILE_LEFT].w = TILE_WIDTH;
		TileClips[TILE_LEFT].h = TILE_HEIGHT;

		TileClips[TILE_BOTTOMLEFT].x = 80;
		TileClips[TILE_BOTTOMLEFT].y = 160;
		TileClips[TILE_BOTTOMLEFT].w = TILE_WIDTH;
		TileClips[TILE_BOTTOMLEFT].h = TILE_HEIGHT;

		TileClips[TILE_TOP].x = 160;
		TileClips[TILE_TOP].y = 0;
		TileClips[TILE_TOP].w = TILE_WIDTH;
		TileClips[TILE_TOP].h = TILE_HEIGHT;

		TileClips[TILE_CENTER].x = 160;
		TileClips[TILE_CENTER].y = 80;
		TileClips[TILE_CENTER].w = TILE_WIDTH;
		TileClips[TILE_CENTER].h = TILE_HEIGHT;

		TileClips[TILE_BOTTOM].x = 160;
		TileClips[TILE_BOTTOM].y = 160;
		TileClips[TILE_BOTTOM].w = TILE_WIDTH;
		TileClips[TILE_BOTTOM].h = TILE_HEIGHT;

		TileClips[TILE_TOPRIGHT].x = 240;
		TileClips[TILE_TOPRIGHT].y = 0;
		TileClips[TILE_TOPRIGHT].w = TILE_WIDTH;
		TileClips[TILE_TOPRIGHT].h = TILE_HEIGHT;

		TileClips[TILE_RIGHT].x = 240;
		TileClips[TILE_RIGHT].y = 80;
		TileClips[TILE_RIGHT].w = TILE_WIDTH;
		TileClips[TILE_RIGHT].h = TILE_HEIGHT;

		TileClips[TILE_BOTTOMRIGHT].x = 240;
		TileClips[TILE_BOTTOMRIGHT].y = 160;
		TileClips[TILE_BOTTOMRIGHT].w = TILE_WIDTH;
		TileClips[TILE_BOTTOMRIGHT].h = TILE_HEIGHT;
	}

	//Close the file
	map.close();

	//If the map was loaded fine
	return tilesLoaded;
}

void loadForestArea2(Tile* tiles[]){
	//The tile offsets
	int x = 0, y = 0;

	//Open the map
	std::ifstream map("ForestArea2.map");

	//Initialize the tiles
	for (int i = 0; i < TOTAL_TILES; ++i)
	{
		//Determines what kind of tile will be made
		int tileType = -1;

		//Read tile from map file
		map >> tileType;

		//If the was a problem in reading the map
		if (map.fail())
		{
			//Stop loading map
			printf("Error loading map: Unexpected end of file!\n");
			break;
		}

		//If the number is a valid tile number
		if ((tileType >= 0) && (tileType < TOTAL_TILE_SPRITES))
		{
			tiles[i] = new Tile(x, y, tileType);
		}
		//If we don't recognize the tile type
		else
		{
			//Stop loading map
			printf("Error loading map: Invalid tile type at %d!\n", i);
			break;
		}

		//Move to next tile spot
		x += TILE_WIDTH;

		//If we've gone too far
		if (x >= LEVEL_WIDTH)
		{
			//Move back
			x = 0;

			//Move to the next row
			y += TILE_HEIGHT;
		}
	}

	//Close the file
	map.close();
}

void dungenEnemy(Enemy& enemy){
	enemy.setCurrentHealth(100);
	enemy.atkTime = 1;

	//Do some like random generation
	enemy.imageID = rand() % 5 + 0;
	enemy.damage = rand() % 10 + 1;

}

///Randomly generates a weapon
Weapon genChestWeapon(int start, int end){
	Weapon newWeapon;
	int level = rand() % end + start;
	newWeapon.clip.y = level * 100;
	newWeapon.damage = level * 10;
	return newWeapon;
}

void run(){
	//Start up SDL and create window
	if (!init())
		printf("Failed to initialize!\n");
	else {

		//Load media
		if (!loadMedia(tileSet))
			printf("Failed to load media!\n");
		else {
			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;

			//The Player that will be moving around on the screen
			Player Player;
			Player.weapon.damage = 5;
			Player.setPosY(5920);
			Player.setPosX(81);

			Weapon chestWeapon;

			int eHp = 1;

			SDL_Rect hitBox = { 0 };
			bool forestArea2Chest = true;
			SDL_Rect chest = { 160, 5920, 100, 100 };


			SDL_Rect forestArea1Exit1 = {5100, 40, 160, 20};

			int frame = 0;

			//The camera area
			SDL_Rect camera = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
			Enemy enemy;

			#pragma region The Buttons for the Fighting and Chest UI
			///BattleEngineButtons
			SDL_Rect weaponAtk = {
				0,
				600,
				SCREEN_WIDTH / 2,
				100
			};

			SDL_Rect spellAtk = {
					SCREEN_WIDTH / 2,
					600,
					SCREEN_WIDTH / 2,
					100
			};

			///ChestOpen
			SDL_Rect openingChest = {
				0,
				0,
				SCREEN_WIDTH,
				SCREEN_HEIGHT
			};
			#pragma endregion
			
			//While application is running
			while (!quit) {
				//Handle events on queue
				while (SDL_PollEvent(&e) != 0) {
					//User requests quit
					if (e.type == SDL_QUIT)
						quit = true;

					if (fighting){
						///WeaponAtk
						if (e.type == SDL_MOUSEBUTTONDOWN){
							if (e.button.y > 600 && e.button.y < 700 && e.button.x < (SCREEN_WIDTH / 2)){
								enemy.setCurrentHealth(enemy.getCurrentHealth() - Player.weapon.damage);
								std::cout << "Weapon Atk: 10 dmg" << std::endl;
								std::cout << "ENEMY HP: " << enemy.getCurrentHealth() << std::endl;
							}
						}
						///SpellAtk
						if (e.type == SDL_MOUSEBUTTONDOWN){
							if (e.button.y > 600 && e.button.y < 700 && e.button.x >(SCREEN_WIDTH / 2)){
								enemy.setCurrentHealth(enemy.getCurrentHealth() - 5);
								std::cout << "Spell Atk: 10 dmg" << std::endl;
								std::cout << "ENEMY HP: " << enemy.getCurrentHealth() << std::endl;
							}

						}
					}
					if (isOpeningChest){
						///CHANGE
						if (e.type == SDL_MOUSEBUTTONDOWN){
							if (e.button.y > 600 && e.button.y < 700 && e.button.x < (SCREEN_WIDTH / 2)){
								Player.weapon = chestWeapon;
								isOpeningChest = false;
							}
						}
						///LEAVE
						if (e.type == SDL_MOUSEBUTTONDOWN){
							if (e.button.y > 600 && e.button.y < 700 && e.button.x >(SCREEN_WIDTH / 2)){
								isOpeningChest = false;
							}
						}
					}
					//Handle input for the Player
					Player.handleEvent(e);
				}
				if (!fighting && !isOpeningChest){
				
					//Move the Player
					Player.move(tileSet, Player);

					//Center the camera over the Player
					camera.x = (Player.getPosX() + Player::Player_WIDTH / 2) - SCREEN_WIDTH / 2;
					camera.y = (Player.getPosY() + Player::Player_HEIGHT / 2) - SCREEN_HEIGHT / 2;

					//Keep the camera in bounds
					if (camera.x < 0)
						camera.x = 0;
					if (camera.y < 0)
						camera.y = 0;
					if (camera.x > LEVEL_WIDTH - camera.w)
						camera.x = LEVEL_WIDTH - camera.w;
					if (camera.y > LEVEL_HEIGHT - camera.h)
						camera.y = LEVEL_HEIGHT - camera.h;

					//Clear screen by setting the background color to white
					SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
					SDL_RenderClear(gRenderer);

					//Render level
					for (int i = 0; i < TOTAL_TILES; ++i)
						tileSet[i]->render(camera);

					if (AreaState == AREA::FOREST1){
						if (checkCollision(forestArea1Exit1, Player.collider)){
							loadForestArea2(tileSet);
							Player.setPosY(5920);
							Player.setPosX(81);
							AreaState = AREA::FOREST2;
						}
					}
					else if (AreaState == AREA::FOREST2){

						if (forestArea2Chest){
							//If the tile is on screen
							if (checkCollision(camera, chest))
							{
								//Show the tile
								gTileTexture.render(chest.x - camera.x, chest.y - camera.y, gRenderer, &TileClips[TILE_BOTTOMRIGHT]);
							}
							if (checkCollision(chest, Player.collider)){
								isOpeningChest = true; 
								chestWeapon = genChestWeapon(1, 2);
								forestArea2Chest = false;
							}
						}
					}
					//Render objects
					Player.render(frame, camera.x, camera.y);

				}
				else if (isOpeningChest){
					//Clear screen by setting the background color to white
					SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
					SDL_RenderClear(gRenderer);
					Player.weapon.render(SCREEN_WIDTH / 3, SCREEN_HEIGHT / 2.5);
					chestWeapon.render(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2.5);

					//RENDER THE BUTTONS
					SDL_Color textColor = { 255, 255, 255 };
					SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0x00, 0xFF);
					SDL_RenderFillRect(gRenderer, &weaponAtk);
					gTextTexture.loadFromRenderedText("CHANGE", textColor, gRenderer, gFont);
					//Render current frame
					gTextTexture.render(125, 625, gRenderer);


					SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0xFF, 0xFF);
					SDL_RenderFillRect(gRenderer, &spellAtk);
					gTextTexture.loadFromRenderedText("LEAVE", textColor, gRenderer, gFont);
					//Render current frame
					gTextTexture.render(625, 625, gRenderer);

					std::string currentDamage = "PLAYER DAMAGE:" + std::to_string(Player.weapon.damage);
					gTextTexture.loadFromRenderedText(currentDamage, textColor, gRenderer, gFont);
					gTextTexture.render(125, 225, gRenderer);
					std::string chestDamage = "DAMAGE:" + std::to_string(chestWeapon.damage);
					gTextTexture.loadFromRenderedText(chestDamage, textColor, gRenderer, gFont);
					gTextTexture.render(625, 225, gRenderer);

				}
				else if (fighting){

					//Clear screen by setting the background color to white
					SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
					SDL_RenderClear(gRenderer);

					if (enemy.getCurrentHealth() <= 0){
						Player.setXp(Player.getXp() + 10);
						if (Player.getXp() >= (Player.getLevel() * 80)){
							Player.levelUp();
							Player.setXp(0);
						}
						fighting = false;
						timer.start();
						fightTime = rand() % 10 + 1;
					}

					enemy.render(camera.x, camera.y, SDL_Rect{ 0 });

					//RENDER THE BUTTONS
					SDL_Color textColor = { 255, 255, 255 };
					SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0x00, 0xFF);
					SDL_RenderFillRect(gRenderer, &weaponAtk);
					gTextTexture.loadFromRenderedText("Weapon Attack", textColor, gRenderer, gFont);
					//Render current frame
					gTextTexture.render(125, 625, gRenderer);


					SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0xFF, 0xFF);
					SDL_RenderFillRect(gRenderer, &spellAtk);
					gTextTexture.loadFromRenderedText("Spell Attack", textColor, gRenderer, gFont);
					//Render current frame
					gTextTexture.render(625, 625, gRenderer);


					SDL_Rect fightBar;
					fightBar.h = gFightingBar.getHeight();
					fightBar.w = gFightingBar.getWidth();
					fightBar.x = 0;
					fightBar.y = 0;
					SDL_Rect* currentClip = &fightBar;
					gFightingBar.render(0, 700, gRenderer, currentClip);


					///Render / Load as little as possible
					std::string enemyHealth = "Enemy HP:" + std::to_string(enemy.getCurrentHealth());
					gTextTexture.loadFromRenderedText(enemyHealth, textColor, gRenderer, gFont);
					//Render current frame
					gTextTexture.render(105, 705, gRenderer);
					std::string playerHealth = "Player HP:" + std::to_string(Player.getCurrentHealth()) + " | Lvl: " + std::to_string(Player.getLevel()) + " XP: " + std::to_string(Player.getXp()) + "/" + std::to_string(Player.getLevel() * 80);
					gTextTexture.loadFromRenderedText(playerHealth, textColor, gRenderer, gFont);
					//Render current frame
					gTextTexture.render(105, 765, gRenderer);

					Player.weapon.render(0, 700);

					if (enemy.atkTimer.getTicks() / 1000.f >= enemy.atkTime){
						Player.setCurrentHealth(Player.getCurrentHealth() - enemy.damage);
						enemy.atkTimer.stop();
						enemy.atkTimer.start();
					}

				}

				//Update screen
				SDL_RenderPresent(gRenderer);

				std::cout << timer.getTicks() / 1000.f << std::endl;
				if (timer.getTicks() / 1000.f >= fightTime){
					dungenEnemy(enemy);
					enemy.atkTimer.start();
					fighting = true; 
					timer.stop();
				}

				//Cylce Frames
				++frame;
				if (frame / 6 >= WALKING_ANIMATION_FRAMES) frame = 0;
			}
		}
		SDL_Delay(5000);
		//Free resources and close SDL
		close(tileSet);
	}
}

int main(int argc, char* args[]) {
	
	run();
	
	return 0;
}