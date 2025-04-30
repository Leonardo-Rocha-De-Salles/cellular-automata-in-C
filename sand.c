#include "SDL.h"
#include "SDL_error.h"
#include "SDL_events.h"
#include "SDL_log.h"
#include "SDL_mouse.h"
#include "SDL_pixels.h"
#include "SDL_platform.h"
#include "SDL_render.h"
#include "SDL_video.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

//Dovrei fare la risoluzione custom per rendere migliore, poi aggiungere anche multithreading e robe del genere.

//Fare pulsante per scelta de materiali.


#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

typedef enum{
	RUNNING,
	PAUSED,
	QUIT
}sim_state;


typedef enum{
	SAND,
	EMPTY,
	WATER
}material;

typedef struct{
	bool m1,m2;
	material material;
}Input;

typedef struct{
	material pixel_material;
	uint32_t color;
}cell;

typedef struct{
	cell* grid;
}World;


typedef struct{
	SDL_Window *window;
	SDL_Renderer* renderer;
	uint32_t WindowID;
}sdl_t;


typedef struct{
	sdl_t* WINDOW;
	SDL_Texture* texture;
	sim_state state;
}mainSim;
	
//Se proprio volessi potrei separare state e texture dalla struct, creare una struct window e una maingame per non creare due texture quando faccio due finestre
//
//Qui metterò tutte le tipologie di materiali in modo da assegnarle facilmente con cells[EMPTY] per esempio
const cell cells[] = {
	[EMPTY] = {EMPTY, 0x00000000},
	[SAND] = {SAND, 0xC2B280FF},
	[WATER] = {WATER, 0x0000FFFF}
};


int random(){
	return(rand() % 2);
}

bool init_Sim(mainSim* sim){
	sim -> texture = SDL_CreateTexture(sim -> WINDOW -> renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	if(sim -> texture == NULL){
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create texture: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	sim -> state = RUNNING;
	return true;
}

bool init_sdl(sdl_t* mainWindow, int Width, int Height){
	if(SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0){
		SDL_Log("Unable to initialize mainWindow: %s", SDL_GetError());
		fprintf(stderr, "mainWindow Error Logs:\n");
		return false;
	}
	//Creazione della nostra SDL_Window
	mainWindow -> window = SDL_CreateWindow("Sand Sim", //Nome finestra 
				  SDL_WINDOWPOS_CENTERED,  //Proprietà posizione al lancio
				  SDL_WINDOWPOS_CENTERED, //Proprietà posizione al lancio
				  Width,   //Larghezza finestra
				  Height,  //Altezza finestra
				  0);	//Flags vari
	//Check Window Creation
	if(mainWindow -> window == NULL){
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	//RendererCreation and check
	mainWindow -> renderer = SDL_CreateRenderer(mainWindow->window, -1, SDL_RENDERER_ACCELERATED);
	if(mainWindow -> renderer == NULL){
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create renderer: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	mainWindow -> WindowID = SDL_GetWindowID(mainWindow -> window);
	return true;
}

void mouseCreate(material material, int mouseX, int mouseY, World* world){
	for(int y = -5; y<5;y++){
		for(int x = -5; x<5; x++){
			world -> grid[((mouseY*SCREEN_WIDTH)+(SCREEN_WIDTH*y) + mouseX)+x] = cells[material];
		}
	}
	return;
}

void update_texture(mainSim* mainWindow, World* world){
	void* pixels;
	int pitch;

	//Probabilmente non ho bisogno di pixel_buffer
	SDL_LockTexture(mainWindow -> texture, NULL, &pixels, &pitch);
	uint32_t* texturePixels = (uint32_t*)pixels;

	for(int y = 0; y < SCREEN_HEIGHT; y++){
		for(int x = 0; x < SCREEN_WIDTH; x++){
			int index = y*SCREEN_WIDTH + x;
			texturePixels[index] = world -> grid[index].color;
		}
	}
	SDL_UnlockTexture(mainWindow->texture);
//Gli update di colori vanno fatti scriv:%s/\<old_var\>/new_var/gendo il colore uint32 in texturePixels, questo significa che si aggiornerà
	//Seguendo ciò che c'è dentro grid.color
}

void init_texture(mainSim* mainWindow, World* world){
	(void)mainWindow;
	for(int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++){
		world -> grid[i] = cells[EMPTY];}
	update_texture(mainWindow, world);
	return;
}
	
void cleanupMain(mainSim* sim){
	SDL_DestroyWindow(sim -> WINDOW -> window);
	SDL_DestroyTexture(sim -> texture);
	SDL_DestroyRenderer(sim -> WINDOW -> renderer);
}

void cleanupMaterialWindow(sdl_t* mainWindow){
	SDL_DestroyWindow(mainWindow -> window);
	SDL_DestroyRenderer(mainWindow -> renderer);
	return;
}

void doInput(mainSim* mainWindow, World* world, Input* input){
	SDL_Event event;
	int mouseX, mouseY;
	while(SDL_PollEvent(&event)){
		switch(event.type)
		{
			case SDL_QUIT:
				if(event.window.windowID == mainWindow -> WINDOW -> WindowID){
					mainWindow -> state = QUIT;
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				if(event.window.windowID == mainWindow -> WINDOW -> WindowID){
				switch(event.button.button){
					case SDL_BUTTON_LEFT:
						input->material = SAND;
						input -> m1 = true;
						SDL_Log("Mouse (%d,%d)\n", mouseX, mouseY);
						break;
					case SDL_BUTTON_RIGHT:
						input->material = EMPTY;
						input -> m2 = true;
						break;
					default:
						break;
				}
				}
				break;

			case SDL_MOUSEMOTION:
				if(event.window.windowID == mainWindow -> WINDOW -> WindowID){
				if(input->m1){
						mouseX = event.motion.x;
						mouseY = event.motion.y;
						SDL_Log("Mouse (%d,%d)\n", mouseX, mouseY);
						mouseCreate(input->material, mouseX, mouseY, world);
						break;
				}
				else if(input->m2){
						mouseX = event.motion.x;
						mouseY = event.motion.y;
						mouseCreate(input->material, mouseX, mouseY, world);
				}
			}
				break;
			case SDL_MOUSEBUTTONUP:
				if(event.window.windowID == mainWindow -> WINDOW -> WindowID){
				switch(event.button.button){
					case SDL_BUTTON_LEFT:
						input -> m1 = false;
						break;
					case SDL_BUTTON_RIGHT:
						input -> m2 = false;
						break;
					default:
						break;
				}
				}
				break;
					default:
				break;
		
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym){
					case SDLK_SPACE:
						mainWindow -> state = PAUSED;
						break;
					case SDLK_ESCAPE:
						mainWindow -> state = QUIT;
						break;
					default:
						break;
				}
			break;

		}
	}
}


void update_mainWindow(mainSim* mainWindow){
		SDL_RenderClear(mainWindow-> WINDOW -> renderer);
		SDL_RenderCopy(mainWindow-> WINDOW -> renderer, mainWindow->texture, NULL, NULL);	
		SDL_RenderPresent(mainWindow-> WINDOW -> renderer);
}

void update_materialWindow(sdl_t* window){
	SDL_SetRenderDrawColor(window -> renderer, 144, 144, 144, 144);
	SDL_RenderClear(window -> renderer);
	SDL_RenderPresent(window->renderer);
}

void sandSim(World* world){
	//Il problema principale è come viene updata la sabbia da sinistra a destra nel secondo for loop.
	bool direction = false;
	for(int y = SCREEN_HEIGHT-2; y>= 0; y--){
		if(direction){
		direction = !direction;
		for(int x = SCREEN_WIDTH-1; x >= 0; x--){
			int index = y*SCREEN_WIDTH + x;
			if(world -> grid[index].pixel_material == SAND){
				int below = index + SCREEN_WIDTH;
				int right = below + 1;
				int left = below - 1;
				if(below < SCREEN_WIDTH*SCREEN_HEIGHT && world -> grid[below].pixel_material == EMPTY){
					world -> grid[index] = cells[EMPTY];
					world -> grid[below] = cells[SAND];
				}
				else if(world->grid[below].pixel_material == SAND){
					if(world->grid[left].pixel_material == EMPTY && world->grid[right].pixel_material == EMPTY){
						int number = random();
						if(number == 0){
							world->grid[left] = cells[SAND];
							world->grid[index] = cells[EMPTY];
						}
						else{
							world->grid[right] = cells[SAND];
							world->grid[index] = cells[EMPTY];
						}
					}
					else if(world->grid[right].pixel_material == EMPTY){
							world->grid[right] = cells[SAND];
							world->grid[index] = cells[EMPTY];
						}
					else if(world->grid[left].pixel_material == EMPTY){
							world->grid[left] = cells[SAND];
							world->grid[index] = cells[EMPTY];
						}
					}
				}
			}
		}
		else{
			direction = !direction;
			for(int x = 0; x < SCREEN_WIDTH; x++){int index = y*SCREEN_WIDTH + x;
			if(world -> grid[index].pixel_material == SAND){
				int below = index + SCREEN_WIDTH;
				int right = below + 1;
				int left = below - 1;
				if(below < SCREEN_WIDTH*SCREEN_HEIGHT && world -> grid[below].pixel_material == EMPTY){
					world -> grid[index] = cells[EMPTY];
					world -> grid[below] = cells[SAND];
				}
				else if(world->grid[below].pixel_material == SAND){
					if(world->grid[left].pixel_material == EMPTY && world->grid[right].pixel_material == EMPTY){
						int number = random();
						if(number == 0){
							world->grid[left] = cells[SAND];
							world->grid[index] = cells[EMPTY];
						}
						else{
							world->grid[right] = cells[SAND];
							world->grid[index] = cells[EMPTY];
						}
					}
					else if(world->grid[right].pixel_material == EMPTY){
							world->grid[right] = cells[SAND];
							world->grid[index] = cells[EMPTY];
						}
					else if(world->grid[left].pixel_material == EMPTY){
							world->grid[left] = cells[SAND];
							world->grid[index] = cells[EMPTY];
						}
					}
				}
			}
		}
	}	
}

int main(int argc, char** argv){
	(void)argc;
	(void)argv;
	sdl_t* mainWindow = (sdl_t*)malloc(sizeof(sdl_t));
	sdl_t* materialWindow = (sdl_t*)malloc(sizeof(sdl_t));
	mainSim* mainsim = (mainSim*)malloc(sizeof(mainSim));

	if(!init_sdl(materialWindow, SCREEN_WIDTH/8, SCREEN_HEIGHT)){exit(EXIT_FAILURE);}
	if(!init_sdl(mainWindow, SCREEN_WIDTH, SCREEN_HEIGHT)){exit(EXIT_FAILURE);}

	mainsim -> WINDOW = mainWindow;

	if(!init_Sim(mainsim)){exit(EXIT_FAILURE);}
	srand(time(NULL));

	World* world = (World*)malloc(sizeof(World));
	world -> grid = (cell*)malloc(SCREEN_HEIGHT*SCREEN_WIDTH*sizeof(cell));
	
	Input* input = (Input*)malloc(sizeof(Input));

	input -> m1 = false;
	input -> m2 = false;

	init_texture(mainsim, world);
	
	while(mainsim -> state != QUIT){
		if(mainsim -> state  == PAUSED){continue;}
		doInput(mainsim, world, input);
		sandSim(world);
		update_texture(mainsim, world);
		update_mainWindow(mainsim);
		update_materialWindow(materialWindow);
		SDL_Delay(2);
	}
	cleanupMaterialWindow(materialWindow);
	cleanupMain(mainsim);
	SDL_Quit();
	exit(EXIT_SUCCESS);
}
