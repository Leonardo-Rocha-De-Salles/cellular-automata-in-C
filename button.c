#include "button.h"
#include "SDL_render.h"


Button* create_button(int x, int y, int width, int height, SDL_Color normal_color, SDL_Color hover_color, SDL_Color pressed_color,  void (*on_click)(Input*)){
	Button* button = (Button*)malloc(sizeof(Button));
	button -> rect = (SDL_Rect){x,y, width, height};
	button -> color = normal_color;
	button -> hover_color = hover_color;
	button -> pressed_color = pressed_color;
	button -> is_hovered = false;
	button -> is_pressed = false;
	button -> on_click = on_click;
	return button;
}

void render_button(SDL_Renderer* renderer, Button* button){
	if(button -> is_pressed){
		SDL_SetRenderDrawColor(renderer,
			 button -> pressed_color.r, 
			 button -> pressed_color.g, 
			 button -> pressed_color.b, 
			 button -> pressed_color.a 
			 );
	} else if(button -> is_hovered){
		SDL_SetRenderDrawColor(renderer,
			 button -> hover_color.r, 
			 button -> hover_color.g, 
			 button -> hover_color.b, 
			 button -> hover_color.a);
	} else {
		SDL_SetRenderDrawColor(renderer,
			 button -> color.r, 
			 button -> color.g, 
			 button -> color.b, 
			 button -> color.a 
			 );
	}

	SDL_RenderFillRect(renderer, &button -> rect);
}



