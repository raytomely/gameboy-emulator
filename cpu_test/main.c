#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"

int divider_clock = 0;
int timer_clock = 0;

void test_flags(void);
void update_timer(CPU *cpu, int cycles);
void handle_interrupts(CPU *cpu);

int main()
{
    //test_flags();
    //int step = 0, break_point = 0x27;
    int cycles = 0;
    CPU cpu;
    cpu_init(&cpu);
    load_rom(&cpu, "blargg_cpu_test/instr_timing.gb");


    while(1)
    {
        //if (cpu.pc==break_point)step=1;
        //if (step)getchar();

        //	step cpu if not halted
        if (!cpu.flags.halt)
            cycles = cpu_emulation(&cpu);
        else
            cycles = 1;

        update_timer(&cpu, cycles);

        handle_interrupts(&cpu);
    }
    return 0;
}



void update_timer(CPU *cpu, int cycles)
{
    //  increment divider
	divider_clock += cycles;
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
