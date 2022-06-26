#include <SDL/SDL.h>
#include "cpu.h"

int ppu_cycles = 0, last_ppu_mode = 0, line_rendered = 0, frame_rendered = 0;
int old_time = 0,  actual_time = 0;
unsigned char LY, LYC, SCX, SCY;
int tile_map, tile_data, tile_number;
Uint32 gb_colors[4] = {0xe0f8d0, 0x88c070, 0x346856, 0x081820};
SDL_Event event;
extern void handle_events(SDL_Event *event);
extern SDL_Surface *screen, *viewport;
SDL_Rect viewport_pos = {0, 0};


void setPixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp)
    {
        case 1:
            *p = pixel;
            break;
        case 2:
            *(Uint16 *)p = pixel;
            break;
        case 3:
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            {
                p[0] = (pixel >> 16) & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = pixel & 0xff;
            }
            else
            {
                p[0] = pixel & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = (pixel >> 16) & 0xff;
            }
            break;
        case 4:
            *(Uint32 *)p = pixel;
            break;
    }
}


void setPixel32_old(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    if (x >= surface->w || y >= surface->h || x < 0 || y < 0)
        return;
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    *(Uint32 *)p = pixel;
}


void setPixel32(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    if (x >= surface->w || y >= surface->h || x < 0 || y < 0)
        return;
    int bpp = surface->format->BytesPerPixel;
    x *= 2; y *= 2;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    *(Uint32 *)p = pixel;
     p = (Uint8 *)surface->pixels + y * surface->pitch + (x+1) * bpp;
    *(Uint32 *)p = pixel;
     p = (Uint8 *)surface->pixels + (y+1) * surface->pitch + x * bpp;
    *(Uint32 *)p = pixel;
        p = (Uint8 *)surface->pixels + (y+1) * surface->pitch + (x+1) * bpp;
    *(Uint32 *)p = pixel;
}


void draw_test(SDL_Surface *surface)
{
    int i;
    SDL_LockSurface(surface);
    for (i = 0; i < surface->h; i++)
        setPixel(surface, surface->w / 2, i, SDL_MapRGB(surface->format, 255, 0, 0));
    SDL_UnlockSurface(surface);
}


void draw_tile(unsigned char *tile_address, SDL_Surface *surface, int x, int y)
{
    int i, j, k = 0;
    unsigned char byte1, byte2, pixel;
    for (i = 0; i < 8; i ++)
    {
        byte1 = tile_address[0];
        byte2 = tile_address[1];
        tile_address += 2;

        for (j = 0; j < 8; j ++)
        {
            pixel = (byte1 >> 7) | ((byte2 >> 6) & 2);

            switch(pixel)
            {
                case 0:
                    setPixel32(surface, x+j, y+k, SDL_MapRGB(surface->format, 255, 0, 0));
                    break;
                case 1:
                    setPixel32(surface, x+j, y+k, SDL_MapRGB(surface->format, 0, 255, 0));
                    break;
                case 2:
                    setPixel32(surface, x+j, y+k, SDL_MapRGB(surface->format, 0, 0, 255));
                    break;
                case 3:
                    setPixel32(surface, x+j, y+k, SDL_MapRGB(surface->format, 0, 255, 255));
                    break;
            }
            byte1 <<= 1;
            byte2 <<= 1;
        }
        k++;
    }

}


void draw_background_tile(unsigned char *tile_address, SDL_Surface *surface, int x, int y, unsigned char palette)
{
    int i, j, k = 0;
    unsigned char byte1, byte2, pixel;

    for (i = 0; i < 8; i ++)
    {
        byte1 = tile_address[0];
        byte2 = tile_address[1];
        tile_address += 2;

        for (j = 0; j < 8; j ++)
        {
            pixel = (byte1 >> 7) | ((byte2 >> 6) & 2);

            switch(pixel)
            {
                case 0:
                    setPixel32(surface, x+j, y+k, gb_colors[palette & 3]);
                    break;
                case 1:
                    setPixel32(surface, x+j, y+k, gb_colors[(palette >> 2) & 3]);
                    break;
                case 2:
                    setPixel32(surface, x+j, y+k, gb_colors[(palette >> 4) & 3]);
                    break;
                case 3:
                    setPixel32(surface, x+j, y+k, gb_colors[(palette >> 6) & 3]);
                    break;
            }
            byte1 <<= 1;
            byte2 <<= 1;
        }
        k++;
    }

}


void draw_half_background_tile(unsigned char *tile_address, SDL_Surface *surface, int x, int y, unsigned char palette, unsigned char x_offset)
{
    int i, j, k = 0, j_start = 0, j_end = 8;
    unsigned char byte1, byte2, pixel;

    if (x <= 0)
        j_start = x_offset;
    else
        j_end = x_offset;

    for (i = 0; i < 8; i ++)
    {
        byte1 = tile_address[0] << j_start;
        byte2 = tile_address[1] << j_start;
        tile_address += 2;

        for (j = j_start; j < j_end; j ++)
        {
            pixel = (byte1 >> 7) | ((byte2 >> 6) & 2);

            switch(pixel)
            {
                case 0:
                    setPixel32(surface, x+j, y+k, gb_colors[palette & 3]);
                    break;
                case 1:
                    setPixel32(surface, x+j, y+k, gb_colors[(palette >> 2) & 3]);
                    break;
                case 2:
                    setPixel32(surface, x+j, y+k, gb_colors[(palette >> 4) & 3]);
                    break;
                case 3:
                    setPixel32(surface, x+j, y+k, gb_colors[(palette >> 6) & 3]);
                    break;
            }
            byte1 <<= 1;
            byte2 <<= 1;
        }
        k++;
    }

}


void draw_sprite_tile(unsigned char *tile_address, SDL_Surface *surface, int x, int y, unsigned char palette, int flip)
{
    int i, j, k, j_start ,j_increment, j_threshold, k_increment;
    int x_offset = (x < 0) ? -x : 0;
    unsigned char byte1, byte2, pixel;

    switch (flip)
    {
        case 0:  // no flip
            j_start = 0 + x_offset; j_increment = 1; j_threshold = 8; k = 0; k_increment = 1;
            break;
        case 1:  // x flip
            j_start = 7; j_increment = -1; j_threshold = -1 + x_offset; k = 0; k_increment = 1;
            break;
        case 2:  // y flip
            j_start = 0 + x_offset; j_increment = 1;  j_threshold = 8; k = 7; k_increment = -1;
            break;
        case 3:  // x and y flip
            j_start = 7; j_increment = -1; j_threshold = -1 + x_offset; k = 7; k_increment = -1;
            break;
    }

    for (i = 0; i < 8; i ++)
    {
        byte1 = tile_address[0];
        byte2 = tile_address[1];
        tile_address += 2;

        for (j = j_start; j != j_threshold; j += j_increment)
        {
            pixel = (byte1 >> 7) | ((byte2 >> 6) & 2);

            switch (pixel)
            {
               /*case 0:
                    setPixel32(surface, x+j, y+k, SDL_MapRGB(surface->format, 255, 0, 0));
                    break;*/
                case 1:
                    setPixel32(surface, x+j, y+k, gb_colors[(palette >> 2) & 3]);
                    break;
                case 2:
                    setPixel32(surface, x+j, y+k, gb_colors[(palette >> 4) & 3]);
                    break;
                case 3:
                    setPixel32(surface, x+j, y+k, gb_colors[(palette >> 6) & 3]);
                    break;
            }
            byte1 <<= 1;
            byte2 <<= 1;
        }
        k += k_increment;
    }

}


void show_tile_data(CPU *cpu, SDL_Surface *surface)
{
    SDL_LockSurface(surface);
    unsigned char *tile = &cpu->memory[0x8000];
    int i, x = 0, y = 0;
    for (i = 0; i < 192; i ++)
    {
        draw_tile(tile, surface, x, y);
        tile+=16;
        x+=8;
        if (x > surface->w)
        {
            x = 0;
            y+=8;
            if (y > surface->h)
                y=0;
        }
    }
    SDL_UnlockSurface(surface);
}


void draw_background(CPU *cpu, SDL_Surface *surface)
{
    int tile_map, tile_data, i, x = 0, y = 0;
    tile_map = (((cpu->memory[0xff40] >> 3) & 1) == 1) ? 0x9c00 : 0x9800;
    tile_data = (((cpu->memory[0xff40] >> 4) & 1) == 1) ? 0x8000 : 0x8800;

    unsigned char tile_number, *tile_address = NULL;

    for (i = 0; i < 1024; i++)
    {
        tile_number = cpu->memory[tile_map + i];

        if (tile_data == 0x8000)
        {
            tile_address = &cpu->memory[0x8000+(tile_number*16)];
        }
        else if (tile_data == 0x8800)
        {
            tile_address = &cpu->memory[0x9000+((char)tile_number*16)];
        }

        draw_background_tile(tile_address, surface, x, y, cpu->memory[0xff47]);
        x += 8;
        if (x >= 256)
        {
            x = 0;
            y += 8;
        }
    }
}


void draw_scrolling_background(CPU *cpu, SDL_Surface *surface)
{
    int tile_map, tile_data, i, j, k = 0, x = 0, y = 0, x_offset, tile_offset;
    tile_map = (((cpu->memory[0xff40] >> 3) & 1) == 1) ? 0x9c00 : 0x9800;
    tile_data = (((cpu->memory[0xff40] >> 4) & 1) == 1) ? 0x8000 : 0x8800;

    unsigned char tile_number, *tile_address = NULL;
    SCX = cpu->memory[0xff43];
    SCY = cpu->memory[0xff42];
    tile_offset = SCX / 8;
    x_offset = SCX % 8;
    x -= x_offset;

    for (i = 0; i < 18; i++)
    {
        for (j = 0; j < 20 + (x_offset != 0); j++)
        {
            //if (SCX >= 104)
            if ((tile_offset + j) > 31)
            {
                //tile_number = cpu->memory[tile_map + j + k - 32];
                tile_number = cpu->memory[tile_map + k + ((tile_offset + j) % 32)];
                //printf("tile_map= 0x%04x j= %d k= %d t= 0x%04x \n\n", tile_map + j + k, j, k, 0x9800 + k + ((SCX / 8 + j) % 32));
            }
            else
                tile_number = cpu->memory[tile_map + j + k + tile_offset];

            if (tile_data == 0x8000)
            {
                tile_address = &cpu->memory[0x8000+(tile_number*16)];
            }
            else if (tile_data == 0x8800)
            {
                tile_address = &cpu->memory[0x9000+((char)tile_number*16)];
            }

            if (j == 0 || j == 20)
                draw_half_background_tile(tile_address, surface, x, y, cpu->memory[0xff47], x_offset);
            else
                draw_background_tile(tile_address, surface, x, y, cpu->memory[0xff47]);
            x += 8;
        }
        x = 0 - x_offset;
        y += 8;
        k += 32;
    }
}


void draw_sprites(CPU *cpu, SDL_Surface *surface)
{
    int i, flip;
    int x, y, palette;
    for (i = 0xfe00; i < 0xfe9f + 1; i += 4)
    {
        y = cpu->memory[i]-16;
        x = cpu->memory[i+1]-8;
        tile_number = cpu->memory[i+2];
        palette = (cpu->memory[i+3] & 16) ? cpu->memory[0xff49] : cpu->memory[0xff48];
        flip = (cpu->memory[i+3] >> 5) & 3;

        //if (x>0 && y>0)
        if(cpu->memory[0xff40] & 0x04)
        {
            draw_sprite_tile(&cpu->memory[0x8000+((tile_number & 0xfe)*16)], surface, x, y, palette, flip);
            draw_sprite_tile(&cpu->memory[0x8000+((tile_number | 0x01)*16)], surface, x, y+8, palette, flip);
        }
        else
        {
            draw_sprite_tile(&cpu->memory[0x8000+(tile_number*16)], surface, x, y, palette, flip);
        }
    }

}


void draw_scanline(CPU *cpu, SDL_Surface *surface, int y)
{
    int tile_map, tile_data, i, j, x, y_offset, x_offset, tile_offset;
    int j_start = 0, j_end = 8;
    unsigned char tile_number, *tile_address = NULL;
    unsigned char byte1, byte2, pixel, palette = cpu->memory[0xff47];

    tile_map = (((cpu->memory[0xff40] >> 3) & 1) == 1) ? 0x9c00 : 0x9800;
    tile_data = (((cpu->memory[0xff40] >> 4) & 1) == 1) ? 0x8000 : 0x8800;
    x = 0;
    y_offset = (y % 8) * 2;
    SCX = cpu->memory[0xff43];
    tile_offset = SCX / 8;
    x_offset = SCX % 8;
    x -= x_offset;

    for (i = 0; i < 20 + (x_offset != 0); i++)
    {

        if ((tile_offset + i) > 31)
            tile_number = cpu->memory[tile_map + ((y / 8) * 32) + ((tile_offset + i)% 32)];
        else
            tile_number = cpu->memory[tile_map + i + ((y / 8) * 32) + tile_offset];

        if (tile_data == 0x8000)
        {
            tile_address = &cpu->memory[0x8000+(tile_number*16)];
        }
        else if (tile_data == 0x8800)
        {
            tile_address = &cpu->memory[0x9000+((char)tile_number*16)];
        }

        tile_address += y_offset;

        byte1 = tile_address[0];
        byte2 = tile_address[1];

        j_start = 0;
        j_end = 8;
        if (i == 0)
            j_start = x_offset;
        else if (i == 20)
            j_end = x_offset;

        for (j = j_start; j < j_end; j ++)
        {
            pixel = (byte1 >> 7) | ((byte2 >> 6) & 2);

            switch(pixel)
            {
                case 0:
                    setPixel32(surface, x+j, y, gb_colors[palette & 3]);
                    break;
                case 1:
                    setPixel32(surface, x+j, y, gb_colors[(palette >> 2) & 3]);
                    break;
                case 2:
                    setPixel32(surface, x+j, y, gb_colors[(palette >> 4) & 3]);
                    break;
                case 3:
                    setPixel32(surface, x+j, y, gb_colors[(palette >> 6) & 3]);
                    break;
            }
            byte1 <<= 1;
            byte2 <<= 1;
        }
        x += 8;
    }
}


void draw_frame(CPU *cpu, SDL_Surface *surface)
{
    SDL_LockSurface(surface);
    draw_scrolling_background(cpu, surface);
    draw_sprites(cpu, surface);
    SDL_UnlockSurface(surface);
}


void update_ppu(CPU *cpu, int cycles)
{
    ppu_cycles += cycles;

    //	Mode 2 / OAM mode
    if(ppu_cycles < 81)
    {
        cpu->memory[0xff41] = (cpu->memory[0xff41] & 0xfc) | 0x02;
        if ((cpu->memory[0xff41] >> 5) & 0x01)
            if (last_ppu_mode != 2)
                cpu->memory[0xff0f] = cpu->memory[0xff0f] | 2;
        last_ppu_mode = 2;
    }

    //	Mode 3 / Drawing
    else if (ppu_cycles < 253)
    {
		cpu->memory[0xff41] = (cpu->memory[0xff41] & 0xfc) | 0x03;
    }

    //	Mode 0 / H-Blank
	else if (ppu_cycles < 457)
    {
        cpu->memory[0xff41] = (cpu->memory[0xff41] & 0xfc) | 0x00;
        if ((cpu->memory[0xff41] >> 3) & 0x01)
            if (last_ppu_mode != 0)
                cpu->memory[0xff0f] = cpu->memory[0xff0f] | 2;
        last_ppu_mode = 0;
    }

    //	Mode 1 / V-Blank
	if (cpu->memory[0xff44] >= 144)
	{
	    cpu->memory[0xff41] = (cpu->memory[0xff41] & 0xfc) | 0x01;
        if ((cpu->memory[0xff41] >> 4) & 0x01)
            if (last_ppu_mode != 1)
                cpu->memory[0xff0f] = cpu->memory[0xff0f] | 2;
        last_ppu_mode = 1;
	}


	// as soon as in H-Blank -> draw current line
	if (ppu_cycles > 252 && cpu->memory[0xff44] < 145 && ! line_rendered)
    {
        draw_scanline(cpu, screen, cpu->memory[0xff44]);
        line_rendered = 1;
    }

    // if vblank, draw once (drawFlag)
	if (!frame_rendered && cpu->memory[0xff44] == 144)
    {
		//	only draw if display is enabled
		if (cpu->memory[0xff40] >> 7)
		{
			//	V-Blank interrupt IF
			cpu->memory[0xff0f] = cpu->memory[0xff0f] | 1;

			//draw_frame(cpu, surface);
			//SDL_FillRect(screen, NULL, 0XFFFFFF);
            //draw_scrolling_background(cpu, screen);
            draw_sprites(cpu, screen);
            SDL_BlitSurface(viewport, NULL, screen, &viewport_pos);
			SDL_Flip(screen);
            SDL_FillRect(screen, NULL, 0XFFFFFF);
			handle_events(&event);

            actual_time = SDL_GetTicks();
            if (actual_time - old_time < 16) // if less than 16 ms has passed
            {
                SDL_Delay(16 - (actual_time - old_time));
                old_time = SDL_GetTicks();
            }
            else
            {
                old_time = actual_time;
            }

			frame_rendered = 1;
		}
	}

	// if ppuCycles > line, ppCycles -= line => increase LY
	if (ppu_cycles > 456)
    {
		ppu_cycles -= 456;
		cpu->memory[0xff44] += 1;
		line_rendered = 0;
	}

    //  LYC==LC Coincidence flag
	LY = cpu->memory[0xff44];
	LYC = cpu->memory[0xff45];
	if (LY == LYC)
    {
		if ((cpu->memory[0xff41] >> 6) & 0x01)
			if (((cpu->memory[0xff41] >> 2) & 0x01) == 0)
			{
				cpu->memory[0xff0f] = cpu->memory[0xff0f] | 2;		//	trigger STAT interrupt
				cpu->memory[0xff41] = cpu->memory[0xff41] | 4;
			}
	}
	else
		cpu->memory[0xff41] = cpu->memory[0xff41] & ~4;


    //	if LY > 154, LY = 0; reset draw flag
	if (cpu->memory[0xff44] > 154)
    {
		cpu->memory[0xff44] = 0;
		frame_rendered = 0;
    }

}



