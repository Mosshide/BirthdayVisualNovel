/*
Last Update:	5/26/2014

Known Bugs:		Never any bugs ever.


<Program written with Microsoft Visual Studio 2012 in C++ using SDL 2.0 for windows/graphics/sound/input.>
*/

#include "SDL.h" 
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>      
#include <stdlib.h>
#include <time.h>
using namespace std;

//enumerations

enum states{
	text_debug_screen = -1,
	main_menu,
	walk_screen,
	battle_screen,
	story_screen,
	map_making,
	credits
};

enum adjacent{
	right,
	up_right,
	up,
	up_left,
	left,
	bottom_left,
	bottom,
	bottom_right
};

enum effect{
	fly,
	rain,
	fire,
	wind,
	snow
};

enum visibility{
	visible,
	invisible
};

//timers 
Uint32 sdltimer;
Uint32 dsdltimer;
Uint32 sel_timer_x;
Uint32 sel_timer_y;
Uint32 tmrRain;

bool quit;

//////gamestuff
//game states/variables
char buffer [33];
int intFPS;
int current_map;

//battle variables
int turn;//0 is player, 1 attacking enemy, 2 enemy attacking player
int enemy_selected = -1;

//walk variables
float fltStandardVelocity = .5;

//ui variables
int intMMBtnSelection = 0;
int map_selected_x = 0;
int map_selected_y = 0;
bool sel_change_ok_x = true;
bool sel_change_ok_y = true;
int intDefaultBtnPaddingY = 5;
int intDefaultBtnPaddingX = 5;
bool blnNoSaveFile = true;

//battle ui
int battle_option[4] = {-1, -1, -1, -1};
bool no_mana_note[4];
int target_enemy[4] = {-1, -1, -1, -1};
int party_sel = 0;

//particle stuff
bool blnMakeRain;
int intRainCount;
float fltRainSpeed = 1;
float fltFlySpeed = .20;
float fltFlyAcc = .001;
float fltFlyJerk = .00005;
effect pfxMM = rain;
float fltParticleScaling = 1;

//mouse location
bool lb_down;
bool lb_down_init;
bool lb_up;
bool rb_down;
bool rb_down_init;
bool rb_up;
int mouse_x;
int mouse_y;
int delta_mouse_x;
int delta_mouse_y;

//key variables
bool b_up;
bool blnADown;
bool blnSDown;
bool blnDDown;
bool blnWDown;
bool blnAUp;
bool blnSUp;
bool blnDUp;
bool blnWUp;
bool blnEscUp;
bool blnMouseWheelUp;
bool blnMouseWheelDown;
bool blnMouseWheelClickDown;
bool blnTabUp;

int intMouseWheelY;

//screen values
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 450;
const int SCREEN_BPP = 32;
float WIDTH_RATIO;
float HEIGHT_RATIO;
const bool FULLSCREEN = false;

//music
bool blnMusic = true;
int intCurrentTrack = 0;

//debug variables
string debug_text[7];

//sdl stuff
SDL_Event event;
SDL_Rect clips[5];
SDL_Window *window = NULL;
SDL_Rect rctDisplay;
SDL_Renderer *renderer = NULL;

TTF_Font *font = NULL;
TTF_Font *fontMMButtons = NULL;

SDL_Color textColor = { 255, 255, 255 };

//The music that will be played
Mix_Music *music[3] = {NULL, NULL, NULL};
//The sound effects that will be used
//Mix_Chunk *cannon = NULL;

//structs
struct texture{
	SDL_Texture *txr;
	int w;
	int h;
	int alphaMod;
}
txrDialogBox,
txrElf,
txrButton,
txrStoryBG[20],
txrStoryBGEffects[20],
txrBattleTiles[1],
txrParticle,
txrAnswer[3],
txrPortrait[20],
txrMapBuffer;

struct story_hold{
	//story specific
	int StoryBG;
	int leftCharacter;
	int rightCharacter;
	float leftCharacterAlpha;
	float rightCharacterAlpha;
	int leftCharacterLast;
	int rightCharacterLast;
	bool showNametag;
	bool question;
	int currentFlag;
	bool useParticles;
	bool showDialogue;
	bool menu;
	int menuSelection;
	int choiceSelection;
}
story_set;

struct map_making{
	bool showGUI;
	int btnSelected;
	SDL_Rect tilesSelected;
	float fltMapZoomFactor;
	SDL_Rect brushTileClip;
	int selBrushTile;
	int brushSize;
	int brushType;
}
making_map;

struct sprite{
	bool visible;
	int id;
	float x;
	float y;
	int xTiled;
	int yTiled;
	int w;
	int h;
	float xVel;
	float yVel;
	float xAcc;
	float yAcc;
}
particle[10], 
particleMM[100], 
sprScreen,
sprTileset;

struct textEntity{
	string text;
	sprite entity;
	texture texture;
	bool showBG;
	bool clickable;
	bool visible;
}
nametag, 				  
btnBattle[1], 
txtBattle[2], 
txtMessage[4],
txtFPS, 
txtBGPlaceholder,
btnMapMaking[6],
txtCredits[4],
//0-3, bounds changers
txtMapMaking[3]
//0-2, map bounds
;

string strCreditTitles[20];

struct battles{
	string name;
	int enemy[4];
	int enemy_h[4];
	int background;
}
battle;

struct equippable{
	string name;
	string description;
	int slot;
	int id; //int(id/50)=item slot #
	int damage;
	int armor;
	int health;
	int agility;
	int strength;
	int intelligence;
};

struct spell{
	string name;
	string description;
	int id;//items start from 0, spells start from 100
	int heal;
	int damage;
	int fortify;
	int sharpen;
	int targets;
}
spells[100], 
items[100];

struct character{
	char name[25];
	spell spells[4];
	int id;
	int health;
	int mana;
	int item_slot[10];
	int armor;
	int agility;
	int strength;
	int intelligence;
	int cdp;
	sprite walk;
	sprite battle;
};

struct inv_slot{//inventory slot
	string spell_name;
	int amount;
};

struct menu{
	int id;
	sprite entity;
	int itemAmount;
	textEntity *item;
	int itemSelected;
	int scrollAmount;
	int scrollAmountMax;
	bool scrollable;
	bool scRollover;
	bool scrolling;
	int scrollBarSize;
	int spacing;
	bool moveable;
	bool moveOver;
	bool moving;
	bool showMenuBG;
	bool showBtnBG;
	textEntity title;
	bool showTitleBG;
	bool showTitle;
	bool visible;
}
menuMain,
menuStory,
menuStoryChoices;

struct map{
	//need to save
	int xTiles;
	int yTiles;
	int *tile;
	//don't need to save
	string name;
	sprite entity;
}
bMap[1];

struct story_frame{
	//show_story(string text0, string text1, string text2, string text3, 
	//string nt, bool n, bool sd, int intBgPic, int l_c, int r_c, bool particles)
	string text[4];
	string nametag;
	bool showNametag;
	bool showDialog;
	int BGPicture;
	int leftCharacter;
	int rightcharacter;
	bool particles;
	bool presentChoices;
	int currentFlag;
	string choice[3];
}
scnEnterSchool[13],
scnBeHuebert[6],
scnBeMe[12],
scnBeNoOne[9];

struct story_progression{
	int flag[2];
	int scene;
	int frame;
};

struct player{
	character ael;
	story_progression story;
	int wMap;
	int bMap;
	states state;
}
the_player;

//functions

void save_save()
{
	ofstream save("saves/save.her", ios::binary);
	save.write((char *)&blnNoSaveFile, sizeof(blnNoSaveFile));
	save.write((char *)&the_player.story.frame, sizeof(the_player.story.frame));
	save.write((char *)&the_player.story.flag, sizeof(the_player.story.flag));
	save.write((char *)&the_player.story.scene, sizeof(the_player.story.scene));
	save.close();

	menuMain.item[1].clickable = true;
	menuStory.item[2].clickable = true;
}

void load_save()
{
	ifstream load( "saves/save.her", ios::binary );
	load.read((char *)&blnNoSaveFile, sizeof(blnNoSaveFile));
	load.read((char *)&the_player.story.frame, sizeof(the_player.story.frame));
	load.read((char *)&the_player.story.flag, sizeof(the_player.story.flag));
	load.read((char *)&the_player.story.scene, sizeof(the_player.story.scene));
	load.close();
}

void wipe_save(){
	//player wipe;
	//the_player = wipe;
	the_player.state = story_screen;
	the_player.story.frame = 0;
	the_player.story.scene = 0;
	the_player.ael.agility = 3;
	//blnNoSaveFile = false;
	//save_save();
}

SDL_Surface *load_image(string filename)
{
    //The image that's loaded
    SDL_Surface* loadedImage = NULL;

    //The optimized image that will be used
    SDL_Surface* optimizedImage = NULL;

    //Load the image using SDL_image
    loadedImage = IMG_Load( filename.c_str() );

    //If the image loaded
  //  if( loadedImage != NULL )
  //  {
  //      //Create an optimized image
  //      //optimizedImage = SDL_DisplayFormatAlpha( loadedImage );
  //      optimizedImage = ( loadedImage, screen->format, NULL );
		////optimizedImage = SDL_ConvertSurface( loadedImage, screen->format, NULL );

  //      //Free the old image
  //      SDL_FreeSurface( loadedImage );
  //      
  //      //If the image was optimized just fine
  //      /*if( optimizedImage != NULL )
  //      {
  //          //Map the color key
  //          Uint32 colorkey = SDL_MapRGB( optimizedImage->format, 255, 56, 255 );
  //          
  //          //Set all pixels of color R 0, G 0xFF, B 0xFF to be transparent
  //          SDL_SetColorKey( optimizedImage, SDL_SRCCOLORKEY, colorkey );
  //      }*/
  //  }

    //Return the optimized image
    //return optimizedImage;
	return loadedImage;
}

texture loadTexture(string path)
{
	//The final texture
	texture newTexture;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL )
	{
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
	}
	else
	{
		newTexture.w = loadedSurface->w;
		newTexture.h = loadedSurface->h;
		//Create texture from surface pixels
        newTexture.txr = SDL_CreateTextureFromSurface( renderer, loadedSurface );
		if( newTexture.txr == NULL )
		{
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}
		else SDL_SetTextureBlendMode(newTexture.txr, SDL_BLENDMODE_BLEND);

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	return newTexture;
}

texture txrRender_text(string textureText, SDL_Color textColor, TTF_Font* ft)
{
	texture tempT;

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Blended(ft, textureText.c_str(), textColor);
	//if( textSurface == NULL )
	//{
	//	printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
	//}
	//else
	//{
		//Create texture from surface pixels
        tempT.txr = SDL_CreateTextureFromSurface(renderer, textSurface);
		if( tempT.txr == NULL )
		{
			//cout << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << endl;
			tempT.w = 0;
			tempT.h = 0;
		}
		else
		{
			//Get image dimensions
			tempT.w = textSurface->w;
			tempT.h = textSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface( textSurface );
	//}
	
	//Return success
	return tempT;
}

void apply_surface(int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip = NULL)
{
    ////Rectangle to hold the offsets
    SDL_Rect offset;

    ////Get offsets
    offset.x = x;
    offset.y = y;

    //Blit the surface
    SDL_BlitSurface( source, clip, destination, &offset );
}

void apply_texture(int x, int y, int w, int h, SDL_Texture* source, SDL_Renderer* dest, SDL_Rect* clip = NULL)
{
    ////Rectangle to hold the offsets
    SDL_Rect offset;

    ////Get offsets
    offset.x = x;
    offset.y = y;
	offset.w = w;
    offset.h = h;

    //Render texture to screen
	SDL_RenderCopy( dest, source, clip, &offset );
}

bool init()
{
    //Initialize all SDL subsystems
    if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
    {
        return false;
    }

	SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" );

    //Set up the screen
    if (!FULLSCREEN) window = SDL_CreateWindow("KernQuest: Gakkou Desu Ne",
                          SDL_WINDOWPOS_UNDEFINED,
                          SDL_WINDOWPOS_UNDEFINED,
                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (FULLSCREEN) window = SDL_CreateWindow("KernQuest: Gakkou Desu Ne",
                          SDL_WINDOWPOS_UNDEFINED,
                          SDL_WINDOWPOS_UNDEFINED,
                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN_DESKTOP);

	SDL_GetDisplayBounds(0, &rctDisplay);
	WIDTH_RATIO = (float)SCREEN_WIDTH/(float)rctDisplay.w;
	HEIGHT_RATIO = (float)SCREEN_HEIGHT/(float)rctDisplay.h;

	renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_PRESENTVSYNC  );
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	//SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

	IMG_Init( IMG_INIT_PNG );
	//screen = SDL_GetWindowSurface( window );

	//Initialize SDL_ttf
    if( TTF_Init() == -1 )
    {
        return false;    
    }

	//Open the font
    font = TTF_OpenFont( "cour.ttf", 18 );
	fontMMButtons = TTF_OpenFont( "cour.ttf", 32 );

	srand (time(NULL));

	if ( blnMusic ){
    	if( Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 4096 ) == -1 )
	    {
	        return false;    
	    }
    }

    return true;
}

bool load_files()
{
	//muh imgs
	/*txrBattleTiles[0] = loadTexture( "img/tiles.png" );
	txrElf = loadTexture( "img/Rogue Elf Sprite.png" );
	txrPortrait[1] = loadTexture( "img/ael proto.png" );
	txrPortrait[2] = loadTexture( "img/ael pose 1.png" );
	txrPortrait[3] = loadTexture( "img/Kern Portrait dragon.png" );
	txrPortrait[4] = loadTexture( "img/Lilliana Portait.png" );*/
	txrPortrait[5] = loadTexture( "img/ael.png" );
	txrPortrait[6] = loadTexture( "img/kern.png" );
	txrPortrait[7] = loadTexture( "img/rosaline.png" );
	/*txrStoryBG[1] = loadTexture( "img/Forest Background.png" );
	txrStoryBG[2] = loadTexture( "img/baby_kern_forest.png" );
	txrStoryBG[3] = loadTexture( "img/Abbey alter no light.png" );
	txrStoryBG[4] = loadTexture( "img/Abbey Inside Background.png" );*/
	txrStoryBG[5] = loadTexture( "img/nihoni.png" );
	/*txrStoryBGEffects[0] = loadTexture( "img/Abbey alter light.png" );*/
	txrDialogBox = loadTexture( "img/dialog box.png" );
	txrParticle = loadTexture( "img/particle.png" );
	txrButton = loadTexture( "img/button.png" );

	//muh musc
	if ( blnMusic ){
	    //Load the music
	    music[0] = Mix_LoadMUS( "mus/dd&d.ogg" );
		music[2] = Mix_LoadMUS( "mus/errybody.ogg" );
		music[3] = Mix_LoadMUS( "mus/chugar.ogg" );
		music[1] = Mix_LoadMUS( "mus/last_whisper.ogg" );
		music[4] = Mix_LoadMUS( "mus/hl3_confirmed.ogg" );
	    //If there was a problem loading the music
	    /*if( music == NULL )
	    {
	        return false;    
	    }*/
	    //cannon = Mix_LoadWAV( "mus/gunsounds.wav" );
	}

    //If everything loaded fine
    return true;    
}

void clean_up()
{
    //Free the surface
	SDL_DestroyRenderer( renderer );
	SDL_DestroyWindow( window );
	renderer = NULL;
	window = NULL;
    //SDL_FreeSurface( title );
	//title = NULL;

	//Close the font
    TTF_CloseFont( font );
	
    //Quit SDL_ttf
    TTF_Quit();
	//Quit SDL_IMG
	IMG_Quit();

	if ( blnMusic ){
    	//Stop the music
        Mix_HaltMusic();
    	
	    //Free the music
	    Mix_FreeMusic( music[0] );
	}
	//Quit SDL_mixer
    Mix_CloseAudio();

    //Quit SDL
    SDL_Quit();
}

int get_difference(int a, int b){
	if (a < b) return b - a;
	else if (a > b) return a - b;
	else return 0;
}

void get_userinput(){
	intMouseWheelY = 0;

	while( SDL_PollEvent( &event ) ){
		//If the user has Xed out the window
        if( event.type == SDL_QUIT ){
            //Quit the program
            quit = true;
        }
		if( event.type == SDL_KEYDOWN ){
			switch( event.key.keysym.sym ){
				case SDLK_ESCAPE: 
					blnEscUp = true;
                    break;
				case SDLK_TAB: 
					blnTabUp = true;
                    break;
				case SDLK_w: 
					blnWDown = true;
                    break;
				case SDLK_a: 
					blnADown = true;
                    break;
				case SDLK_s: 
					blnSDown = true;
                    break;
				case SDLK_d: 
					blnDDown = true;
                    break;
			}
		}
		if( event.type == SDL_KEYUP ){
			switch( event.key.keysym.sym ){
				case SDLK_BACKQUOTE: 
					if (the_player.state == story_screen) the_player.state = text_debug_screen;
                    break;
				case SDLK_b: 
					b_up = true;
                    break;
				case SDLK_w: 
					blnWDown = false;
					blnWUp = true;
                    break;
				case SDLK_a: 
					blnADown = false;
					blnAUp = true;
                    break;
				case SDLK_s: 
					blnSDown = false;
					blnSUp = true;
                    break;
				case SDLK_d: 
					blnDDown = false;
					blnDUp = true;
                    break;
			}
		}
		if( event.type == SDL_MOUSEWHEEL ){
			intMouseWheelY = event.wheel.y;
		}
		if( event.type == SDL_MOUSEBUTTONDOWN ){
	        //If the left mouse button was pressed
	        if( event.button.button == SDL_BUTTON_LEFT ){
	        	lb_down = true;
				lb_down_init = true;
	        }
			if( event.button.button == SDL_BUTTON_MIDDLE ){
	        	blnMouseWheelClickDown = true;
	        }
	    }
		if( event.type == SDL_MOUSEBUTTONUP ){
	        //If the left mouse button was pressed
	        if( event.button.button == SDL_BUTTON_LEFT ){
	        	lb_down = false;
	        	lb_up = true;
	        }
			if( event.button.button == SDL_BUTTON_MIDDLE ){
	        	blnMouseWheelClickDown = false;
	        }
			if( event.button.button == SDL_BUTTON_RIGHT ){
	        	rb_down = false;
	        	rb_up = true;
	        }
	    }
	}
	
	if (FULLSCREEN) {
		SDL_GetMouseState(&delta_mouse_x, &delta_mouse_y);
		delta_mouse_x = delta_mouse_x*WIDTH_RATIO;
		delta_mouse_y = delta_mouse_y*HEIGHT_RATIO;

		delta_mouse_x = delta_mouse_x - mouse_x;
		delta_mouse_y = delta_mouse_y - mouse_y;
		SDL_GetMouseState(&mouse_x, &mouse_y);
		mouse_x = mouse_x*WIDTH_RATIO;
		mouse_y = mouse_y*HEIGHT_RATIO;
		
	}
	else{
		SDL_GetMouseState(&delta_mouse_x, &delta_mouse_y);
		delta_mouse_x = delta_mouse_x - mouse_x;
		delta_mouse_y = delta_mouse_y - mouse_y;
		SDL_GetMouseState(&mouse_x, &mouse_y);
	}
}

bool in_bounds(float x, float y, float a, float b, float w, float h){
	if (x >= a && x <= a + w && y >= b && y <= b + h) return true;
	else return false;
}

bool rect_collision(float x, float y, float w, float h, float a, float b, float c, float d){
	//If any of the sides from A are outside of B
    if(y + h <= b) return false;
    else if(y >= b + d) return false;
    else if(x + w <= a) return false;
    else if(x >= a + c) return false;

    //If none of the sides from A are outside B
    else return true;
}

bool sprite_collision(sprite s1, sprite s2){
	return rect_collision(s1.x, s1.y, s1.w, s1.h, s2.x, s2.y, s2.w, s2.h);
}

bool sprite_screen_collision(sprite s1){
	return rect_collision(s1.x, s1.y, s1.w, s1.h, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

bool mouse_in_sprite(sprite sp, int paddingX, int paddingY){
	if (in_bounds(float(mouse_x), float(mouse_y), sp.x - paddingX, sp.y - paddingY, 
												  sp.w + 2*paddingX, sp.h + 2*paddingY)){
		return true;
	}
	else return false;
}

void update_timers(){
	dsdltimer = SDL_GetTicks() - sdltimer;
	sdltimer = SDL_GetTicks();

	if(dsdltimer < 10)
    {
        //Sleep the remaining frame time
        SDL_Delay(10 - dsdltimer);
		dsdltimer = 10;
    }

	//joypad selection timers
	if (!sel_change_ok_x) sel_timer_x += dsdltimer;
	if (sel_timer_x >= 200){
		sel_timer_x = 0;
		sel_change_ok_x = true;
	}
	if (!sel_change_ok_y) sel_timer_y += dsdltimer;
	if (sel_timer_y >= 200){
		sel_timer_y = 0;
		sel_change_ok_y = true;
	}

	tmrRain += dsdltimer;
	if (tmrRain >= int((SCREEN_HEIGHT/fltRainSpeed)/(sizeof(particleMM)/sizeof(sprite)))){
		tmrRain -= int((SCREEN_HEIGHT/fltRainSpeed)/(sizeof(particleMM)/sizeof(sprite)));
		blnMakeRain = true;
	}
}

void create_particle(int x, int y, float xv, float yv, float xa, float ya, int s, sprite p[]){
	p[s].visible = true;
	p[s].x = x;
	p[s].y = y;
	p[s].xAcc = xa;
	p[s].yAcc = ya;
	p[s].xVel = xv;
	p[s].yVel = yv;
	p[s].w = 8;
	p[s].h = 8;
}

void kill_particle(int s, sprite p[]){
	p[s].visible = false;
	p[s].x = 0;
	p[s].y = 0;
	p[s].xAcc = 0;
	p[s].yAcc = 0;
	p[s].xVel = 0;
	p[s].yVel = 0;
	p[s].w = 0;
	p[s].h = 0;
}

void move_particle_fly(sprite p[], int s, bool d){
	//change x acceleration
	//p[s].xAcc += ((fltFlyJerk)*(rand() % 3)) - fltFlyJerk;
	//if (p[s].xAcc > fltFlyAcc) p[s].xAcc = fltFlyAcc;
	//else if (p[s].xAcc < -fltFlyAcc) p[s].xAcc = -fltFlyAcc;
	////change y acceleration
	//p[s].yAcc += ((fltFlyJerk)*(rand() % 3)) - fltFlyJerk;
	//if (p[s].yAcc > fltFlyAcc) p[s].yAcc = fltFlyAcc;
	//else if (p[s].yAcc < -fltFlyAcc) p[s].yAcc = -fltFlyAcc;

	//change x acceleration
	p[s].xAcc = ((fltFlyAcc)*(rand() % 3)) - fltFlyAcc;
	//change y acceleration
	p[s].yAcc = ((fltFlyAcc)*(rand() % 3)) - fltFlyAcc;

	//change x velocity
	p[s].xVel += p[s].xAcc*dsdltimer;
	if (p[s].xVel > fltFlySpeed) p[s].xVel = fltFlySpeed;
	else if (p[s].xVel < -fltFlySpeed) p[s].xVel = -fltFlySpeed;
	//change y velocity
	p[s].yVel += p[s].yAcc*dsdltimer;
	if (p[s].yVel > fltFlySpeed) p[s].yVel = fltFlySpeed;
	else if (p[s].yVel < -fltFlySpeed) p[s].yVel = -fltFlySpeed;
	//change position
	p[s].x += p[s].xVel*dsdltimer;
	p[s].y += p[s].yVel*dsdltimer;
	
	if (!sprite_screen_collision(p[s])){
		if (d){
			if (p[s].x > SCREEN_WIDTH || p[s].x < 0){
				p[s].xVel = -p[s].xVel;
				p[s].xAcc = -p[s].xAcc;
			}
			if (p[s].y > SCREEN_HEIGHT || p[s].y < 0){
				p[s].yVel = -p[s].yVel;
				p[s].yAcc = -p[s].yAcc;
			}
		}
		else{
			kill_particle(s, p);
		}
	}
}

void move_particle_rain(sprite p[], int s, bool d){
	p[s].xAcc = 0;
	p[s].yAcc = .005;

	//p[s].xVel = .1;
	if (p[s].yVel + p[s].yAcc*dsdltimer <= fltRainSpeed) p[s].yVel += p[s].yAcc*dsdltimer;
	else p[s].yVel = fltRainSpeed;
	
	p[s].x += p[s].xVel*dsdltimer;
	p[s].y += p[s].yVel*dsdltimer;
	
	if (!sprite_screen_collision(p[s])){
		if (d){
			if (p[s].x > SCREEN_WIDTH || p[s].x < 0){
				p[s].xVel = -p[s].xVel;
				p[s].xAcc = -p[s].xAcc;
			}
			if (p[s].y > SCREEN_HEIGHT){
				p[s].yVel = -p[s].yVel;
				p[s].y = SCREEN_HEIGHT;
				//p[s].yAcc = -p[s].yAcc;
			}
			if (p[s].y < 0){
				p[s].yVel = -p[s].yVel;
				p[s].y = 0;
				//p[s].yAcc = -p[s].yAcc;
			}
		}
		else{
			kill_particle(s, p);
		}
	}
}

void set_textEntity_text(textEntity &te, string text, int f){
	te.text = text;
	/*switch (f){
		case 0:
			SDL_FreeSurface(te.txr);
			te.txr = TTF_RenderText_Blended(font, text.c_str(), textColor);
			break;
		case 1:
			SDL_FreeSurface(te.txr);
			te.txr = TTF_RenderText_Blended(fontMMButtons, text.c_str(), textColor);
			break;
	}*/

	SDL_DestroyTexture(te.texture.txr);
	te.texture = txrRender_text(text, textColor, font);
	
	te.entity.w = te.texture.w;
	te.entity.h = te.texture.h;
}

void center_sprite(sprite &soda, sprite relation, bool xAxis, bool yAxis){
	if (xAxis) soda.x = (relation.x + relation.w/2) - (soda.w/2);
	if (yAxis) soda.y = (relation.y + relation.h/2) - (soda.h/2);
}

textEntity create_textEntity(string text, int f, int x, int y, bool sb, bool v){
	textEntity teTemp;

	
	teTemp.entity.w = 0;
	teTemp.entity.h = 0;
	teTemp.texture.txr = NULL;
	teTemp.showBG = sb;
	teTemp.visible = v;

	set_textEntity_text(teTemp, text, f);

	teTemp.entity.x = x;
	teTemp.entity.y = y;

	return teTemp;
}

menu create_menu(string txtTitle, int x, int y, int w, int h, int items, bool v){
	menu menuTemp;

	menuTemp.entity.x = x;
	menuTemp.entity.y = y;
	menuTemp.entity.w = w;
	menuTemp.entity.h = h;
	menuTemp.showMenuBG = true;
	menuTemp.visible = v;
	menuTemp.spacing = 5;
	menuTemp.itemAmount = items;
	menuTemp.item = new textEntity[items];
	menuTemp.scrollAmount = 0;
	menuTemp.scrolling = false;
	menuTemp.moveable = false;
	menuTemp.moveOver = false;
	menuTemp.moving = false;

	//check if the menu should scroll
	if (menuTemp.itemAmount*(30 + menuTemp.spacing) > menuTemp.entity.h){
		menuTemp.scrollable = true;
		
		if (menuTemp.entity.h <= 40){
			menuTemp.scrollBarSize = 10;
			menuTemp.scrollAmountMax = 10;
		}
		else{
			menuTemp.scrollBarSize = menuTemp.entity.h - 30 - menuTemp.itemAmount*5;
			if (menuTemp.scrollBarSize < 10) menuTemp.scrollBarSize = 10;
			menuTemp.scrollAmountMax = menuTemp.entity.h - 20 - menuTemp.scrollBarSize;
		}
	}
	else menuTemp.scrollable = false;

	menuTemp.title = create_textEntity(txtTitle, 0, 0, 0, true, true);
	menuTemp.title.entity.x = menuTemp.entity.x + (menuTemp.entity.w/2) - (menuTemp.title.entity.w/2);
	menuTemp.title.entity.y = menuTemp.entity.y - 25 - menuTemp.spacing;

	return menuTemp;
}

void align_menu_items(menu me){
	/*me.title.entity.x = me.entity.x + (me.entity.w/2) - (me.title.entity.w/2);
	me.title.entity.y = me.entity.y - 25 - me.spacing;*/

	for (int i = 0; i < me.itemAmount; i++){
		me.item[i].entity.x = me.entity.x + (me.entity.w/2) - (me.item[i].entity.w/2);
		me.item[i].entity.y = me.entity.y + 10 + i*(30 + me.spacing);
	}
}

bool item_selected(menu me, int item){
	return me.itemSelected == item && 
		mouse_in_sprite(me.item[item].entity, (me.entity.w - 10 - me.item[item].entity.w)/2, intDefaultBtnPaddingY);
}

void init_battle(int map){
	the_player.bMap = map;
	the_player.state = battle_screen;
	//set map position
	bMap[map].entity.x = (SCREEN_WIDTH/2) - (the_player.ael.battle.xTiled * 32) - (txrElf.w/2);
	bMap[map].entity.y = (SCREEN_HEIGHT/2) - (the_player.ael.battle.yTiled * 32) + (txrElf.h/4);
	//set CDPoints
	the_player.ael.cdp = the_player.ael.agility;
	set_textEntity_text(txtBattle[1], _itoa(the_player.ael.cdp, buffer, 10), 0);
}

void present_choices(string text0, string text1, string text2, int flag){
	story_set.question = true;
	menuStoryChoices.visible = true;
	story_set.currentFlag = flag;

	set_textEntity_text(menuStoryChoices.item[0], text0, 0);
	set_textEntity_text(menuStoryChoices.item[1], text1, 0);
	set_textEntity_text(menuStoryChoices.item[2], text2, 0);
	/*SDL_DestroyTexture(btnChoices[0].texture.txr);
	btnChoices[0].texture = txrRender_text(text0, textColor, font);
	btnChoices[0].text = text0;
	SDL_DestroyTexture(btnChoices[1].texture.txr);
	btnChoices[1].texture = txrRender_text(text1, textColor, font);
	btnChoices[1].text = text1;
	SDL_DestroyTexture(btnChoices[2].texture.txr);
	btnChoices[2].texture = txrRender_text(text2, textColor, font);
	btnChoices[2].text = text2;*/

	align_menu_items(menuStoryChoices);
}

map create_map(){
	map newMap;

	cout << "Please enter the map's name:" << endl; 
	getline(cin, newMap.name);
	//cin >> newMap.name;
	SDL_Delay(50);
	cout << "Please enter the map's max X:" << endl;
	cin >> newMap.xTiles;
	SDL_Delay(50);
	cout << "Please enter the map's max Y:" << endl;
	cin >> newMap.yTiles;
	cin.ignore (std::numeric_limits<std::streamsize>::max(), '\n');
	cout << "--------------------------------------" << endl;
	cout << endl;

	newMap.tile = new int [newMap.xTiles*newMap.yTiles];
	for (int i = 0; i < newMap.xTiles*newMap.yTiles; i++){
		newMap.tile[i] = 0;
	}

	newMap.entity.x = 0;
	newMap.entity.y = 0;
	newMap.entity.w = newMap.xTiles*32;
	newMap.entity.h = newMap.yTiles*32;
	
	txrMapBuffer.txr = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, newMap.xTiles*32, newMap.yTiles*32);
	SDL_SetRenderTarget( renderer, txrMapBuffer.txr );
	for (int i = 0; i < bMap[0].xTiles; i++){
		for (int ii = 0; ii < bMap[0].yTiles; ii++){
			making_map.brushTileClip.x = 32*(newMap.tile[newMap.xTiles*ii + i]%20);
			making_map.brushTileClip.y = 32*(newMap.tile[newMap.xTiles*ii + i]/20);
			SDL_SetTextureColorMod(txrBattleTiles[0].txr, 255, 255, 255);
			apply_texture(i*32, ii*32, 32, 32, txrBattleTiles[0].txr, renderer, &making_map.brushTileClip);
			
			if (!making_map.showGUI && get_difference(i, int((mouse_x - newMap.entity.x)/(32*making_map.fltMapZoomFactor))) +
				get_difference(ii, int((mouse_y - newMap.entity.y)/(32*making_map.fltMapZoomFactor))) <
				making_map.brushSize)
			{
				SDL_Rect rctHoverTile = {32*i, 32*ii, 32, 32};
				SDL_RenderDrawRect(renderer, &rctHoverTile);
			}
		}
	}
	SDL_SetRenderTarget( renderer, NULL );

	return newMap;
}

void print_map_properties(map mup){
	cout << "Map name:" << endl;
	cout << mup.name << endl;
	cout << "Map Dimensions (X,Y):" << endl;
	cout << "(" << mup.xTiles << "," << mup.yTiles << ")" << endl;
	cout << "--------------------------------------" << endl << endl;
}

void init_map_making(){
	the_player.state = map_making;
	making_map.fltMapZoomFactor = 1.0;
	making_map.brushSize = 3;

	making_map.brushTileClip.x = 0;
	making_map.brushTileClip.y = 0;
	making_map.brushTileClip.w = 32;
	making_map.brushTileClip.h = 32;

	bMap[0] = create_map();

	set_textEntity_text(txtMapMaking[0], _itoa(making_map.brushSize, buffer, 10), 0);
}
 
void save_map(){
	string nameTemp = "maps/" + bMap[0].name + ".nmp";
	ofstream savingMap(nameTemp.c_str(), ios::binary);
	savingMap.write((char *)&bMap[0].xTiles, sizeof(bMap[0].xTiles));
	savingMap.write((char *)&bMap[0].yTiles, sizeof(bMap[0].yTiles));
	
	for (int i = 0; i < bMap[0].xTiles; i++){
		for (int ii = 0; ii < bMap[0].yTiles; ii++){
			savingMap.write((char *)&bMap[0].tile[bMap[0].xTiles*ii + i], sizeof(bMap[0].tile[bMap[0].xTiles*ii + i]));
		}
	}
	
	cout << "Map saved." << endl;
	cout << "--------------------------------------" << endl << endl;
	savingMap.close();
}

void load_map(){
	cout << "Please enter the map's name:" << endl;
	getline(cin, bMap[0].name);
	string nameTemp = "maps/" + bMap[0].name + ".nmp";
	ifstream loadingMap( nameTemp.c_str(), ios::binary );
	if (loadingMap){
		loadingMap.read((char *)&bMap[0].xTiles, sizeof(bMap[0].xTiles));
		loadingMap.read((char *)&bMap[0].yTiles, sizeof(bMap[0].yTiles));
		delete bMap[0].tile;
		bMap[0].tile = new int [bMap[0].xTiles*bMap[0].yTiles];
		for (int i = 0; i < bMap[0].xTiles*bMap[0].yTiles; i++){
			bMap[0].tile[i] = 0;
		}
		for (int i = 0; i < bMap[0].xTiles; i++){
			for (int ii = 0; ii < bMap[0].yTiles; ii++){
				loadingMap.read((char *)&bMap[0].tile[bMap[0].xTiles*ii + i], sizeof(bMap[0].tile[bMap[0].xTiles*ii + i]));
			}
		}
		bMap[0].entity.x = 0;
		bMap[0].entity.y = 0;
		bMap[0].entity.w = bMap[0].xTiles*32;
		bMap[0].entity.h = bMap[0].yTiles*32;

		txrMapBuffer.txr = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, bMap[0].xTiles*32, bMap[0].yTiles*32);
		SDL_SetRenderTarget( renderer, txrMapBuffer.txr );
		for (int i = 0; i < bMap[0].xTiles; i++){
			for (int ii = 0; ii < bMap[0].yTiles; ii++){
				making_map.brushTileClip.x = 32*(bMap[0].tile[bMap[0].xTiles*ii + i]%20);
				making_map.brushTileClip.y = 32*(bMap[0].tile[bMap[0].xTiles*ii + i]/20);
				SDL_SetTextureColorMod(txrBattleTiles[0].txr, 255, 255, 255);
				apply_texture(i*32, ii*32, 32, 32, txrBattleTiles[0].txr, renderer, &making_map.brushTileClip);
			
				if (!making_map.showGUI && get_difference(i, int((mouse_x - bMap[0].entity.x)/(32*making_map.fltMapZoomFactor))) +
					get_difference(ii, int((mouse_y - bMap[0].entity.y)/(32*making_map.fltMapZoomFactor))) <
					making_map.brushSize)
				{
					SDL_Rect rctHoverTile = {32*i, 32*ii, 32, 32};
					SDL_RenderDrawRect(renderer, &rctHoverTile);
				}
			}
		}
		SDL_SetRenderTarget( renderer, NULL );

		cout << "Map loaded." << endl;
		cout << "--------------------------------------" << endl << endl;
	}
	else{
		cout << "Map not found!" << endl;
	}

	loadingMap.close();
	//cin.ignore (std::numeric_limits<std::streamsize>::max(), '\n');
}

story_frame create_story_frame(string text0, string text1, string text2, string text3, string nt, bool n, bool sd, int intBgPic, int l_c, int r_c, bool particles, bool pc, int flaggy, string choice0, string choice1, string choice2){
	story_frame frmTemp;

	frmTemp.BGPicture = intBgPic;

	frmTemp.leftCharacter = l_c;
	frmTemp.rightcharacter = r_c;

	frmTemp.text[0] = text0;
	frmTemp.text[1] = text1;
	frmTemp.text[2] = text2;
	frmTemp.text[3] = text3;

	frmTemp.particles = particles;

	frmTemp.showDialog = sd;

	frmTemp.nametag = nt;
	frmTemp.showNametag = n;
	frmTemp.presentChoices = pc;
	frmTemp.currentFlag = flaggy;
	frmTemp.choice[0] = choice0;
	frmTemp.choice[1] = choice1;
	frmTemp.choice[2] = choice2;

	return frmTemp;
}

void init_credits(){
	the_player.state = credits;

	for (int i = 0; i < sizeof(txtCredits)/sizeof(textEntity); i++){
		txtCredits[i] = create_textEntity(strCreditTitles[rand()%10], 0, 0, SCREEN_HEIGHT + 100*i, false, true);
		center_sprite(txtCredits[i].entity, sprScreen, true, false);
	}

	if ( blnMusic ){
		Mix_HaltMusic();
		if( Mix_PlayMusic( music[4], 1 ) == -1 )
		{
			quit = true;
		}  
	    
		Mix_VolumeMusic(64);
	}
	
}

//update functions
void update_particles(sprite p[], int n, bool bounce, effect e){
	if (p[n].visible){
		switch (e){
			case fly:
				move_particle_fly(p, n, bounce);
				break;
			case rain:
				move_particle_rain(p, n, bounce);
				break;
		}
	} 
	if (!p[n].visible){
		switch (e){
			case fly:
				create_particle(rand() % SCREEN_WIDTH - 4, rand() % SCREEN_HEIGHT - 4, 
						.1*(rand()%11) - .5, .1*(rand()%11) - .5, 
						.001*(rand()%11) - .005, .001*(rand()%11) - .005, n, p);
				break;
			case rain:
				if (blnMakeRain){
					create_particle(rand()%SCREEN_WIDTH-4, -4, 
									0, fltRainSpeed,//-((fltRainSpeed/200)*(rand()%100)), 
									0, 0, n, p);
					blnMakeRain = false;
				}
				break;
		}
	}
}

void update_menu(menu &me){
	if (!lb_down && !lb_up){
		me.itemSelected = -1;
		if (me.visible){
			for (int i = 0; i < me.itemAmount; i++){
				if (mouse_in_sprite(me.item[i].entity, (me.entity.w - 10 - me.item[i].entity.w)/2, intDefaultBtnPaddingY)){
					me.itemSelected = i;
				}
				//cout << me.itemSelected << endl;
			}
		}
	}
	

	//scrolling
	if (me.scrollable){
		if (in_bounds(mouse_x, mouse_y, me.entity.x + me.entity.w, me.entity.y + me.scrollAmount + 10, 10, me.scrollBarSize)){
			me.scRollover = true;
		}
		else me.scRollover = false;
		if (me.scRollover && lb_down_init){
			me.scrolling = true;
			lb_down_init = false;
		}
		if (me.scrolling && lb_down){
			me.scrollAmount += delta_mouse_y;

			if (me.scrollAmount < 0) me.scrollAmount = 0;
			if (me.scrollAmount > me.scrollAmountMax) me.scrollAmount = me.scrollAmountMax;

			for (int i = 0; i < me.itemAmount; i++){
				me.item[i].entity.y = me.entity.y + 10 + i*(30 + me.spacing) - (me.itemAmount*(30 + me.spacing) - me.entity.h + 10)*(1.0*me.scrollAmount/me.scrollAmountMax);
			}
		}
		if (mouse_in_sprite(me.entity, 0, 0)){
			me.scrollAmount -= 4*intMouseWheelY;

			if (me.scrollAmount < 0) me.scrollAmount = 0;
			if (me.scrollAmount > me.scrollAmountMax) me.scrollAmount = me.scrollAmountMax;

			for (int i = 0; i < me.itemAmount; i++){
				me.item[i].entity.y = me.entity.y + 10 + i*(30 + me.spacing) - (me.itemAmount*(30 + me.spacing) - me.entity.h + 10)*(1.0*me.scrollAmount/me.scrollAmountMax);
			}
		}
		if (me.scrolling && lb_up){
			me.scrolling = false;
			lb_up = false;
		}
	}
	

	//moving
	if (me.moveable){
		if (mouse_in_sprite(me.title.entity, (me.entity.w - me.title.entity.w)/2, intDefaultBtnPaddingY)){
			me.moveOver = true;
		}
		else me.moveOver = false;
		if (me.moveOver && lb_down_init){
			me.moving = true;
			lb_down_init = false;
		}
		if (me.moving && lb_down){
			me.entity.x += delta_mouse_x;
			me.entity.y += delta_mouse_y;

			me.title.entity.y += delta_mouse_y;
			me.title.entity.x += delta_mouse_x;

			for (int i = 0; i < me.itemAmount; i++){
				me.item[i].entity.y += delta_mouse_y;
				me.item[i].entity.x += delta_mouse_x;
			}
		}
		if (me.moving && lb_up){
			me.moving = false;
			lb_up = false;
		}
	}
}

void update_text_debug(){
	getline(cin, debug_text[0]);
	set_textEntity_text(txtMessage[0], debug_text[0], 0);
	if (debug_text[0] == "QUIT") quit = true;
	if (debug_text[0] == "STOP") the_player.state = main_menu;
}

void update_state_walk(){
	////up
	//if (blnWDown){
	//	if (in_bounds(the_player.ael.walk.x, the_player.ael.walk.y - (dsdltimer*fltStandardVelocity), 0, 0, maps[0].x, maps[0].y)){
	//		the_player.ael.walk.y -= int(dsdltimer*fltStandardVelocity);
	//	}
	//	else the_player.ael.walk.y = 0;
	//}
	////left
	//if (blnADown){
	//	if (in_bounds(the_player.ael.walk.x - (dsdltimer*fltStandardVelocity), the_player.ael.walk.y, 0, 0, maps[0].x, maps[0].y)){
	//		the_player.ael.walk.x -= int(dsdltimer*fltStandardVelocity);
	//	}
	//	else the_player.ael.walk.x = 0;
	//}
	////down
	//if (blnSDown){
	//	if (in_bounds(the_player.ael.walk.x, the_player.ael.walk.y + (dsdltimer*fltStandardVelocity), 0, 0, maps[0].x, maps[0].y)){
	//		the_player.ael.walk.y += int(dsdltimer*fltStandardVelocity);
	//	}
	//	else the_player.ael.walk.y = maps[0].y;
	//}
	////right
	//if (blnDDown){
	//	if (in_bounds(the_player.ael.walk.x + (dsdltimer*fltStandardVelocity), the_player.ael.walk.y, 0, 0, maps[0].x, maps[0].y)){
	//		the_player.ael.walk.x += int(dsdltimer*fltStandardVelocity);
	//	}
	//	else the_player.ael.walk.x = maps[0].x;
	//}
}

void update_state_battle(){
	//muh paachikaaru
	for (int i = 0; i < sizeof(particle)/sizeof(sprite); i++){
		update_particles(particle, i, true, fly);
	}

	if (lb_up)
	{
		if (mouse_in_sprite(btnBattle[0].entity, intDefaultBtnPaddingX, intDefaultBtnPaddingY))
		{
			the_player.ael.cdp += the_player.ael.agility;
			set_textEntity_text(txtBattle[1], _itoa(the_player.ael.cdp, buffer, 10), 0);
		}
		else if (mouse_in_sprite(bMap[the_player.bMap].entity, 0, 0))
		{
			if (get_difference((mouse_x - bMap[the_player.bMap].entity.x)/32, the_player.ael.battle.xTiled) +
				get_difference((mouse_y - bMap[the_player.bMap].entity.y)/32, the_player.ael.battle.yTiled) <=
				the_player.ael.cdp)
			{
				the_player.ael.battle.xTiled = (mouse_x - bMap[the_player.bMap].entity.x)/32;
				the_player.ael.battle.yTiled = (mouse_y - bMap[the_player.bMap].entity.y)/32;
				bMap[0].entity.x = (SCREEN_WIDTH/2) - (the_player.ael.battle.xTiled * 32) - (txrElf.w/2);
				bMap[0].entity.y = (SCREEN_HEIGHT/2) - (the_player.ael.battle.yTiled * 32) + (txrElf.h/4);
				the_player.ael.cdp -= get_difference((mouse_x - bMap[the_player.bMap].entity.x)/32, the_player.ael.battle.xTiled) +
											get_difference((mouse_y - bMap[the_player.bMap].entity.y)/32, the_player.ael.battle.yTiled);
				set_textEntity_text(txtBattle[1], _itoa(the_player.ael.cdp, buffer, 10), 0);
			}
		}
		lb_up = false;
	}
	/*if (blnWUp)
	{
		if (the_player.ael.battle.y > 0)
		{
			the_player.ael.battle.y -= 1;
		}
		blnWUp = false;
	}
	else if (blnAUp)
	{
		if (the_player.ael.battle.x > 0)
		{
			the_player.ael.battle.x -= 1;
		}
		blnAUp = false;
	}
	else if (blnSUp)
	{
		if (the_player.ael.battle.y < bMap[the_player.bMap].y - 1)
		{
			the_player.ael.battle.y += 1;
		}
		blnSUp = false;
	}
	else if (blnDUp)
	{
		if (the_player.ael.battle.x < bMap[the_player.bMap].x - 1)
		{
			the_player.ael.battle.x += 1;
		}
		blnDUp = false;
	}*/
}

void show_story(story_frame sf){
	story_set.StoryBG = sf.BGPicture;

	story_set.leftCharacter = sf.leftCharacter;
	if (story_set.leftCharacter != story_set.leftCharacterLast) story_set.leftCharacterAlpha = 0;
	SDL_SetTextureAlphaMod(txrPortrait[story_set.leftCharacter].txr, story_set.leftCharacterAlpha);
	story_set.leftCharacterLast = sf.leftCharacter;
	story_set.rightCharacter = sf.rightcharacter;
	if (story_set.rightCharacter != story_set.rightCharacterLast) story_set.rightCharacterAlpha = 0;
	SDL_SetTextureAlphaMod(txrPortrait[story_set.rightCharacter].txr, story_set.rightCharacterAlpha);
	story_set.rightCharacterLast = sf.rightcharacter;

	the_player.state = story_screen;

	set_textEntity_text(txtMessage[0], sf.text[0], 0);
	set_textEntity_text(txtMessage[1], sf.text[1], 0);
	set_textEntity_text(txtMessage[2], sf.text[2], 0);
	set_textEntity_text(txtMessage[3], sf.text[3], 0);

	story_set.useParticles = sf.particles;

	story_set.showDialogue = sf.showDialog;

	//nametag
	set_textEntity_text(nametag, sf.nametag, 0);
	story_set.showNametag = sf.showNametag;

	if (sf.presentChoices){
		present_choices(sf.choice[0], sf.choice[1], sf.choice[2], sf.currentFlag);
	}
	else story_set.question = false;
} 

void update_scene0(){
	//				  //0123456789101112131415161718192021222324252627282930313233
	//switch (the_player.story.frame){
	//	case 0:
	//		set_textEntity_text(txtBGPlaceholder, "Black Scene", 0);
	//		show_story("It's a boy!", 
	//				   "", 
	//				   "",
	//				   "", 
	//				   "Female Voice", true, true, 0, 0, 2, false);
	//		break;
	//	case 1:
	//		show_story("Let's not...", 
	//				   "", 
	//				   "",
	//				   "", 
	//				   "Male Voice", true, true, 0, 3, 2, false);
	//		present_choices("some text", "some text1", "some text2");
	//		break;
	//	case 2:
	//		show_story("What?", 
	//				   "", 
	//				   "",
	//				   "", 
	//				   "Female Voice", true, true, 0, 3, 1, false);
	//		break;
	//				  //0123456789101112131415161718192021222324252627282930313233
	//	case 3:
	//		show_story("Let's not give him to the elders... those morbid bastards.", 
	//				   "", 
	//				   "",
	//				   "", 
	//				   "Male Voice", true, true, 0, 3, 1, false);
	//		break;
	//	case 4:
	//		show_story("But the law...", 
	//				   "", 
	//				   "",
	//				   "", 
	//				   "Female Voice", true, true, 0, 3, 1, false);
	//		break;
	//				  //0123456789101112131415161718192021222324252627282930313233
	//	case 5:
	//		show_story("The law, the law! Behold this child! He will be great, I", 
	//				   "know it. The elders will only limit him out of fear.", 
	//				   "",
	//				   "", 
	//				   "Male Voice", true, true, 0, 3, 1, false);
	//		break;
	//	case 6:
	//		show_story("You really enjoy ruffling their feathers don't you?", 
	//				   "", 
	//				   "",
	//				   "", 
	//				   "Female Voice", true, true, 0, 3, 2, false);
	//		break;
	//				  //0123456789101112131415161718192021222324252627282930313233
	//	case 7:
	//		show_story("See here: let us leave him near that monastery - hide him", 
	//				   "behind the berry bushes - with a stockpile of food.", 
	//				   "Unchecked he will definitely destroy that place and all",
	//				   "the humans within.", 
	//				   "Male Voice", true, true, 0, 3, 2, false);
	//		break;
	//	case 8:
	//		show_story("I suppose this could be interesting.", 
	//				   "", 
	//				   "",
	//				   "", 
	//				   "Female Voice", true, true, 0, 3, 1, false);
	//		break;
	//	case 9:
	//		the_player.story.frame = 0;
	//		the_player.story.scene = 1;
	//		set_textEntity_text(txtBGPlaceholder, "Abbey Alter, Linora's Aunt", 0);
	//		show_story("Dear Linidei, my sister has recently had a child. She", 
	//					"seems so happy... I've been thinking about it, and I would", 
	//					"like a child too. If it pleases you, may I be blessed with",
	//					"the opportunity to have a child of my own?", 
	//					"Linora's Aunt", true, true, 4, 0, 0, false);
	//		break;
	//}
}

void update_scene1(){
	//switch (the_player.story.flag[0]){
	//	case 0://Linidei
	//		switch (the_player.story.frame){
	//						  //0123456789101112131415161718192021222324252627282930313233
	//			case 0:
	//				set_textEntity_text(txtBGPlaceholder, "Abbey Alter, Linora's Aunt", 0);
	//				show_story("Dear Linidei, my sister has recently had a child. She", 
	//						   "seems so happy... I've been thinking about it, and I would", 
	//						   "like a child too. If it pleases you, may I be blessed with",
	//						   "the opportunity to have a child of my own?", 
	//						   "Linora's Aunt", true, true, 4, 0, 0, false);
	//				break;
	//			case 1:
	//				set_textEntity_text(txtBGPlaceholder, "I shall grant your earnest desire.", 0);
	//				show_story("", 
	//						   "", 
	//						   "",
	//						   "", 
	//						   "null", false, false, 0, 0, 0, false);
	//				break;
	//			case 2:
	//				set_textEntity_text(txtBGPlaceholder, "Early Morning Forest, Aunt Berry Picking", 0);
	//				show_story("Today will be...", 
	//						   "", 
	//						   "",
	//						   "", 
	//						   "Linora's Aunt", true, true, 1, 0, 0, false);
	//				break;
	//						  //0123456789101112131415161718192021222324252627282930313233
	//			case 3:
	//				show_story("Wahhhh!", 
	//						   "", 
	//						   "",
	//						   "", 
	//						   "???", true, true, 1, 0, 0, false);
	//				break;
	//			case 4:
	//				show_story("Is that a baby? My word, where is that coming from?", 
	//						   "", 
	//						   "",
	//						   "", 
	//						   "Linora's Aunt", true, true, 1, 0, 0, false);
	//				break;
	//						  //0123456789101112131415161718192021222324252627282930313233
	//			case 5:
	//				show_story("The forest outside the monastery echoed with the cries and", 
	//						   "sobs of a baby. The woman, huddled in her shall, made her", 
	//						   "way towards the noise without hesitation. Could this be",
	//						   "her prayer's answer already? Had Linidei bestowed a child", 
	//						   "null", false, true, 1, 0, 0, false);
	//				break;
	//			case 6:
	//				show_story("upon her so quickly?", 
	//						   "", 
	//						   "",
	//						   "", 
	//						   "null", false, true, 1, 0, 0, false);
	//				break;
	//						  //0123456789101112131415161718192021222324252627282930313233
	//			case 7:
	//				show_story("As she went around the corner of the monastery, the cries", 
	//						   "grew louder. Before she saw the baby she saw the food. It", 
	//						   "appeared as if someone had planted meat bushes behind the",
	//						   "berry bushes. A miniature haven of preserved meat", 
	//						   "null", false, true, 2, 0, 0, false);
	//				break;
	//			case 8:
	//				show_story("surrounded a bundle.", 
	//						   "", 
	//						   "",
	//						   "", 
	//						   "null", false, true, 2, 0, 0, false);
	//				break;
	//			case 9:
	//				show_story("The bundle shifted and the cries came forth once more. The", 
	//						   "woman immediately recognized the bundle as a baby and", 
	//						   "shuffled past the meat to comfort the child.",
	//						   "", 
	//						   "null", false, true, 2, 0, 0, false);
	//				break;
	//			case 10:
	//				show_story("She flinched away as she noticed the race of the child, he", 
	//						   "was a Tiefling. The horns and red skin couldn't be a joke.", 
	//						   "Someone had left this Tiefling here. The baby cried out",
	//						   "once more, and she realized she couldn't abandon this", 
	//						   "null", false, true, 2, 0, 0, false);
	//				break;
	//			case 11:
	//				show_story("child as well.", 
	//						   "", 
	//						   "",
	//						   "", 
	//						   "null", false, true, 2, 0, 0, false);
	//				break;
	//			case 12:
	//				show_story("I asked for a child and I have been blessed with this", 
	//						   "young Tiefling. I shall not judge him but rather love him", 
	//						   "for what he is... a beautiful baby boy.",
	//						   "", 
	//						   "Linora's Aunt", true, true, 2, 0, 0, false);
	//				break;
	//			case 13:
	//				show_story("The woman scooped the child into her arms and smiled at", 
	//						   "him. He stopped crying and began quietly blubbering. He", 
	//						   "was calming down. She cooed him to silence and he slowly",
	//						   "closed his eyes, drifting into sleep.", 
	//						   "null", false, true, 2, 0, 0, false);
	//				break;
	//			case 14:
	//				set_textEntity_text(txtBGPlaceholder, "Back to Abbey", 0);
	//				show_story("After carrying him into the monastery and watching him", 
	//						   "sleep she realized what a wonderful child she had been", 
	//						   "blessed with and prayed to Linidei in thanks.",
	//						   "", 
	//						   "null", false, true, 4, 0, 0, false);
	//				break;
	//			case 15:
	//				show_story("I shall name him Kern, and I hope that I'll be able to", 
	//						   "give him as much love, if not more, than his true parents", 
	//						   "would.",
	//						   "", 
	//						   "Linora's Aunt", true, true, 4, 0, 0, false);
	//				break;
	//			case 16:
	//				set_textEntity_text(txtBGPlaceholder, "Kern Growing Up", 0);
	//				show_story("Before the woman knew it, Kern had already grown to be", 
	//						   "ten. The boy was strong willed, bright spirited, and", 
	//						   "joyful. He addressed the woman as 'Mom' but he knew he was",
	//						   "not her true child. This did not change his love and", 
	//						   "null", false, true, 0, 0, 0, false);
	//				break;
	//			case 17:
	//				show_story("appreciation to her, though. In the ten short years Kern", 
	//						   "had learned...", 
	//						   "",
	//						   "", 
	//						   "null", false, true, 0, 0, 0, false);
	//				break;
	//			case 18:
	//				show_story("To Be Continued...", 
	//						   "", 
	//						   "",
	//						   "", 
	//						   "null", false, true, 0, 0, 0, false);
	//				break;
	//			case 19:
	//				the_player.story.frame = 18;
	//				the_player.story.scene = 1;
	//				the_player.state = main_menu;
	//				break;
	//		}
	//		break;
	//	}
}

void update_story_frame(){
	the_player.state = story_screen;

	//longest txrMessage string:
	//0123456789101112131415161718192021222324252627282930313233
	//do story things

	switch (the_player.story.scene){
		case 0:
			if (the_player.story.frame >= sizeof(scnEnterSchool)/sizeof(story_frame)){
				the_player.story.scene = the_player.story.flag[story_set.currentFlag] + 1;
				the_player.story.frame = 0;
				if ( blnMusic ){
					Mix_HaltMusic();
					if( Mix_PlayMusic( music[the_player.story.scene], -1 ) == -1 )
					{
						quit = true;
					}  
	    
					Mix_VolumeMusic(64);
				}
				update_story_frame();
			}
			else show_story(scnEnterSchool[the_player.story.frame]);
			break;
		case 1:
			if (the_player.story.frame >= sizeof(scnBeMe)/sizeof(story_frame)){
				the_player.story.scene = 0;
				the_player.story.frame = 0;
				init_credits();
			}
			else show_story(scnBeMe[the_player.story.frame]);
			break;
		case 2:
			if (the_player.story.frame >= sizeof(scnBeHuebert)/sizeof(story_frame)){
				the_player.story.scene = 0;
				the_player.story.frame = 0;
				init_credits();
			}
			else show_story(scnBeHuebert[the_player.story.frame]);
			break;
		case 3:
			if (the_player.story.frame >= sizeof(scnBeNoOne)/sizeof(story_frame)){
				the_player.story.scene = 0;
				the_player.story.frame = 0;
				init_credits();
			}
			else show_story(scnBeNoOne[the_player.story.frame]);
			break;
	}
}

void update_state_story(){
	//muh paachikaaru
	if (story_set.useParticles){
		for (int i = 0; i < sizeof(particle)/sizeof(sprite); i++){
			update_particles(particle, i, true, fly);
		}
	}

	//update character fading
	if (story_set.leftCharacterAlpha < 255) story_set.leftCharacterAlpha += dsdltimer/2.0;
	if (story_set.leftCharacterAlpha > 255) story_set.leftCharacterAlpha = 255;
	SDL_SetTextureAlphaMod(txrPortrait[story_set.leftCharacter].txr, story_set.leftCharacterAlpha);
	if (story_set.rightCharacterAlpha < 255) story_set.rightCharacterAlpha += dsdltimer/2.0;
	if (story_set.rightCharacterAlpha > 255) story_set.rightCharacterAlpha = 255;
	SDL_SetTextureAlphaMod(txrPortrait[story_set.rightCharacter].txr, story_set.rightCharacterAlpha);

	update_menu(menuStory);

	update_menu(menuStoryChoices);

	if (blnEscUp){
		story_set.menu = !story_set.menu;
		menuStory.visible = !menuStory.visible;
		blnEscUp = false;
	}
	
	if (lb_up){
		if (story_set.menu){
			if (item_selected(menuStory, 0)){
				story_set.menu = false;
				menuStory.visible = false;
			}
			else if (item_selected(menuStory, 1)){
				blnNoSaveFile = false;
				save_save();
				story_set.menu = false;
				menuStory.visible = false;
			}	
			else if (item_selected(menuStory, 2)){
				if (!blnNoSaveFile){
					load_save();
					update_story_frame();
					story_set.menu = false;
					menuStory.visible = false;
					if ( blnMusic ){
						Mix_HaltMusic();
						if( Mix_PlayMusic( music[the_player.story.scene], -1 ) == -1 )
						{
							quit = true;
						}  
	    
						Mix_VolumeMusic(64);
					}
				}
			}
			else if (item_selected(menuStory, 3)){
				//present options
				the_player.state = main_menu;
				story_set.menu = false;
				menuStory.visible = false;
				if ( blnMusic ){
					Mix_HaltMusic();
					if( Mix_PlayMusic( music[0], -1 ) == -1 )
					{
						quit = true;
					}  
	    
					Mix_VolumeMusic(64);
				}
			}
			else if (item_selected(menuStory, 4)){
				quit = true;
			}
		}
		else if (!story_set.question){
			the_player.story.frame++;
			update_story_frame();
		}
		else{
			the_player.story.flag[0] = menuStoryChoices.itemSelected;
			story_set.question = false;
			menuStoryChoices.visible = false;
			the_player.story.frame++;
			update_story_frame();
		}

		lb_up = false;
	}
	if (rb_up){
		if (!story_set.menu && !story_set.question && the_player.story.frame > 0){
			the_player.story.frame--;
			update_story_frame();
		}
	}
}

void update_state_main_menu(){
	update_menu(menuMain);

	if (blnEscUp){
		quit = true;
		blnEscUp = false;
	}

	//click button with mouse
	if (lb_up){
		if (item_selected(menuMain, 0)){
			wipe_save();
			update_story_frame();
		}
		else if (item_selected(menuMain, 1)){
			if (!blnNoSaveFile){
				load_save();
				update_story_frame();
				if ( blnMusic ){
					Mix_HaltMusic();
					if( Mix_PlayMusic( music[the_player.story.scene], -1 ) == -1 )
					{
						quit = true;
					}  
	    
					Mix_VolumeMusic(64);
				}
			}
		}	
		else if (item_selected(menuMain, 2)){
			quit = true;
		}
		else if (item_selected(menuMain, 3)){
			init_map_making();
		}

		lb_up = false;
	}

	/*for (int i = 0; i < sizeof(particleMM)/sizeof(sprite); i++){
		update_particles(particleMM, i, true, pfxMM);
	}*/

	//update candle flicker
	if (txrStoryBGEffects[0].alphaMod < 240){
		txrStoryBGEffects[0].alphaMod += (rand() % 32) - 8;
	}
	else{
		if (rand() % 256 < 4) txrStoryBGEffects[0].alphaMod = 255 - (rand() % 150);
		else txrStoryBGEffects[0].alphaMod = 255 - (rand() % 15);
	}
	if (txrStoryBGEffects[0].alphaMod < 0) txrStoryBGEffects[0].alphaMod = 0;
	if (txrStoryBGEffects[0].alphaMod > 255) txrStoryBGEffects[0].alphaMod = 255;
	SDL_SetTextureAlphaMod(txrStoryBGEffects[0].txr, txrStoryBGEffects[0].alphaMod);
}

void update_state_map_making(){
	if (blnEscUp){
		quit = true;
		blnEscUp = false;
	}
	//cout << "esc" << endl;
	if (blnTabUp){
		making_map.showGUI = !making_map.showGUI;
		blnTabUp = false;
	}
	//cout << "tab" << endl;
	making_map.btnSelected = -1;
	for (int i = 0; i < sizeof(btnMapMaking)/sizeof(textEntity); i++){
		if (mouse_in_sprite(btnMapMaking[i].entity, intDefaultBtnPaddingX, intDefaultBtnPaddingY)){
			making_map.btnSelected = i;
		}
	}
	//cout << "roll" << endl;
	if (lb_down){
		if (!making_map.showGUI){
			for (int i = 0; i < bMap[the_player.bMap].xTiles; i++){
				for (int ii = 0; ii < bMap[the_player.bMap].yTiles; ii++){
					SDL_SetRenderTarget( renderer, txrMapBuffer.txr );
					if (get_difference(i, int((mouse_x - bMap[0].entity.x)/(32*making_map.fltMapZoomFactor))) +
						get_difference(ii, int((mouse_y - bMap[0].entity.y)/(32*making_map.fltMapZoomFactor))) <
						making_map.brushSize)
					{
						bMap[0].tile[bMap[0].xTiles*ii + i] = making_map.selBrushTile;
					}

					/*if (get_difference(i*32*making_map.fltMapZoomFactor, mouse_x - bMap[0].entity.x) +
						get_difference(ii*32*making_map.fltMapZoomFactor, mouse_y - bMap[0].entity.y) <=
						making_map.brushSize*32*making_map.fltMapZoomFactor)
					{
						bMap[0].tile[bMap[0].xTiles*ii + i] = making_map.selBrushTile;
					}*/

					
					making_map.brushTileClip.x = 32*(bMap[0].tile[bMap[0].xTiles*ii + i]%20);
					making_map.brushTileClip.y = 32*(bMap[0].tile[bMap[0].xTiles*ii + i]/20);
					SDL_SetTextureColorMod(txrBattleTiles[0].txr, 255, 255, 255);
					apply_texture(i*32, ii*32, 32, 32, txrBattleTiles[0].txr, renderer, &making_map.brushTileClip);
					SDL_SetRenderTarget( renderer, NULL );
				}
			}
			/*if (mouse_in_sprite(bMap[0].entity, 0, 0)){
				bMap[0].tile[bMap[0].xTiles*((mouse_y - int(bMap[0].entity.y))/int(32*making_map.fltMapZoomFactor)) + 
					(mouse_x - int(bMap[0].entity.x))/int(32*making_map.fltMapZoomFactor)] = making_map.selBrushTile;
			}*/
		}
	}
	//cout << "lbd" << endl;
	if (lb_up){
		if (making_map.showGUI){
			if (making_map.btnSelected == 0){
				bMap[0] = create_map();
			}
			else if (making_map.btnSelected == 1){
				print_map_properties(bMap[0]);
			}
			else if (making_map.btnSelected == 2){
				save_map();
			}
			else if (making_map.btnSelected == 3){
				load_map();
			}
			else if (making_map.btnSelected == 4){
				if (making_map.brushSize < 10){
					making_map.brushSize++;
					set_textEntity_text(txtMapMaking[0], _itoa(making_map.brushSize, buffer, 10), 0);
				}
			}
			else if (making_map.btnSelected == 5){
				if (making_map.brushSize > 1){
					making_map.brushSize--;
					set_textEntity_text(txtMapMaking[0], _itoa(making_map.brushSize, buffer, 10), 0);
				}
			}
			else if (mouse_in_sprite(sprTileset, 0, 0)){
				for (int i = 0; i < 20; i++){
					for (int ii = 0; ii < 10; ii++){
						if (in_bounds(mouse_x, mouse_y, 
									  sprTileset.x + 32*i, 
									  sprTileset.y + 32*ii, 
									  32, 
									  32))
						{
							making_map.selBrushTile = i + ii*20;
						}	
					}
				}
			}
		}
		
		lb_up = false;
	}
	//cout << "lbu" << endl;
	if (blnMouseWheelClickDown)
	{
		bMap[0].entity.x += delta_mouse_x;
		if (bMap[0].entity.x > SCREEN_WIDTH) bMap[0].entity.x = SCREEN_WIDTH;
		if (bMap[0].entity.x + bMap[0].entity.w < 0) bMap[0].entity.x = -bMap[0].entity.w;
		bMap[0].entity.y += delta_mouse_y;
		if (bMap[0].entity.y > SCREEN_HEIGHT) bMap[0].entity.y = SCREEN_HEIGHT;
		if (bMap[0].entity.y + bMap[0].entity.h < 0) bMap[0].entity.y = -bMap[0].entity.h;
	}
	//cout << "wheel" << endl;

	//zoom
	making_map.fltMapZoomFactor += intMouseWheelY/5.0;
	if (making_map.fltMapZoomFactor < .25) making_map.fltMapZoomFactor = .15;
	else if (making_map.fltMapZoomFactor > 5) making_map.fltMapZoomFactor = 5;
	bMap[0].entity.w = bMap[0].xTiles*64*making_map.fltMapZoomFactor;
	bMap[0].entity.h = bMap[0].yTiles*32*making_map.fltMapZoomFactor;
}

void update_state_credits(){
	for (int i = 0; i < sizeof(txtCredits)/sizeof(textEntity); i++){
		txtCredits[i].entity.y -= dsdltimer*.05;
		if (txtCredits[i].entity.y < -txtCredits[i].entity.h){
			txtCredits[i] = create_textEntity(strCreditTitles[rand()%20], 0, 0, SCREEN_HEIGHT + 100, false, true);
			center_sprite(txtCredits[i].entity, sprScreen, true, false);
		}
	}

	if (!Mix_PlayingMusic() || blnEscUp){
		the_player.state = main_menu;

		if ( blnMusic ){
			Mix_HaltMusic();
			if( Mix_PlayMusic( music[0], -1 ) == -1 )
			{
				quit = true;
			}  
	    
			Mix_VolumeMusic(64);
		}

		blnEscUp = false;
	}
}

void update_game(){
	update_timers();

	if (the_player.state == text_debug_screen){
		update_text_debug();
	}
	else if (the_player.state == main_menu){
		update_state_main_menu();
	}
	else if (the_player.state == walk_screen){
		update_state_walk();
	}
	else if (the_player.state == battle_screen){
		update_state_battle();
	}
	else if (the_player.state == story_screen){
		update_state_story();
	}
	else if (the_player.state == map_making){
		update_state_map_making();
	}
	else if (the_player.state == credits){
		update_state_credits();
	}

	lb_up = false;
	rb_up = false;
	b_up = false;
	blnAUp = false;
	blnSUp = false;
	blnDUp = false;
	blnWUp = false;
	blnEscUp = false;
	blnTabUp = false;
	lb_down_init = false;
}

//render functions

void render_textEntity(textEntity te){
	if (te.visible){
		if (te.showBG){
			apply_texture(te.entity.x - 5, te.entity.y - 5, te.entity.w + 10, te.entity.h + 10, txrButton.txr, renderer);
		}
		//text
		apply_texture( te.entity.x, te.entity.y, te.entity.w, te.entity.h, te.texture.txr, renderer );
	}
}

void render_menu(menu me){
	SDL_SetTextureColorMod(txrButton.txr, 255, 255, 255);
	if (me.showMenuBG){
		apply_texture(me.entity.x, me.entity.y, me.entity.w, me.entity.h, txrButton.txr, renderer);
	}

	//title
	if (me.showTitle){
		if (me.showTitleBG){
			SDL_SetTextureColorMod(txrButton.txr, 128, 128, 128);
			apply_texture(me.entity.x, me.title.entity.y - 5, me.entity.w, me.title.entity.h + 10, txrButton.txr, renderer);
		}
		apply_texture(me.title.entity.x, me.title.entity.y, me.title.entity.w, me.title.entity.h, me.title.texture.txr, renderer);
	}
	
	//items
	for (int i = 0; i < me.itemAmount; i++){
		if (!me.item[i].clickable) SDL_SetTextureColorMod(txrButton.txr, 64, 64, 64);
		else if (me.itemSelected == i) SDL_SetTextureColorMod(txrButton.txr, 255, 255, 255);
		else SDL_SetTextureColorMod(txrButton.txr, 128, 128, 128);

		if (me.item[i].visible){
			if (me.item[i].showBG){
				if (me.item[i].entity.y > me.entity.y + me.entity.h - me.item[i].entity.h - 10){
					SDL_Rect piece = {0, 0, txrButton.w, txrButton.h*1.0*(me.entity.y + me.entity.h - me.item[i].entity.y)/(me.item[i].entity.h + 10)};
					apply_texture(me.entity.x + 5, me.item[i].entity.y - 5, me.entity.w - 10, me.entity.y + me.entity.h - me.item[i].entity.y, txrButton.txr, renderer, &piece);
				}
				else if (me.item[i].entity.y < me.entity.y + 10) {
					SDL_Rect piece = {0, txrButton.h - txrButton.h*1.0*(me.item[i].entity.y + me.item[i].entity.h - me.entity.y)/(me.item[i].entity.h + 10), 
										txrButton.w, txrButton.h*1.0*(me.item[i].entity.y + me.item[i].entity.h - me.entity.y)/(me.item[i].entity.h + 10)};
					apply_texture(me.entity.x + 5, me.entity.y + 5, me.entity.w - 10, me.item[i].entity.y + me.item[i].entity.h - me.entity.y, txrButton.txr, renderer, &piece);
				}
				else apply_texture(me.entity.x + 5, me.item[i].entity.y - 5, me.entity.w - 10, me.item[i].entity.h + 10, txrButton.txr, renderer);
			}
			//text
			if (me.item[i].entity.y > me.entity.y + me.entity.h - me.item[i].entity.h - 5){
				SDL_Rect piece = {0, 0, me.item[i].entity.w, me.entity.y + me.entity.h - me.item[i].entity.y - 5};
				apply_texture(me.item[i].entity.x, me.item[i].entity.y, me.item[i].entity.w, me.entity.y + me.entity.h - me.item[i].entity.y - 5, me.item[i].texture.txr, renderer, &piece);
			}
			else if (me.item[i].entity.y < me.entity.y + 5){
				SDL_Rect piece = {0, me.item[i].texture.h - (me.item[i].entity.y + me.item[i].entity.h - me.entity.y - 5), 
									me.item[i].texture.w, me.item[i].entity.y + me.item[i].entity.h - me.entity.y - 5};
				apply_texture(me.item[i].entity.x, me.entity.y + 5, me.item[i].entity.w, me.item[i].entity.y + me.item[i].entity.h - me.entity.y - 5, me.item[i].texture.txr, renderer, &piece);
			}
			else apply_texture(me.item[i].entity.x, me.item[i].entity.y, me.item[i].entity.w, me.item[i].entity.h, me.item[i].texture.txr, renderer);
			//apply_texture(me.item[i].entity.x, me.item[i].entity.y, me.item[i].entity.w, me.item[i].entity.h, me.item[i].texture.txr, renderer);
		}
	}

	//render scroll bar if applicable
	if (me.scrollable){
		SDL_SetTextureColorMod(txrButton.txr, 128, 128, 128);
		apply_texture(me.entity.x + me.entity.w, me.entity.y, 10, 10, txrButton.txr, renderer);
		apply_texture(me.entity.x + me.entity.w, me.entity.y + me.entity.h - 10, 10, 10, txrButton.txr, renderer);
		SDL_SetTextureColorMod(txrButton.txr, 255, 255, 255);
		apply_texture(me.entity.x + me.entity.w, me.entity.y + 10 + me.scrollAmount, 10, me.scrollBarSize, txrButton.txr, renderer);
	}

	SDL_SetTextureColorMod(txrButton.txr, 255, 255, 255);
}

void render_state_main_menu(){
	//bg
	apply_texture(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, txrStoryBG[5].txr, renderer);
	//apply_texture(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, txrStoryBG[3].txr, renderer);
	//apply_texture(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, txrStoryBGEffects[0].txr, renderer);

	//particles
	/*if (pfxMM == rain) SDL_SetTextureColorMod(txrParticle.txr, 0, 0, 255);
	else SDL_SetTextureColorMod(txrParticle.txr, 195, 161, 45);
	for (int i = 0; i < sizeof(particleMM)/sizeof(sprite); i++){
		if (particleMM[i].visible) apply_texture(particleMM[i].x, particleMM[i].y, txrParticle.w, txrParticle.h, txrParticle.txr, renderer );
	}*/

	render_menu(menuMain);

	SDL_SetTextureColorMod(txrButton.txr, 255, 255, 255);
}

void render_state_map_making(){
	//tiles
	//cout << bMap[0].tile[(bMap[0].xTiles-1)*0 + 0] << endl;
	
	
	apply_texture(bMap[0].entity.x, bMap[0].entity.y, bMap[0].xTiles*32*making_map.fltMapZoomFactor, 
												bMap[0].yTiles*32*making_map.fltMapZoomFactor, txrMapBuffer.txr, renderer);
	for (int i = 0; i < bMap[the_player.bMap].xTiles; i++){
		for (int ii = 0; ii < bMap[the_player.bMap].yTiles; ii++){
			if (!making_map.showGUI && get_difference(i, int((mouse_x - bMap[0].entity.x)/(32*making_map.fltMapZoomFactor))) +
				get_difference(ii, int((mouse_y - bMap[0].entity.y)/(32*making_map.fltMapZoomFactor))) <
				making_map.brushSize)
			{
				SDL_Rect rctHoverTile = {bMap[0].entity.x + 32*i*making_map.fltMapZoomFactor, 
					bMap[0].entity.y + 32*ii*making_map.fltMapZoomFactor, 
					32*making_map.fltMapZoomFactor, 
					32*making_map.fltMapZoomFactor};
				SDL_RenderDrawRect(renderer, &rctHoverTile);
			}
		}
	}

	if (making_map.showGUI){
		//buttons
		for (int i = 0; i < sizeof(btnMapMaking)/sizeof(textEntity); i++){
			if (making_map.btnSelected == i) SDL_SetTextureColorMod(txrButton.txr, 255, 255, 255);
			else SDL_SetTextureColorMod(txrButton.txr, 128, 128, 128);
			render_textEntity(btnMapMaking[i]);
		}

		//text
		for (int i = 0; i < sizeof(txtMapMaking)/sizeof(textEntity); i++){
			SDL_SetTextureColorMod(txrButton.txr, 128, 128, 128);
			render_textEntity(txtMapMaking[i]);
		}
		
		//tileset
		apply_texture(sprTileset.x, sprTileset.y, sprTileset.w, sprTileset.h, txrBattleTiles[0].txr, renderer);
		for (int i = 0; i < 20; i++){
			for (int ii = 0; ii < 10; ii++){
				if (in_bounds(mouse_x, mouse_y, 
							  sprTileset.x + 32*i, 
							  sprTileset.y + 32*ii, 
							  32, 
							  32))
				{
					SDL_Rect rctHoverTile = {sprTileset.x + 32*i,
											 sprTileset.y + 32*ii,
											 32,
											 32};
					SDL_RenderDrawRect(renderer, &rctHoverTile);
				}	
			}
		}
	}

	SDL_SetTextureColorMod(txrDialogBox.txr, 255, 255, 255);
}

void render_state_walk(){
	////map
	//SDL_Rect temp_clip = {0, 0, maps[0].x, maps[0].y};
	//apply_texture( (SCREEN_WIDTH/2) - the_player.ael.walk.x, (SCREEN_HEIGHT/2) - the_player.ael.walk.y + (txrElf->h/2), light_gray, renderer, &temp_clip);
	//
	////sprite drawing 
	//character charToDraw[10];
	//int intLowest = maps[0].y + 5;
	//int intMinimum = 0;
	//bool blnPlayerDrawn = false;
	///*for (int i = 0; i < 10; i++){
	//	for (int ii = 0; i < 10; i++){
	//		if (maps[0].characters[ii].walk.y < intLowest && maps[0].characters[ii].walk.y > intMinimum){
	//			charToDraw[i] = maps[0].characters[ii];
	//			intLowest = maps[0].characters[ii].walk.y;
	//		}
	//	}
	//	intMinimum = intLowest;
	//	intLowest = maps[0].y + 5;
	//}*/
	//for (int i = 0; i < 10; i++){
	//	//player sprite
	//	if (!blnPlayerDrawn && the_player.ael.walk.y < charToDraw[i].walk.y){
	//		apply_texture( (SCREEN_WIDTH/2) - (txrElf->w/2), (SCREEN_HEIGHT/2) - (txrElf->h/2), txrElf, renderer);
	//		blnPlayerDrawn = true;
	//	}
	//	//clone sprite
	//	if (maps[0].characters[i].walk.visible) apply_texture((SCREEN_WIDTH/2) - the_player.ael.walk.x + maps[0].characters[i].walk.x - (txrElf->w/2), 
	//											(SCREEN_HEIGHT/2) - the_player.ael.walk.y + (txrElf->h/2) + maps[0].characters[i].walk.y - (txrElf->h), txrElf, renderer);
	//}
	//if (!blnPlayerDrawn){
	//		apply_texture( (SCREEN_WIDTH/2) - (txrElf->w/2), (SCREEN_HEIGHT/2) - (txrElf->h/2), txrElf, renderer);
	//}
}

void render_state_battle(){
	//particles
	for (int i = 0; i < sizeof(particle)/sizeof(sprite); i++){
		if (particle[i].visible) apply_texture( particle[i].x, particle[i].y, txrParticle.w, txrParticle.h, txrParticle.txr, renderer );
	}

	//tiles
	for (int i = 0; i < bMap[the_player.bMap].xTiles; i++){
		for (int ii = 0; ii < bMap[the_player.bMap].yTiles; ii++){
			if (get_difference(i, the_player.ael.battle.xTiled) +
				get_difference(ii, the_player.ael.battle.yTiled) <=
				the_player.ael.cdp)
			{
				SDL_SetTextureColorMod(txrBattleTiles[0].txr, 255, 255, 255);
				apply_texture(bMap[the_player.bMap].entity.x + (i*32), 
						  bMap[the_player.bMap].entity.y + (ii*32), 
						  txrBattleTiles[0].w, txrBattleTiles[0].h, 
						  txrBattleTiles[0].txr, renderer);
			}
			else{
				SDL_SetTextureColorMod(txrBattleTiles[0].txr, 156, 50, 50);
				apply_texture(bMap[the_player.bMap].entity.x + (i*32), 
						  bMap[the_player.bMap].entity.y + (ii*32), 
						  txrBattleTiles[0].w, txrBattleTiles[0].h,  
						  txrBattleTiles[0].txr, renderer);
			}
		}
	}

	//elf
	apply_texture( (SCREEN_WIDTH/2) - (txrElf.w/2), (SCREEN_HEIGHT/2) - (txrElf.h/2), txrElf.w, txrElf.h, txrElf.txr, renderer );
	//text boxes
	for (int i = 0; i < sizeof(txtBattle)/sizeof(textEntity); i++){
		render_textEntity(txtBattle[i]);
	}
	//buttons
	for (int i = 0; i < sizeof(btnBattle)/sizeof(textEntity); i++){
		render_textEntity(btnBattle[i]);
	}
}

void render_state_story(){	
	//bg
	if (story_set.StoryBG != 0){
		apply_texture(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, txrStoryBG[story_set.StoryBG].txr, renderer);
	}
	
	//render_textEntity(txtBGPlaceholder);

	//particles
	if (story_set.useParticles){
		for (int i = 0; i < (sizeof(particle)/sizeof(sprite))/2; i++){
			if (particle[i].visible) apply_texture( particle[i].x, particle[i].y, txrParticle.w, txrParticle.h, txrParticle.txr, renderer );
		}
	}
	

	//character txrPortraits
	apply_texture( 0, 0, 400, 450, txrPortrait[story_set.leftCharacter].txr, renderer );
	apply_texture( 400, 0, 400, 450, txrPortrait[story_set.rightCharacter].txr, renderer );
	
	//more particles
	if (story_set.useParticles){
		for (int i = (sizeof(particle)/sizeof(sprite))/2; i < sizeof(particle)/sizeof(sprite); i++){
			if (particle[i].visible) apply_texture( particle[i].x, particle[i].y, txrParticle.w, txrParticle.h, txrParticle.txr, renderer );
		}
	}
	SDL_SetTextureColorMod(txrParticle.txr, 255, 255, 255);
	
		
	SDL_SetTextureColorMod(txrButton.txr, 64, 64, 64);
	SDL_SetTextureAlphaMod(txrButton.txr, 192);
	//nametag
	if (story_set.showNametag){
		render_textEntity(nametag);
	}
		
	//dialog box
	if (story_set.showDialogue) apply_texture( 50, SCREEN_HEIGHT - 125, 680, 120, txrButton.txr, renderer );
	
	//dialogue
	for (int i = 0; i < sizeof(txtMessage)/sizeof(textEntity); i++){
		render_textEntity(txtMessage[i]);
	}
	
	//choices
	SDL_SetTextureAlphaMod(txrButton.txr, 255);
	if (story_set.question){
		//for (int i = 0; i < sizeof(btnChoices)/sizeof(textEntity); i++){
		//	if (story_set.choiceSelection == i) SDL_SetTextureColorMod(txrButton.txr, 255, 255, 255);
		//	else SDL_SetTextureColorMod(txrButton.txr, 128, 128, 128);
		//	//dialog box based buttons
		//	render_textEntity(btnChoices[i]);
		//}

		render_menu(menuStoryChoices);
	}
	
	if (story_set.menu){
		SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0x99 );
		SDL_Rect actuallyWholeScreen = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
		SDL_RenderFillRect(renderer, &actuallyWholeScreen);
	}
		
	//menu
	if (story_set.menu){
		render_menu(menuStory);
	}
		
	SDL_SetTextureColorMod(txrButton.txr, 255, 255, 255);
	SDL_SetTextureAlphaMod(txrButton.txr, 255);
	SDL_SetTextureColorMod(txrDialogBox.txr, 255, 255, 255);
}

void render_state_credits(){
	for (int i = 0; i < sizeof(txtCredits)/sizeof(textEntity); i++){
		render_textEntity(txtCredits[i]);
	}
}

void render_game(){
	//Clear screen
	SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0xFF );
	SDL_RenderClear( renderer );

	if (the_player.state == text_debug_screen){
		render_state_story();
	}
	else if (the_player.state == main_menu){
		render_state_main_menu();
	}
	else if (the_player.state == walk_screen){
		render_state_walk();
	}
	else if (the_player.state == battle_screen){
		render_state_battle();
	}
	else if (the_player.state == story_screen){
		render_state_story();
	}
	else if (the_player.state == map_making){
		render_state_map_making();
	}
	else if (the_player.state == credits){
		render_state_credits();
	}

	//render fps
	/*if (dsdltimer > 0){
		intFPS = int(1000/dsdltimer);
		set_textEntity_text(txtFPS, _itoa(intFPS, buffer, 10), 0);
		render_textEntity(txtFPS);
	}*/
	
	//Update screen
	SDL_RenderPresent( renderer );
}

//extra init stuff
void write_scnEnterSchool(){
	/*scnEnterSchool[0] = create_story_frame(string text0, 
											 string text1, 
											 string text2, 
											 string text3, 
											 string nt, bool n, bool sd, 
											 int intBgPic, int l_c, int r_c, bool particles, 
											 bool pc, int flaggy, 
											 string choice0, 
											 string choice1, 
											 string choice2);*/

										  //0123456789101112131415161718192021222324252627282930313233
	scnEnterSchool[0] = create_story_frame("It's your first day in Japanese class you just can't wait", 
										   "to see how many friends you will make. The classroom is", 
										   "just what one would expect from a typical Japanese", 
										   "classroom. Kanji line the walls and weeaboos fill the", 
										   "Kern's Conscience", true, true, 
										   5, 6, 0, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnEnterSchool[1] = create_story_frame("seats. You silently look around at all your peers and form", 
										   "opinions about each. In the back you see the Rat Pack, a", 
										   "group mostly formed by kobolds and other less-intelligent", 
										   "rat-like creatures. You decide that you won't sit by them", 
										   "Kern's Conscience", true, true, 
										   5, 6, 0, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnEnterSchool[2] = create_story_frame("because their unintelligence is more likely to rub off on", 
										   "you than your intelligence is to rub off on them. Just in", 
										   "front of them are the Tryhards, a collection of elves and", 
										   "other high maintenance races that you'd probably fit in", 
										   "Kern's Conscience", true, true, 
										   5, 6, 0, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnEnterSchool[3] = create_story_frame("with, if only you weren't so modest. You decide to pass", 
										   "on the idea of sitting with them. Finally, a creepy", 
										   "glare up to the front of the class reveals two lovely", 
										   "young ladies, an elf rogue and a human witch. You are", 
										   "Kern's Conscience", true, true, 
										   5, 6, 0, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnEnterSchool[4] = create_story_frame("surprised that the witch would take a class like this, but", 
										   "then again she does have that elf as a bodyguard. You", 
										   "consider sitting next to them but instead decide to stand", 
										   "in place for a while and observe. The witch's witch hat", 
										   "Kern's Conscience", true, true, 
										   5, 6, 0, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnEnterSchool[5] = create_story_frame("covers her beautiful blonde hair witch is arranged into", 
										   "cute little drills. Hanging from these drills are the", 
										   "petrified hands of an unknown animal, which seem to grab", 
										   "at the witch's face. You blink to confirm that they are", 
										   "Kern's Conscience", true, true, 
										   5, 6, 7, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnEnterSchool[6] = create_story_frame("not actually moving. In order to stare at her front, you", 
										   "move to the teacher's desk and continue to observe. She", 
										   "has enchanting blue eyes that threaten to steal your soul,", 
										   "but fortunately your mom remembered to make you take your", 
										   "Kern's Conscience", true, true, 
										   5, 6, 7, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnEnterSchool[7] = create_story_frame("talisman that protects from soul stealing to school. The", 
										   "witch also dons quasi-traditional German clothing witch", 
										   "exaggerates the bust. You don't have a talisman to protect", 
										   "you from that. You also notice that she's white, so that's", 
										   "Kern's Conscience", true, true, 
										   5, 6, 7, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnEnterSchool[8] = create_story_frame("a plus. On the contrary, her elf bodyguard looks pretty", 
										   "standard and even a little stereotypical. It seems the", 
										   "teacher has finally entered the room.", 
										   "", 
										   "Kern's Conscience", true, true, 
										   5, 6, 5, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnEnterSchool[9] = create_story_frame("Shitto down, kurasu wa hajeemeru!", 
										   "", 
										   "", 
										   "", 
										   "Sensei", true, true, 
										   5, 6, 0, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnEnterSchool[10] = create_story_frame("Startled raw, you quickly grab the seat next to the", 
										   "witch. You try not to stare at her from such a close", 
										   "distance. She smells of the desert and a chill runs down", 
										   "your spine as she seems to be draining your body's energy.", 
										   "Kern's Conscience", true, true, 
										   5, 6, 7, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnEnterSchool[11] = create_story_frame("She whispers to the elf once in a while in a haunting", 
										   "tone until finally you can't hold it in anymore and:", 
										   "", 
										   "", 
										   "Kern's Conscience", true, true, 
										   5, 6, 7, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnEnterSchool[12] = create_story_frame("She whispers to the elf once in a while in a haunting", 
										   "tone until finally you can't hold it in anymore and:", 
										   "", 
										   "", 
										   "Kern's Conscience", true, true, 
										   5, 6, 0, false, 
										   true, 0, 
										   "Shake with one of the witch's petrified hands.", 
										   "Tell the witch she's cute while touching her arm.", 
										   "Fall out of your chair.");
}

void write_scnBeMe(){
	//choice: "Shake with one of the witch's petrified hands."
	scnBeMe[0] = create_story_frame(	   "You turn to the witch and time seems to slow down as your", 
										   "hand closes in on one of her hands. The witch and elf both", 
										   "stare with morbid intent, as your hand engages on one of", 
										   "her hair-hands. You squeeze and the hair-hand can offer no", 
										   "Kern's Conscience", true, true, 
										   5, 6, 7, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnBeMe[1] = create_story_frame(	   "resistance except the rigidity that comes with death. The", 
										   "seconds of handshake feel like hours and, when you finally", 
										   "disengage the shake, the rogue acquires a look of disgust", 
										   "that is so intense, you permanantly lose two charisma", 
										   "Kern's Conscience", true, true, 
										   5, 6, 7, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnBeMe[2] = create_story_frame(	   "points. You still swear her hair-hands don't move, but the", 
										   "one you shook somehow seems to be shying away from you now.", 
										   "", 
										   "", 
										   "Kern's Conscience", true, true, 
										   5, 6, 7, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnBeMe[3] = create_story_frame(	   "Are you retarded?! Get your hands off of her - hands!", 
										   "", 
										   "", 
										   "", 
										   "Elf Rogue", true, true, 
										   5, 6, 5, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnBeMe[4] = create_story_frame(	   "Wait no, it's alright Ael... That's unbelievable. Nobody's", 
										   "ever done that before. What's your name, demon-child?", 
										   "", 
										   "", 
										   "Witch", true, true, 
										   5, 6, 7, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnBeMe[5] = create_story_frame(	   "It's Kern, what about you?", 
										   "", 
										   "", 
										   "", 
										   "Kern", true, true, 
										   5, 6, 7, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnBeMe[6] = create_story_frame(	   "I'm Ael and she's Rosaline. Don't you forget it!", 
										   "", 
										   "", 
										   "", 
										   "Ael", true, true, 
										   5, 6, 5, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnBeMe[7] = create_story_frame(	   "Don't worry about her, she's like that to everyone. Well,", 
										   "um, Kern... would you like to watch some ecchi anime with", 
										   "me sometime?", 
										   "", 
										   "Rosaline", true, true, 
										   5, 6, 7, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnBeMe[8] = create_story_frame(	   "Wait, your mom's warned you about this before. \"She will", 
										   "lure you in with a moe stereotype and ecchi anime, and", 
										   "then at your weakest, she will devour all of your clothes,", 
										   "even those witch still lie in you closet.\"", 
										   "Kern's Conscience", true, true, 
										   5, 6, 7, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnBeMe[9] = create_story_frame(	   "Uh, sorry. As much as I'd like to tap dat, I'm going to", 
										   "to have to decline your offer.", 
										   "", 
										   "", 
										   "Kern", true, true, 
										   5, 6, 7, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnBeMe[10] = create_story_frame(	   "Oh, okay.", 
										   "", 
										   "", 
										   "", 
										   "Rosaline", true, true, 
										   5, 6, 7, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnBeMe[11] = create_story_frame(	   "Days later, you mysteriously die with the only clue as to", 
										   "your demise being severe stomach and back pains before", 
										   "your passing.", 
										   "", 
										   "Kern's Conscience", false, true, 
										   0, 0, 0, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
}

void write_scnBeNoOne(){
	//choice: "Fall out of your chair."
	scnBeNoOne[0] = create_story_frame(	   "You swiftly fall out of your chair, but just before you", 
										   "would have hit the floor, you suddenly lose all weight and", 
										   "become suspended in air. It seems that the witch cast a", 
										   "levitation spell on you by reflex.", 
										   "Kern's Conscience", true, true, 
										   5, 6, 7, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnBeNoOne[1] = create_story_frame(	   "Ah- thanks.", 
										   "", 
										   "", 
										   "", 
										   "Kern", true, true, 
										   5, 6, 7, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnBeNoOne[2] = create_story_frame(	   "Rosaline, I know you're a kind girl, but should you really", 
										   "be wasting your mana on such a worthless being?", 
										   "", 
										   "", 
										   "Rogue Elf", true, true, 
										   5, 5, 7, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnBeNoOne[3] = create_story_frame(	   "I suppose you're right.", 
										   "", 
										   "", 
										   "", 
										   "Rosaline", true, true, 
										   5, 5, 7, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnBeNoOne[4] = create_story_frame(	   "Immediately you regain all your weight. Normally, falling", 
										   "out of a chair would be harmless to you, but you became so", 
										   "weightless that in all the talk you ended up close to the", 
										   "ceiling. You also shifted a little bit in the air, so you", 
										   "Kern's Conscience", true, true, 
										   5, 5, 7, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnBeNoOne[5] = create_story_frame(	   "end up hitting the side of a desk, losing consciousness.", 
										   "", 
										   "", 
										   "", 
										   "Kern's Conscience", true, true, 
										   5, 5, 7, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnBeNoOne[6] = create_story_frame(	   "...", 
										   "", 
										   "", 
										   "", 
										   "Kern's Conscience", false, true, 
										   0, 0, 0, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnBeNoOne[7] = create_story_frame(	   "You wake up in the nurse's office and look around.", 
										   "A female kobold looms over your face and seems to be the", 
										   "one who brought you here. She's kinda cute... for a", 
										   "kobold.", 
										   "Kern's Conscience", true, true, 
										   5, 0, 0, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnBeNoOne[8] = create_story_frame(	   "Is a new love blossoming? Could our young Kern be", 
										   "exploring past the realms of medicine and science? Find", 
										   "out next time in --2Kern4School--!", 
										   "", 
										   "Kern's Conscience", false, true, 
										   0, 0, 0, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
}

void write_scnBeHuebert(){
	//choice: "Tell the witch she's cute while touching her arm."
	scnBeHuebert[0] = create_story_frame(  "You brush up against the witch, calling her cute in her", 
										   "native language with intermixed cat noises, returning the", 
										   "spine chill you previously received. She then promptly", 
										   "goes into shock and her elf companion rushes to her side.", 
										   "Kern's Conscience", true, true, 
										   5, 6, 7, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnBeHuebert[1] = create_story_frame(  "What the hell is wrong with you?", 
										   "", 
										   "", 
										   "", 
										   "Rogue Elf", true, true, 
										   5, 6, 5, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnBeHuebert[2] = create_story_frame(  "What do you mean?", 
										   "", 
										   "", 
										   "", 
										   "Kern", true, true, 
										   5, 6, 5, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnBeHuebert[3] = create_story_frame(  "You are confused, as the defeat of the witch just earned", 
										   "you fifty experience points. You're actually about to", 
										   "level up, too. A swift punch to the throat ends your life", 
										   "and you fall into a black abyss.", 
										   "Kern's Conscience", true, true, 
										   5, 6, 5, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnBeHuebert[4] = create_story_frame(  "...", 
										   "", 
										   "", 
										   "", 
										   "Kern's Conscience", false, true, 
										   0, 0, 0, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
	scnBeHuebert[5] = create_story_frame(  "Fortunately, a ressurection only costs fifty experience", 
										   "points, so you are quickly up and running again to", 
										   "terrorize the next pretty girl you meet. Unfortunately,", 
										   "you are by some mistake reborn as a golem.", 
										   "Kern's Conscience", true, true, 
										   5, 0, 0, false, 
										   false, 0, 
										   "0", 
										   "0", 
										   "0");
}

void write_story(){
	write_scnEnterSchool();
	write_scnBeMe();
	write_scnBeNoOne();
	write_scnBeHuebert();
}

void init_clips(){
	//set some clips
	clips[0].x = 0;
	clips[0].y = 0;
	clips[0].w = 0;
	clips[0].h = 0;

	clips[1].x = 50;
	clips[1].y = 50;
	clips[1].w = 50;
	clips[1].h = 50;
	
	clips[2].x = 0;
	clips[2].y = 50;
	clips[2].w = 50;
	clips[2].h = 50;

	clips[3].x = 50;
	clips[3].y = 0;
	clips[3].w = 50;
	clips[3].h = 50;
	
	clips[4].x = 0;
	clips[4].y = 0;
	clips[4].w = 50;
	clips[4].h = 50;

}

void init_buttons/*and menus*/(){
	//main menu menu
	menuMain = create_menu("Main Menu", 50, SCREEN_HEIGHT - 200, 150, 100 , 3, true);
	menuMain.item[0] = create_textEntity("New Game", 0, 10, 10, true, true);
	menuMain.item[1] = create_textEntity("Continue", 0, 10, 60, true, true);
	menuMain.item[2] = create_textEntity("Quit Game", 0, 10, 110, true, true);
	//menuMain.item[3] = create_textEntity("Make A Map", 0, 10, 160, true, true);
	/*menuMain.item[4] = create_textEntity("New Game", 0, 10, 10, true, true);
	menuMain.item[5] = create_textEntity("Continue", 0, 10, 60, true, true);
	menuMain.item[6] = create_textEntity("Quit Game", 0, 10, 110, true, true);
	menuMain.item[7] = create_textEntity("Make A Map", 0, 10, 160, true, true);*/
	align_menu_items(menuMain);
	menuMain.moveable = true;

	//battle buttons
	btnBattle[0] = create_textEntity("Forfeit Turn", 0, 25, SCREEN_HEIGHT - 50, true, true);

	//story choice buttons
	menuStoryChoices = create_menu("Choose Your choice", 50, 150, 675, 120 , 3, false);
	menuStoryChoices.item[0] = create_textEntity("1", 0, 50, 100, true, true);
	menuStoryChoices.item[1] = create_textEntity("2", 0, 50, 150, true, true);
	menuStoryChoices.item[2] = create_textEntity("3", 0, 50, 200, true, true);
	menuStoryChoices.showTitle = false;
	menuStoryChoices.showMenuBG = false;
	align_menu_items(menuStoryChoices);

	//story menu menu
	menuStory = create_menu("Menu", (SCREEN_WIDTH/2) - 75, 50, 175, 180, 5, false);
	menuStory.item[0] = create_textEntity("Resume Game", 0, SCREEN_WIDTH/2, 50, true, true);
	menuStory.item[1] = create_textEntity("Save Game", 0, SCREEN_WIDTH/2, 100, true, true);
	menuStory.item[2] = create_textEntity("Load Last Save", 0, SCREEN_WIDTH/2, 150, true, true);
	menuStory.item[3] = create_textEntity("Main Menu", 0, SCREEN_WIDTH/2, 200, true, true);
	menuStory.item[4] = create_textEntity("Quit Game", 0, 0, 0, true, true);
	align_menu_items(menuStory);

	//map making buttons
	btnMapMaking[0] = create_textEntity("Create New Map", 0, 10, 10, true, true);
	btnMapMaking[1] = create_textEntity("Map Properties", 0, 10, 50, true, true);
	btnMapMaking[2] = create_textEntity("Save Map", 0, 10, 90, true, true);
	btnMapMaking[3] = create_textEntity("Load Map", 0, 10, 130, true, true);
	btnMapMaking[4] = create_textEntity("[^] Brush", 0, 10, 170, true, true);
	btnMapMaking[5] = create_textEntity("Size [v]", 0, 55, 210, true, true);
}

void init_text_boxes(){
	//battle txt
	txtBattle[0] = create_textEntity("Cooldown Points:", 0, 25, 25, true, true);
	txtBattle[1] = create_textEntity("0", 0, 25, 65, true, true);

	//dialogue txt
	for (int i = 0; i < sizeof(txtMessage)/sizeof(textEntity); i++){
		txtMessage[i] = create_textEntity("null", 0, 75, 336 + 25*i, false, true);
	}

	//background placeholder txt
	txtBGPlaceholder = create_textEntity("Nowhere", 0, 100, (SCREEN_HEIGHT/2) - 100, false, true);

	//ael's nametag
	nametag = create_textEntity("Ael", 0, 85, SCREEN_HEIGHT - 157, true, true);

	//fps properties
	txtFPS = create_textEntity("0", 0, 5,5, false, true);

	//battle txt
	txtMapMaking[0] = create_textEntity("0", 0, 10, 210, true, true);
	txtMapMaking[1] = create_textEntity("X: 0", 0, 10, txtMapMaking[0].entity.y + txtMapMaking[0].entity.h + btnMapMaking[0].entity.h + 30, true, false);
	txtMapMaking[2] = create_textEntity("Y: 0", 0, 30 + txtMapMaking[1].entity.w, txtMapMaking[0].entity.y + txtMapMaking[0].entity.h + btnMapMaking[0].entity.h + 30, true, false);
}

void extra_init(){
	init_clips();
	write_story();

	//set up a screen sprite for centering
	sprScreen.w = SCREEN_WIDTH;
	sprScreen.h = SCREEN_HEIGHT;

	init_buttons();

	init_text_boxes();

	//maps
	bMap[0].xTiles = 10;
	bMap[0].yTiles = 10;
	bMap[0].entity.w = bMap[0].xTiles*32;
	bMap[0].entity.h = bMap[0].yTiles*32;

	//particle scaling
	txrParticle.w = txrParticle.w * fltParticleScaling;
	txrParticle.h = txrParticle.h * fltParticleScaling;

	sprTileset.x = SCREEN_WIDTH - txrBattleTiles[0].w;
	sprTileset.y = SCREEN_HEIGHT - txrBattleTiles[0].h;
	sprTileset.w = txrBattleTiles[0].w;
	sprTileset.h = txrBattleTiles[0].h;

	//arbitrary characters
	/*maps[0].characters[0].walk.x = 200;
	maps[0].characters[0].walk.y = 100;
	maps[0].characters[0].walk.visible = true;
	maps[0].characters[1].walk.x = 60;
	maps[0].characters[1].walk.y = 70;
	maps[0].characters[1].walk.visible = true;
	maps[0].characters[2].walk.x = 40;
	maps[0].characters[2].walk.y = 130;
	maps[0].characters[2].walk.visible = true;*/

	//load the save file to check if it exists
	load_save();
	if (blnNoSaveFile){
		menuMain.item[1].clickable = false;
		menuStory.item[2].clickable = false;
	}

	if ( blnMusic ){
		if( Mix_PlayMusic( music[0], -1 ) == -1 )
	    {
	        quit = true;
	    }  
	    
	    Mix_VolumeMusic(64);
	}

	strCreditTitles[0] = "Producer: Wyatt Sushinsky";
	strCreditTitles[1] = "Executive Producer: Wyatt Sushinsky";
	strCreditTitles[2] = "Director: Wyatt Sushinsky";
	strCreditTitles[3] = "Original Story: Wyatt Sushinsky";
	strCreditTitles[4] = "Concept Artist: Wyatt Sushinsky";
	strCreditTitles[5] = "Lead Programmer: Wyatt Sushinsky";
	strCreditTitles[6] = "Music: Old People With Good Taste";
	strCreditTitles[7] = "UI Designer: Wyatt Sushinsky";
	strCreditTitles[8] = "Gameplay Designer: Wyatt Sushinsky";
	for (int i = 9; i < 20; i++){
		strCreditTitles[i] = "Happy Birthday: Senpai";
	}
}

//what we've all been waiting for

int main( int argc, char* args[] ) {
	init();
	//cout << "init" << endl; 
	load_files();
	//cout << "files loaded" << endl;
	extra_init();
	//cout << "extra" << endl;

	while(!quit){
		get_userinput();
		//cout << "got input" << endl;
		update_game();
		//cout << "updated" << endl;
		render_game();
		//cout << "rendered" << endl;
	}

	clean_up();
	cout << "END" << endl;
	return 0;
}