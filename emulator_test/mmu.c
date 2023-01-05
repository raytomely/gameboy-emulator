#include <SDL/SDL.h>
#include "cpu.h"
#include "mmu.h"

unsigned char rom[0xFA000];
unsigned char romtype = 0x00;
unsigned char ram[0x10000];

unsigned char mbc1romNumber = 1;
unsigned char mbc1romMode = 0;
unsigned char mbc1ramNumber =0;
unsigned char mbc1ramEnabled = 0;

uint8_t mbc2romNumber = 1;
uint8_t mbc2romMode = 0;
uint8_t mbc2ramNumber = 0;
uint8_t mbc2ramEnabled = 0;

uint8_t mbc3romNumber = 1;
uint8_t mbc3romMode = 0;
uint8_t mbc3ramNumber = 0;
uint8_t mbc3ramEnabled = 0;

unsigned char memory_read(CPU *cpu, unsigned short address)
{
    /*if (address < 0x8000)
    {
        //       printf("reading ROM not allowed %x\n", address);
        return 0;
    }*/
    if (address > 0xffff+1)
    {
        //       printf("reading out of RAM not allowed %x\n", address);
        return 0;
    }

    //	MBC1
	if (romtype == 0x01 && mbc1romNumber && (address >= 0x4000 && address < 0x8000))
    {
		Uint32 target = (mbc1romNumber * 0x4000) + (address - 0x4000);
		return rom[target];
    }
    //	MBC2
	else if (romtype == 0x02)
    {
		//	ROM banking
		if (mbc2romNumber && (address >= 0x4000 && address < 0x8000))
        {
			Uint32 target = (mbc2romNumber * 0x4000) + (address - 0x4000);
			return rom[target];
		}
		else
			return cpu->memory[address];
    }
    //	MBC3
	else if (romtype == 0x03)
	{
		//	ROM banking
		if (mbc3romNumber && (address >= 0x4000 && address < 0x8000))
		{
			Uint32 target = (mbc3romNumber * 0x4000) + (address - 0x4000);
			return rom[target];
		}
		//	RAM / RTC banking
		else if (mbc3ramEnabled && mbc3ramNumber && (address >= 0xa000 && address < 0xc000))
		{
			//	RAM bank
			if (mbc3ramNumber < 0x08)
			{
				return ram[mbc3ramNumber * address];
			}
			//	RTC register
			else
            {
				printf("WARNING! Game tries to access unimplemented MBC3 RTC!!\n");
			}
		}
		else
			return cpu->memory[address];
	}

    return cpu->memory[address];
}


void memory_write(CPU *cpu, unsigned short address, unsigned char value)
{
    /*if (address < 0x8000)
    {
        //       printf("Writing ROM not allowed %x\n", address);
        return;
    }
    if (address > 0xffff+1)
    {
        //       printf("Writing out of RAM not allowed %x\n", address);
        return;
    }*/

    if (address == 0xff00)
    {
		cpu->memory[address] = read_input(value);
		return;
    }
    else if (address == 0xff46)
    {
		dma_oam_transfer(cpu);
		cpu->memory[address] = value;
	}

    if (romtype == 0x00)
    {
		//	make ROM readonly
		if (address >= 0x8000)
            cpu->memory[address] = value;
    }
    //	MBC1
	else if (romtype == 0x01)
    {
		//	external RAM enable / disable
		if (address < 0x2000)
		{
			mbc1ramEnabled = value > 0;
		}
		//	choose ROM bank nr (lower 5 bits, 0-4)
		else if (address < 0x4000)
        {
			mbc1romNumber = value & 0x1f;
			if (value == 0x00 || value == 0x20 || value == 0x40 || value == 0x60)
				mbc1romNumber = (value & 0x1f) + 1;
            if (mbc1romMode == 0)
				memcpy(&cpu->memory[0x4000], &rom[mbc1romNumber * 0x4000], 0x4000);
		}
		//	choose RAM bank nr OR ROM bank top 2 bits (5-6)
		else if (address < 0x6000)
		{
			//	mode: ROM bank 2 bits
			if (mbc1romMode == 0)
            {
				mbc1romNumber |= (value & 3) << 5;
				//memcpy(&cpu->memory[0x4000], &rom[mbc1romNumber * 0x4000], 0x4000);
            }
			//	mode: RAM bank selection
			else
				mbc1ramNumber = value & 3;
		}
		else if (address < 0x8000)
		{
			mbc1romMode = value > 0;
		}
		else
		{
            cpu->memory[address] = value;
		}
    }
    //	MBC2
	else if (romtype == 0x02)
    {
		//	external RAM enable / disable
		if (address < 0x2000)
		{
			mbc2ramEnabled = value > 0;
		}
		//	choose ROM bank nr (lower 5 bits, 0-4)
		else if (address < 0x4000)
		{
			mbc2romNumber = value & 0x1f;
			if (value == 0x00 || value == 0x20 || value == 0x40 || value == 0x60)
            {
				mbc2romNumber = (value & 0x1f) + 1;
				memcpy(&cpu->memory[0x4000], &rom[mbc2romNumber * 0x4000], 0x4000);
            }
		}
		else
		{
			cpu->memory[address] = value;
		}
	}
	//	MBC3
	else if (romtype == 0x03)
    {
		//	external RAM enable / disable
		if (address < 0x2000)
        {
			mbc3ramEnabled = value > 0;
		}
		//	choose ROM bank nr
		else if (address < 0x4000)
        {
			mbc3romNumber = value;
			if (value == 0x00)
				mbc3romNumber = 0x01;
            memcpy(&cpu->memory[0x4000], &rom[mbc3romNumber * 0x4000], 0x4000);
		}
		//	choose RAM bank nr OR RTC register (real time clock, for ingame cycles)
		else if (address < 0x6000)
        {
			mbc3ramNumber = value;
		}
		//	TODO: latch clock delay
		else if (address < 0x8000)
        {
            printf("watch\n");
		}
		//	write to RAM
		else if (mbc3ramEnabled && mbc3ramNumber && (address >= 0xa000 && address < 0xc000))
		{
			ram[mbc3romNumber * address] = value;
		}
		//	any other write
		else
        {
			cpu->memory[address] = value;
		}
    }
}

void dma_oam_transfer(CPU *cpu)
{
	unsigned short src = cpu->memory[0xff46] * 0x100;
	int i;
	for (i = 0; i < 40; i++)
    {
		cpu->memory[0xfe00 + (i * 4)] = cpu->memory[src + (i * 4)];
		cpu->memory[0xfe00 + (i * 4) + 1] = cpu->memory[src + (i * 4) + 1];
		cpu->memory[0xfe00 + (i * 4) + 2] = cpu->memory[src + (i * 4) + 2];
		cpu->memory[0xfe00 + (i * 4) + 3] = cpu->memory[src + (i * 4) + 3];
	}
}

unsigned char read_input(unsigned char val)
{
    Uint8* keys = SDL_GetKeyState(NULL);
    unsigned char joypad = 0x0;
    if ((val & 0x30) == 0x10)
    {
        joypad |= keys[SDLK_a]  |  keys[SDLK_t] ? 0 : 1;
        joypad |= (keys[SDLK_s] |  keys[SDLK_t] ? 0 : 1) << 1;
        joypad |= (keys[SDLK_x] |  keys[SDLK_t] ? 0 : 1) << 2;
        joypad |= (keys[SDLK_z] |  keys[SDLK_t] ? 0 : 1) << 3;
    }
    else if ((val & 0x30) == 0x20)
    {
        joypad |= keys[SDLK_RIGHT] ? 0 : 1;
        joypad |= (keys[SDLK_LEFT] ? 0 : 1) << 1;
        joypad |= (keys[SDLK_UP] ? 0 : 1) << 2;
        joypad |= (keys[SDLK_DOWN] ? 0 : 1) << 3;
    }
	val &= 0xf0;
	val |= 0xc0;
	return (val | joypad);
}


int load_rom(CPU *cpu, char* rom_name)
{
    FILE* file = fopen(rom_name, "rb");
    if (file)printf("file ok \n\n");
    else printf("\n\nfailed to load rom\n\n");
    int pos = 0;

    while (fread(&cpu->memory[pos], 1, 1, file))
    {
        pos++;
    }

    fclose(file);
    return file != NULL;
}


void rom_init(CPU *cpu)
{
	romtype = cpu->memory[0x0147];
	if (romtype < 0x04 && romtype != 0x00)
		romtype = 0x01;
	if (romtype >= 0x0f && romtype <= 0x13)
		romtype = 0x03;
	if (romtype == 0x05 || romtype == 0x06)
		romtype = 0x02;
}


void load_mbc_rom(CPU *cpu, char* rom_name)
{
    FILE* file = fopen(rom_name, "rb");
	int i, pos = 0;

	while (fread(&rom[pos], 1, 1, file))
    {
		pos++;
    }

	for (i = 0; i < 0x8000; i++)
    {
		cpu->memory[i] = rom[i];
	}

    fclose(file);
	rom_init(cpu);
}


void load_nintindo_logo(CPU *cpu)
{
    int i;
    for (i = 0; i < 0x134; i++)
    {
        cpu->memory[0x104+i] = cpu->memory[0xa8+i];
    }
}


void save_sram(CPU *cpu, char filename[], int sram_size_is_32k)
{
    FILE* file = fopen(filename, "wb" );
    if (sram_size_is_32k)
        fwrite(ram, sizeof(unsigned char), sizeof(unsigned char) * 0x8000, file);
    else
        fwrite(&cpu->memory[0xA000], sizeof(unsigned char), sizeof(unsigned char) * 0x2000, file);
    fclose(file);
}


void load_sram(CPU *cpu, char filename[], int sram_size_is_32k)
{
    FILE* file = fopen(filename, "rb" );
    if (sram_size_is_32k)
        fread(ram, sizeof(unsigned char), sizeof(unsigned char) * 0x8000, file);
    else
    {
        int pos = 0;
        while (pos < 0x2000)
        {
            fread(&cpu->memory[0xA000 + pos], sizeof(unsigned char), 1, file);
            pos++;
        }
    //fread(&cpu->memory[0xA000], sizeof(unsigned char), sizeof(unsigned char) * 0x2000, file);
    }
    fclose(file);
}

