#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include "cpu.h"
#include "mmu.h"
#include "ppu.h"
#include "SDL_rotozoom.h"

int divider_clock = 0;
int timer_clock = 0;
int run_emulation = 1;
SDL_Surface *screen = NULL, *viewport = NULL;

void pause();
void update_timer(CPU *cpu, int cycles);
void handle_interrupts(CPU *cpu);
void handle_events(SDL_Event *event);


int main(int argc, char *argv[])
{
    //SDL_Surface *screen = NULL, *display_surface = NULL, *viewport = NULL;
    SDL_Surface *bmpSample = NULL; // *scaled_image = NULL;
    SDL_Rect bmpPos, viewport_rect = {2, 2, (160*2)-4, (144*2)-4};
    //SDL_Event event;

    bmpPos.x = 0;
    bmpPos.y = 0;

    SDL_Init(SDL_INIT_VIDEO);

    /* load icon before SDL_SetVideoMode */
    SDL_WM_SetIcon(SDL_LoadBMP("cb.bmp"), NULL);

    //screen = SDL_SetVideoMode(480, 432, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
    screen = SDL_SetVideoMode(512, 512, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
    //display_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 160, 140, 32, 0, 0, 0, 0);
    //display_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 256, 256, 32, 0, 0, 0, 0);
    viewport = SDL_CreateRGBSurface(SDL_SWSURFACE, 160*2, 144*2, 32, 0, 0, 0, 0);
    SDL_FillRect(viewport, NULL, 0xff0000);
    SDL_FillRect(viewport, &viewport_rect, 0xffffff);
    SDL_SetColorKey(viewport, SDL_SRCCOLORKEY, 0xffffff);

    SDL_WM_SetCaption("gameboy emulator", NULL);

    /* load Bitmap image in a surface */
    bmpSample = SDL_LoadBMP("cb.bmp");
    //SDL_SetColorKey(bmpSample, SDL_SRCCOLORKEY, SDL_MapRGB(bmpSample->format, 0, 0, 0));
    /* blit the image on screen */
    //SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 255, 255));
    SDL_BlitSurface(bmpSample, NULL, screen, &bmpPos);

    int cycles = 0; // cycles_counter = 0, cycles_per_frame = 69905;
    CPU cpu;
    memset(cpu.memory, 0, sizeof(cpu.memory));
    cpu_init(&cpu);
	cpu.memory[0xff44] = 0x90;  //	does this break stuff? is it needed for some games?
    //load_rom(&cpu, "rom/Dr. Mario (World).gb"); //cpu.pc=0;
    //load_mbc_rom(&cpu, "rom/Super Mario Land (World).gb");
    load_mbc_rom(&cpu, "rom/Pokemon - Red Version (USA, Europe) (SGB Enhanced).gb");
    //load_sram(&cpu, "rom/pokemon.sram", 1);
    //load_nintindo_logo(&cpu);


    /*int old_time = 0,  actual_time = 0;
    unsigned char *scy = &cpu.memory[0xff42];
    unsigned char *scx = &cpu.memory[0xff43];*/


    while(run_emulation)
    {

        //	step cpu if not halted
        /*if (!cpu.flags.halt)
            cycles = cpu_emulation(&cpu);
        else
            cycles = 1;*/

        cycles = cpu_emulation(&cpu);

        update_ppu(&cpu, cycles);

        update_timer(&cpu, cycles);

        handle_interrupts(&cpu);


        /*cycles_counter += cycles;
        if(cycles_counter >= cycles_per_frame)
        {
            cycles_counter -= cycles_per_frame;

            SDL_FillRect(display_surface, NULL, SDL_MapRGB(display_surface->format, 255, 255, 255));
            //draw_test(display_surface);
            //show_tile_data(&cpu, display_surface);
            draw_scrolling_background(&cpu, display_surface);
            draw_sprites(&cpu, display_surface);
            //render_frame(&cpu, display_surface);
            if ((scaled_image=zoomSurface(display_surface, 2, 2, 0))!=NULL)
            {
                SDL_BlitSurface(scaled_image, NULL, screen, &bmpPos);
                SDL_FreeSurface(scaled_image);
                viewport_rect.x = 0;//(*scx)*2;
                viewport_rect.y = (*scy)*2;
                SDL_BlitSurface(viewport, NULL, screen, &viewport_rect);
            }
            else
            {
                fprintf(stderr, "error : %s\n", SDL_GetError());
                fprintf(stdout, "error : %s\n", SDL_GetError());
            }

            SDL_Flip(screen);

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

        }*/

    }

    //save_sram(&cpu, "rom/pokemon.sram", 1);
    SDL_FreeSurface(bmpSample); /* free the surface */
    //SDL_FreeSurface(display_surface);
    SDL_FreeSurface(viewport);
    SDL_Quit();

    return EXIT_SUCCESS;
}


void pause()
{
    int paused = 1;
    SDL_Event event;

    while (paused)
    {
        SDL_WaitEvent(&event);
        switch(event.type)
        {
            case SDL_QUIT:
            paused  = 0;
        }
    }
}


void update_timer(CPU *cpu, int cycles)
{
    //  increment divider
	divider_clock += cycles/4;
	if (divider_clock >= 256)
    {
		divider_clock -= 256;
		cpu->memory[0xff04] += 1;
	}

	//	if timer is on
	if ((cpu->memory[0xff07] >> 2) & 0x1)
	{
		//	increase timer_clock
		timer_clock += cycles;

		//	which frequency is selected ?
		int freq = 4096;
		if ((cpu->memory[0xff07] & 3) == 1)
			freq = 262144;
		else if ((cpu->memory[0xff07] & 3) == 2)
			freq = 65536;
		else if ((cpu->memory[0xff07] & 3) == 3)
			freq = 16384;

		//	increment the timer according to the frequency (synched to the processed opcodes)
		while (timer_clock >= (4194304 / freq))
        {
			//	increase TIMA
			cpu->memory[0xff05] += 1;
			//	check TIMA for overflow
			if (cpu->memory[0xff05] == 0x00)
			{
				//	set timer interrupt request
				cpu->memory[0xff0f] = cpu->memory[0xff0f] | 4;
				//	reset timer to timer modulo
				cpu->memory[0xff05] =  cpu->memory[0xff06];
			}
			timer_clock -= (4194304 / freq);
		}
	}
}


void handle_interrupts(CPU *cpu)
{
	//	unHALT the system, once we have any interrupt
	if (cpu->memory[0xffff] & cpu->memory[0xff0f] && cpu->flags.halt)
    {
		cpu->flags.halt = 0;
	}

	//	handle interrupts
	if (cpu->flags.interrupts_enabled)
    {
		//	some interrupt is enabled and allowed
		if (cpu->memory[0xffff] & cpu->memory[0xff0f])
        {
			//	handle interrupts by priority (starting at bit 0 - vblank)

			//	v-blank interrupt
			if ((cpu->memory[0xffff] & 1) & cpu->memory[0xff0f] & 1)
			{
				cpu->sp--;
				memory_write(cpu, cpu->sp, cpu->pc >> 8);
				cpu->sp--;
				memory_write(cpu, cpu->sp, cpu->pc & 0xff);
				cpu->pc = 0x40;
				memory_write(cpu, 0xff0f, cpu->memory[0xff0f] & ~1);
			}

			//	lcd stat interrupt
			if ((cpu->memory[0xffff] & 2) & cpu->memory[0xff0f] & 2)
            {
				cpu->sp--;
				memory_write(cpu, cpu->sp, cpu->pc >> 8);
				cpu->sp--;
				memory_write(cpu, cpu->sp, cpu->pc & 0xff);
				cpu->pc = 0x48;
				memory_write(cpu, 0xff0f, cpu->memory[0xff0f] & ~2);
			}

			//	timer interrupt
			else if ((cpu->memory[0xffff] & 4) & cpu->memory[0xff0f] & 4)
            {
				cpu->sp--;
				memory_write(cpu, cpu->sp, cpu->pc >> 8);
				cpu->sp--;
				memory_write(cpu, cpu->sp, cpu->pc & 0xff);
				cpu->pc = 0x50;
				memory_write(cpu, 0xff0f, cpu->memory[0xff0f] & ~4);
			}

			//	clear main interrupts_enable and corresponding flag
			cpu->flags.interrupts_enabled = 0;
		}
	}
}


void handle_events(SDL_Event *event)
{
    if(SDL_PollEvent(event) == 1)
    {
        switch(event->type)
        {
            case SDL_QUIT:
                run_emulation = 0;
                break;
            // keyboard event
            case SDL_KEYDOWN:
                switch(event->key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        run_emulation = 0;
                        break;
                }
        }
    }
}

