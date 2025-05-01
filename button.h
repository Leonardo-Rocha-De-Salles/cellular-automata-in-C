#include <stdbool.h>
#include <SDL.h>

typedef struct Input Input;
typedef struct{
    SDL_Rect rect;
    bool is_hovered;
    bool is_pressed;
    SDL_Color color;    
    SDL_Color hover_color;
    SDL_Color pressed_color;
    void (*on_click)(Input* input);
}Button;

Button* create_button(int x, int y, int width, int height, SDL_Color normal_color, SDL_Color hover_color, SDL_Color pressed_color,  void (*on_click)(Input* input));

void render_button(SDL_Renderer* renderer,Button* button);


