//g++ main.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -o main

//Using SDL, SDL_image, SDL_ttf, standard IO, strings, and string streams
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>

#define PI 3.14159265

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCOREBOARD_WIDTH = 100;
const int PLAYFIELD_WIDTH = SCREEN_WIDTH-SCOREBOARD_WIDTH;

//A circle structure
struct Circle{
	int x, y;
	int r;
};

//Texture wrapper class
class LTexture{
	public:
		//Initialize variables
		LTexture();
		
		//Deallocates memory
		~LTexture();
		
		//Loads image at specified path
		bool loadFromFile(std::string path);
		
		#ifdef _SDL_TTF_H
		//Creates image from font string
		bool loadFromRenderedText(std::string textureText, SDL_Color textColor);
		#endif
		
		//Deallocates texture
		void free();
		
		//Set color modulation
		void setColor(Uint8 red, Uint8 green, Uint8 blue);
		
		//Set blending
		void setBlendMode(SDL_BlendMode blending);
		
		//Set alpha modulation
		void setAlpha(Uint8 alpha);
		
		//Renders texture at given point
		void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);
		
		//Gets image dimensions
		int getWidth();
		int getHeight();
		
	private:
		//The actual hardware texture
		SDL_Texture* mTexture;
		
		//Image dimensions
		int mWidth;
		int mHeight;
};

class LTimer{
	public:
		//Initializes variables
		LTimer();
		
		//The various clock actions
		void start();
		void stop();
		void pause();
		void unpause();
		
		//Gets the timer's time
		Uint32 getTicks();
		
		//Checks the status of the timer
		bool isStarted();
		bool isPaused();
		
	private:
		//The clock time when the timer started
		Uint32 mStartTicks;
		
		//The ticks stored when the timer was paused
		Uint32 mPausedTicks;
		
		//The timer status
		bool mPaused;
		bool mStarted;
};

//The bullet that will move around on the screen
class Bullet{
	public:
		//The dimensions of the bullet
		static const int BULLET_WIDTH = 20;
		static const int BULLET_HEIGHT = 20;
		
		//Maximum axis velocity of the bullet
		static const int BULLET_VEL = 5;
		
		//Initializes the variables
		Bullet(double, double, double);
		
		//Moves the bullet
		bool move();
		
		//Shows the bullet on the screen
		void render();
		
		Circle& getCollider();
	
	private:
		//The X and Y offsets of the bullet
		double mPosX, mPosY;
		
		//The velocity of the bullet
		double mVelX, mVelY;
		
		//The angle of the bullet
		double mAngle;
		
		//Pointer to bullet texture image
		LTexture* mBulletTexture;
		
		//Bullet's collision circle
		Circle mCollider;
};

class Enemy{
	public:
		//The dimensions of the enemy
		static const int ENEMY_WIDTH = 20;
		static const int ENEMY_HEIGHT = 20;
		
		//Initializes the variables
		Enemy(int, int, double, double, void (*)(double*, double*, double*, double, double));
		
		//Shoot the friggin' bullets
		void shoot();
		
		//Move enemy
		bool move();
		
		//Show the enemy on the screen
		void render();
	
	private:
		//The X and Y positions of the enemy
		double mPosX, mPosY;
		
		//The X and Y offsets of the enemy
		double mOffsetX, mOffsetY;
		
		//The angle of the enemy
		double mAngle, mRotation;
		
		//The number of, angle between, and frequency of bullets fired
		double mBullets, mBulletAngle;
		int mBulletFrequency;
		
		//Function used for pathing
		void (*mPath)(double*, double*, double*, double, double);
		
		//Enemy texture
		LTexture* mEnemyTexture;
		
		//Timer for the enemy
		LTimer mDurationTimer;
		
		//Timer for shooting
		LTimer mShotTimer;
};

class Player{
	public:
		static const int PLAYER_WIDTH = 20;
		static const int PLAYER_HEIGHT = 20;
		
		Player();
		
		void move(SDL_Event* e);
		
		void render();
		
		Circle& getCollider();
	
	private:
		double mPosX, mPosY;
		
		Circle mCollider;
};

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//Checks collision between two circles
bool checkCollision(Circle&, Circle&);

//Functions for pathing
void pathing(double*, double*, double*, double, double);	

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

TTF_Font* gFont = NULL;

//Playfield textures
LTexture gBulletTexture;
LTexture gEnemyTexture;
LTexture gPlayerTexture;

//Scoreboard textures
LTexture gScoreTexture, gScoreTextTexture;
LTexture gTimeTexture, gTimeTextTexture;

//Logo texture
LTexture gLogoTexture;
Circle gLogoCollider;
double gLogoAngle = 45;;

//Game duration timer
LTimer gDurationTimer;

//Vectors for bullets and enemies
std::vector<Bullet> gBullets;
std::vector<Enemy> gEnemies;

int gScore = 0;
bool gGameOver = false;

int main(int argc, char *args[]){	
	//Start up SDL and create window
	if(!init()){
		printf("Failed to initialize!\n");
	}else{
		//Load media
		if(!loadMedia()){
			printf("Failed to load media!\n");
		}else{
			//Main loop flag
			bool quit = false;
			
			//Event handler
			SDL_Event e;
			
			Player player;
			
			SDL_Color textColor = {0xD0, 0xD0, 0xD0, 0xFF};
			
			std::stringstream scoreText;
			std::stringstream timeText;
			
			gDurationTimer.start();
			
			//Spawn rate timer
			LTimer spawnTimer;
			spawnTimer.start();
			
			int spawnRate = 1000;
			
			//While application is running
			while(!quit){
				if(gDurationTimer.getTicks()/1000 >= 5){
					gDurationTimer.pause();
					gGameOver = true;
				}
				
				//Handle events on queue
				while(SDL_PollEvent(&e) != 0){
					//User requests quit
					if(e.type == SDL_QUIT){
						quit = true;
					}
					
					//Handle input from the player
					player.move(&e);
				}
				
				SDL_Rect playfield = {0, 0, PLAYFIELD_WIDTH, SCREEN_HEIGHT};
				SDL_Rect scoreboard = {PLAYFIELD_WIDTH, 0, SCOREBOARD_WIDTH, SCREEN_HEIGHT};
				
				//Clear screen
				SDL_SetRenderDrawColor(gRenderer, 0xC8, 0x22, 0x22, 0xFF);
				SDL_RenderClear(gRenderer);
				
				//Set the playfield viewport
				SDL_RenderSetViewport(gRenderer, &playfield);
				
				//Render the playfield
				SDL_SetRenderDrawColor(gRenderer, 0xB4, 0xB4, 0xB4, 0xFF);
				SDL_RenderFillRect(gRenderer, &playfield);
				
				//Spawn enemy every "spawnRate" frames
				if(spawnTimer.getTicks() >= spawnRate && !gGameOver){
					static int theta = 0;
					
					//Determines the position of enemy spawn
					theta += PI/2;
					Enemy enemy(5, 100, 1, (PLAYFIELD_WIDTH-20)/2+((PLAYFIELD_WIDTH-20)/2-10)*cos(theta), &pathing);
					gEnemies.push_back(enemy);
					
					//Restarts timer
					spawnTimer.start();
				}
				
				for(int i = 0; i < gEnemies.size(); ++i){
					//Move/rotate enemies
					if(!gEnemies[i].move()){
						gEnemies.erase(gEnemies.begin()+i);
					}else if(!gGameOver){
						//Shoot bullet;
						gEnemies[i].shoot();
					}
				}
				
				player.render();
				
				for(int i = 0; i < gBullets.size(); ++i){
					//Move the bullet
					if(!gBullets[i].move() || checkCollision(player.getCollider(), gBullets[i].getCollider())){
						gBullets.erase(gBullets.begin()+i);
					}else{
						//Render bullets
						gBullets[i].render();
					}
				}
				
				for(int i = 0; i < gEnemies.size(); ++i){
					//Render enemies
					gEnemies[i].render();
				}
				
				SDL_RenderSetViewport(gRenderer, &scoreboard);
				
				scoreText.str("");
				scoreText << gScore;
				
				//Render text
				if(!gScoreTexture.loadFromRenderedText(scoreText.str().c_str(), textColor)){
					printf("Unable to render score texture!\n");
				}
				
				//Render textures
				gScoreTextTexture.render((SCOREBOARD_WIDTH-gScoreTextTexture.getWidth())/2, SCREEN_HEIGHT-2*gScoreTextTexture.getHeight());
				gScoreTexture.render((SCOREBOARD_WIDTH-gScoreTexture.getWidth())/2, SCREEN_HEIGHT-gScoreTexture.getHeight());
				
				timeText.str("");
				timeText << gDurationTimer.getTicks()/1000;
				
				//Render text
				if(!gTimeTexture.loadFromRenderedText(timeText.str().c_str(), textColor)){
					printf("Unable to render time texture!\n");
				}
				
				//Render textures
				gTimeTextTexture.render((SCOREBOARD_WIDTH-gTimeTextTexture.getWidth())/2, 0);
				gTimeTexture.render((SCOREBOARD_WIDTH-gTimeTexture.getWidth())/2, gTimeTexture.getHeight());
				
				//Render logo
				gLogoTexture.render((SCOREBOARD_WIDTH-gLogoTexture.getWidth())/2, (SCREEN_HEIGHT-gLogoTexture.getHeight())/2, NULL, gLogoAngle);
				
				gLogoCollider.x = (SCOREBOARD_WIDTH)/2+PLAYFIELD_WIDTH;
				gLogoCollider.y = (SCREEN_HEIGHT)/2;
				gLogoCollider.r = gLogoTexture.getWidth()/2;
				
				//Update screen
				SDL_RenderPresent(gRenderer);
			}
		}
	}
	
	//Free resources and close SDL
	close();
	
	return 0;
}

LTexture::LTexture(){
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture(){
	//Deallocate
	free();
}

bool LTexture::loadFromFile(std::string path){
	//Get rid of preexisting texture
	free();
	
	//The final texture
	SDL_Texture* newTexture = NULL;
	
	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	
	if(loadedSurface == NULL){
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}else{
		//Color key image
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0xFF, 0xFF, 0xFF));
		
		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		
		if(newTexture == NULL){
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}else{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}
		
		//Get rid of loaded surface
		SDL_FreeSurface(loadedSurface);
	}
	
	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

#ifdef _SDL_TTF_H
bool LTexture::loadFromRenderedText(std::string textureText, SDL_Color textColor){
	//Get rid of preexisting texture
	free();
	
	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
	if(textSurface == NULL){
		printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
	}else{
		//Create texture from surface pixels
		mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
		if(mTexture == NULL){
			printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
		}else{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}
		
		//Get rid of old surface
		SDL_FreeSurface(textSurface);
	}
	
	//Return success
	return mTexture != NULL;
}
#endif

void LTexture::free(){
	//Free texture if it exists
	if(mTexture != NULL){
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor(Uint8 red, Uint8 green, Uint8 blue){
	//Modulate texture
	SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void LTexture::setBlendMode(SDL_BlendMode blending){
	//Set blending function
	SDL_SetTextureBlendMode(mTexture, blending);
}

void LTexture::setAlpha(Uint8 alpha){
	//Modulate texture alpha
	SDL_SetTextureAlphaMod(mTexture, alpha);
}

void LTexture::render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip){
	//Set rendering space and render to screen
	SDL_Rect renderQuad = {x, y, mWidth, mHeight};
	
	//Set clip rendering dimensions
	if(clip != NULL){
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}
	
	//Render to screen
	SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}	

int LTexture::getWidth(){
	return mWidth;
}

int LTexture::getHeight(){
	return mHeight;
}

LTimer::LTimer(){
	//Initialize the variables
	mStartTicks = 0;
	mPausedTicks = 0;
	
	mPaused = false;
	mStarted = false;
}

void LTimer::start(){
	//Start the timer
	mStarted = true;
	
	//Unpause the timer
	mPaused = false;
	
	//Get the current clock time
	mStartTicks = SDL_GetTicks();
	mPausedTicks = 0;
}

void LTimer::stop(){
	//Stop the timer
	mStarted = false;
	
	//Unpause the timer
	mPaused = false;
	
	//Clear tick variables
	mStartTicks = 0;
	mPausedTicks = 0;
}

void LTimer::pause(){
	//If the timer is running and isn't already paused
	if(mStarted && !mPaused){
		//Pause the timer
		mPaused = true;
		
		//Calculate the paused ticks
		mPausedTicks = SDL_GetTicks()-mStartTicks;
		mStartTicks = 0;
	}
}

void LTimer::unpause(){
	//If the timer is running and paused
	if(mStarted && mPaused){
		//Unpause the timer
		mPaused = false;
		
		//Reset the startng ticks
		mStartTicks = SDL_GetTicks()-mPausedTicks;
		
		//Reset the paused ticks
		mPausedTicks = 0;
	}
}

Uint32 LTimer::getTicks(){
	//The actual timer time
	Uint32 time = 0;
	
	//If the timer is running
	if(mStarted){
		//If the timer is paused
		if(mPaused){
			//Return the number of ticks when the timer is paused
			time = mPausedTicks;
		}else{
			//Return the current time minus the start time
			time = SDL_GetTicks()-mStartTicks;
		}
	}
	
	return time;
}

bool LTimer::isStarted(){
	//Timer is running and paused or unpaused
	return mStarted;
}

bool LTimer::isPaused(){
	//Timer is running and paused
	return mPaused && mStarted;
}

Bullet::Bullet(double sPosX, double sPosY, double sAngle){
	//Initializes the offsets
	mPosX = sPosX;
	mPosY = sPosY;
	
	//Initializes the velocity
	mVelX = cos((sAngle+90)*PI/180)*BULLET_VEL;
	mVelY = sin((sAngle+90)*PI/180)*BULLET_VEL;
	
	//Initialize the angle
	mAngle = sAngle;
	
	//Initializes the bullet pointer
	mBulletTexture = &gBulletTexture;
	
	//Initialize the collider
	mCollider.x = mPosX;
	mCollider.y = mPosY;
	mCollider.r = BULLET_WIDTH/2;
}

bool Bullet::move(){
	//Move the bullet left or right
	mPosX += mVelX;
	
	//Move the bullet up or down
	mPosY += mVelY;
	
	mCollider.x = mPosX;
	mCollider.y = mPosY;
	
	//If the bullet went too far to the left, right, up, or down
	if((mPosY+BULLET_HEIGHT < 0) || (mPosY > SCREEN_HEIGHT) || (mPosX+BULLET_WIDTH < 0) || (mPosX > PLAYFIELD_WIDTH)){
		//Return false if bullet is outside screen
		return false;
	}
	
	//Return true if bullet is inside screen
	return true;
}

void Bullet::render(){
	//Show the bullet
	mBulletTexture->render(mPosX, mPosY, NULL, mAngle);
}

Circle& Bullet::getCollider(){
	return mCollider;
}

Enemy::Enemy(int bullets, int bulletFrequency, double rotation, double sOffsetX, void (*path)(double*, double*, double*, double, double)){
	//Initialize the offsets
	mOffsetX = sOffsetX;
	mOffsetY = -ENEMY_HEIGHT;
	
	//Initialize the angle and rotational speed of the enemy
	mAngle = 0;
	mRotation = rotation;
	
	//Initialize the number of, angle between, and firing frequency of bullets
	mBullets = bullets;
	mBulletAngle = 360/mBullets;
	mBulletFrequency = bulletFrequency;
	
	//Initialize the pathing function
	mPath = path;
	mPath(&mPosX, &mPosY, &mAngle, mRotation, 0);
	
	//Initialize enemy texture
	mEnemyTexture = &gEnemyTexture;
	
	//Initialize the timers;
	mDurationTimer.start();
	mShotTimer.start();
}

bool Enemy::move(){
	//Follow the path
	mPath(&mPosX, &mPosY, &mAngle, mRotation, mDurationTimer.getTicks());
	
	//Add the offsets
	mPosX += mOffsetX;
	mPosY += mOffsetY;
	
	//Check if enemy is outside the screen
	if(mPosY+ENEMY_HEIGHT < 0){
		//Return false if bullet is outside screen
		return false;
	}
	
	//Return true if bullet is inside screen
	return true;
}

void Enemy::shoot(){
	//Shoot every mBulletFrequency mSeconds
	if(mShotTimer.getTicks() > mBulletFrequency){
		//Add bullet/s into "barrel" i.e. vector of bullets to be shot
		for(double i = mAngle; i < mAngle+360; i += mBulletAngle){
			Bullet bullet(mPosX, mPosY, i);
			gBullets.push_back(bullet);
		}
		mShotTimer.start();
	}
}

void Enemy::render(){
	//Show the bullet
	mEnemyTexture->render(mPosX, mPosY, NULL, mAngle);
}

Player::Player(){
	mPosX = (PLAYFIELD_WIDTH-PLAYER_WIDTH)/2;
	mPosY = (SCREEN_HEIGHT-PLAYER_HEIGHT)/2;
	
	//Initialize the collider
	mCollider.x = mPosX;
	mCollider.y = mPosY;
	mCollider.r = PLAYER_WIDTH/2;
}

void Player::move(SDL_Event* e){
	//If the mouse is moving
	SDL_ShowCursor(SDL_ENABLE);
	gLogoAngle = 45;
	
	//If the mouse is outside the screen
	if(e->motion.x+PLAYER_WIDTH < PLAYFIELD_WIDTH && e->motion.x > 0){
		mPosX = e->motion.x;
		
		//Hide the cursor from the screen
		SDL_ShowCursor(SDL_DISABLE);
	}
	
	if(e->motion.y+PLAYER_HEIGHT < SCREEN_HEIGHT && e->motion.y > 0){
		mPosY = e->motion.y;
	}
	
	mCollider.x = mPosX;
	mCollider.y = mPosY;
	
	if(gGameOver){
		//Create the collision circle for the restart button
		Circle mouse;
		mouse.x = e->motion.x;
		mouse.y = e->motion.y;
		mouse.r = 0;
		
		//Restart when logo is pressed
		if(checkCollision(mouse, gLogoCollider)){
			gLogoAngle = -45;
			if(e->button.button == SDL_BUTTON_LEFT && e->button.state == SDL_RELEASED){
				gDurationTimer.start();
				gScore = 0;
				gGameOver = false;
			}
		}
	}
}

void Player::render(){
	gPlayerTexture.render(mPosX, mPosY);
}

Circle& Player::getCollider(){
	return mCollider;
}

bool init(){
	//Initialization flag
	bool success = true;
	
	//Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		success = false;
	}else{
		//Set texture filtering to linear
		if(!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")){
			printf("Warning: Linear texture filtering not enabled!");
		}
		
		//Create window
		gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if(gWindow == NULL){
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}else{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			
			if(gRenderer == NULL){
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}else{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				
				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if(!(IMG_Init(imgFlags) & imgFlags)){
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}
				
				//Initialize SDL_ttf
				if(TTF_Init() == -1){
					printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
					success = false;
				}
			}
		}
	}
	
	return success;
}

bool loadMedia(){
	//Loading success flag
	bool success = true;
	
	//Load bullet texture
	if(!gBulletTexture.loadFromFile("bullet.bmp")){
		printf("Failed to load bullet texture!\n");
		success = false;
	}
	
	//Load enemy texture
	if(!gEnemyTexture.loadFromFile("enemy.bmp")){
		printf("Failed to load enemy texture!\n");
		success = false;
	}
	
	//Load player texture
	if(!gPlayerTexture.loadFromFile("player.bmp")){
		printf("Failed to load player texture!\n");
		success = false;
	}
	
	//Load logo texture
	if(!gLogoTexture.loadFromFile("logo.bmp")){
		printf("Failed to load logo texture!\n");
		success = false;
	}
	
	//Open the font
	gFont = TTF_OpenFont("consola.ttf", 28);
	if(gFont == NULL){
		printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
		success = false;
	}else{
		//Set text color
		SDL_Color textColor = {0xD0, 0xD0, 0xD0, 0xFF};
		
		//Load 'time' texture
		if(!gTimeTextTexture.loadFromRenderedText("TIME", textColor)){
			printf("Unable to render 'time' texture!\n");
			success = false;
		}
		
		//Load 'score' texture
		if(!gScoreTextTexture.loadFromRenderedText("SCORE", textColor)){
			printf("Unable to render 'time' texture!\n");
			success = false;
		}
	}
	
	return success;
}

void close(){
	//Free loaded images
	gBulletTexture.free();
	gEnemyTexture.free();
	gPlayerTexture.free();
	gTimeTexture.free();
	gScoreTexture.free();
	gTimeTextTexture.free();
	gScoreTextTexture.free();
	gLogoTexture.free();
	
	//Free global font
	TTF_CloseFont(gFont);
	gFont = NULL;
	
	//Destroy window
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;
	
	//Quit SDL subsystems
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

bool checkCollision(Circle& c1, Circle& c2){
	if(sqrt(pow(c1.x-c2.x, 2)+pow(c1.y-c2.y, 2)) < c1.r+c2.r){
		if(!gGameOver){
			++gScore;
		}
		return true;
	}
	return false;
}

void pathing(double* xPos, double* yPos, double* angle, double r, double t){
	*yPos = 0.1*t;
	*xPos = 0;
	*angle += r;
}