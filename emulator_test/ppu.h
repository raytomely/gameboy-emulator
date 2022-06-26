
void setPixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
void draw_test(SDL_Surface *surface);
void draw_tile(unsigned char *data, SDL_Surface *surface, int x, int y);
void draw_sprite_tile(unsigned char *tile_address, SDL_Surface *surface, int x, int y, unsigned char palette, int flip);
void draw_backgound_tile(unsigned char *tile_address, SDL_Surface *surface, int x, int y, unsigned char palette);
void draw_half_background_tile(unsigned char *tile_address, SDL_Surface *surface, int x, int y, unsigned char palette, unsigned char x_offset);
void show_tile_data(CPU *cpu, SDL_Surface *surface);
void draw_background(CPU *cpu, SDL_Surface *surface);
void draw_scrolling_background(CPU *cpu, SDL_Surface *surface);
void draw_sprites(CPU *cpu, SDL_Surface *surface);
void draw_scanline(CPU *cpu, SDL_Surface *surface, int y);
void draw_frame(CPU *cpu, SDL_Surface *surface);
void update_ppu(CPU *cpu, int cycles);
