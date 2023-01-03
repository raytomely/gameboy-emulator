#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include "mmu.h"


int cpu_emulation(CPU *cpu)
{
    unsigned char *opcode = &cpu->memory[cpu->pc];
    int cycles = 1;//printf("opcode = 0x%02x  pc= %d 0x%04x \n\n",*opcode,cpu->pc,cpu->pc);
    /*if (cpu->pc > 0x00)//((cpu->h<<8) | cpu->l)<0x8A00)
    {
        if (*opcode == 0xcb)
            fprintf(stdout, "opcode = 0xcb= 0x%02x  pc= %d 0x%04x \n\n",opcode[1],cpu->pc,cpu->pc);
        else
            fprintf(stdout, "opcode = 0x%02x  pc= %d 0x%04x \n\n",*opcode,cpu->pc,cpu->pc);
    }*/
    switch (*opcode)
    {
        case 0x00:
            //  NOP : No OPeration.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->pc++;
            cycles = 4;
            break;

        case 0x01:
            //  LD  BC,u16 : Load value u16 into register BC.
            //  Cycles: 12
            //  Bytes: 3
            //  Flags: None affected.
            cpu->b = opcode[2];
            cpu->c = opcode[1];
            cpu->pc+=3;
            cycles = 12;
            break;

        case 0x02:
        {
            //  LD  (BC),A : Store value in register A into the byte pointed to by register BC.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            unsigned short address=(cpu->b<<8) | cpu->c;
            memory_write(cpu, address, cpu->a);
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x03:
            //  INC  BC : Increment value in register BC by 1.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            cpu->c++;
			if (cpu->c == 0)
				cpu->b++;
            cpu->pc+=1;
            cycles = 8;
            break;

        case 0x04:
            //  INC  B : Increment value in register B by 1.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            cpu->flags.h = (((cpu->b &0xf) + 1) & 0x10) == 0x10;
            cpu->b++;
            cpu->flags.z=(cpu->b == 0);
            cpu->flags.n=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x05:
            //  DEC  B : Decrement value in register B by 1.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            cpu->flags.h = (((cpu->b &0xf) - 1) & 0x10) == 0x10;
            cpu->b--;
            cpu->flags.z=(cpu->b == 0);
            cpu->flags.n=1;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x06:
            //  LD  B,u8 : Load value u8 into register B.
            //  Cycles: 8
            //  Bytes: 2
            //  Flags: None affected.
            cpu->b = opcode[1];
            cpu->pc+=2;
            cycles = 8;
            break;

        case 0x07:
        {
            //  RLCA : Rotate register A left.
            //  C <- [7 <- 0] <- [7]
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: 0
            //      N: 0
            //      H: 0
            //      C: Set according to result.
            unsigned char temp_a = cpu->a;
            cpu->a = (temp_a << 1) | (temp_a >> 7);
            cpu->flags.z = 0;
            cpu->flags.n = 0;
            cpu->flags.h = 0;
            cpu->flags.c = temp_a >> 7;
            cpu->pc+=1;
            cycles = 4;
            break;
        }

        case 0x08:
        {
            //  LD  (u16),SP : Store SP & $FF at address u16 and SP >> 8 at address u16 + 1.
            //  Cycles: 20
            //  Bytes: 3
            //  Flags: None affected.
            unsigned short address = (opcode[2] << 8) | opcode[1];
            memory_write(cpu, address, cpu->sp & 0xff);
            memory_write(cpu, address+1, cpu->sp >> 8);
            cpu->pc+=3;
            cycles = 20;
            break;
        }

        case 0x09:
        {
            //  ADD  HL,BC : Add the value in BC to HL.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags:
            //      N: 0
            //      H: Set if overflow from bit 11.
            //      C: Set if overflow from bit 15.
            unsigned int hl = ((cpu->h<<8) | cpu->l);
            unsigned int bc = ((cpu->b<<8) | cpu->c);
            unsigned int result = hl + bc;
            cpu->h = (result & 0Xffff) >> 8;
            cpu->l = (result & 0Xff);
            cpu->flags.n = 0;
            cpu->flags.h = (((hl & 0xfff) + (bc & 0xfff)) & 0x1000) == 0x1000;
            cpu->flags.c = result > 0xffff;
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x0a:
        {
            //  LD  A,(BC) : Load value in register A from the byte pointed to by register BC.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            unsigned short address=(cpu->b<<8) | cpu->c;
            cpu->a = memory_read(cpu, address);
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x0b:
            //  DEC  BC : Decrement value in register BC by 1.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            cpu->c--;
			if (cpu->c == 0xff)
				cpu->b--;
            cpu->pc+=1;
            cycles = 8;
            break;

        case 0x0c:
            //  INC  C : Increment value in register C by 1.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            cpu->flags.h = (((cpu->c &0xf) + 1) & 0x10) == 0x10;
            cpu->c++;
            cpu->flags.z=(cpu->c == 0);
            cpu->flags.n=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x0d:
            //  DEC  C : Decrement value in register C by 1.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            cpu->flags.h = (((cpu->c &0xf) - 1) & 0x10) == 0x10;
            cpu->c--;
            cpu->flags.z=(cpu->c == 0);
            cpu->flags.n=1;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x0e:
            //  LD  C,u8 : Load value u8 into register C.
            //  Cycles: 8
            //  Bytes: 2
            //  Flags: None affected.
            cpu->c = opcode[1];
            cpu->pc+=2;
            cycles = 8;
            break;

        case 0x0f:
        {
            //  RRCA : Rotate register A right.
            //  [0] -> [7 -> 0] -> C
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: 0
            //      N: 0
            //      H: 0
            //      C: Set according to result.
            unsigned char temp_a = cpu->a;
            cpu->a = (temp_a >> 1) | (temp_a << 7);
            cpu->flags.z = 0;
            cpu->flags.n = 0;
            cpu->flags.h = 0;
            cpu->flags.c = temp_a & 1;
            cpu->pc+=1;
            cycles = 4;
            break;
        }

         case 0x10:
            //  STOP : Enter CPU very low power mode.
            //  Also used to switch between double and normal speed CPU modes in GBC.
            //  Cycles: 4
            //  Bytes: 1 or 2 ?
            //  Flags: None affected.
            cpu->pc++;
            cycles = 4;
            break;

        case 0x11:
            //  LD  DE,u16 : Load value u16 into register DE.
            //  Cycles: 12
            //  Bytes: 3
            //  Flags: None affected.
            cpu->d = opcode[2];
            cpu->e = opcode[1];
            cpu->pc+=3;
            cycles = 12;
            break;

        case 0x12:
        {
            //  LD  (DE),A : Store value in register A into the byte pointed to by register DE.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            unsigned short address=(cpu->d<<8) | cpu->e;
            memory_write(cpu, address, cpu->a);
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x13:
            //  INC  DE : Increment value in register DE by 1.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            cpu->e++;
			if (cpu->e == 0)
				cpu->d++;
            cpu->pc+=1;
            cycles = 8;
            break;

        case 0x14:
            //  INC  D : Increment value in register D by 1.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            cpu->flags.h = (((cpu->d &0xf) + 1) & 0x10) == 0x10;
            cpu->d++;
            cpu->flags.z=(cpu->d == 0);
            cpu->flags.n=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x15:
            //  DEC  D : Decrement value in register D by 1.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            cpu->flags.h = (((cpu->d &0xf) - 1) & 0x10) == 0x10;
            cpu->d--;
            cpu->flags.z=(cpu->d == 0);
            cpu->flags.n=1;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x16:
            //  LD  D,u8 : Load value u8 into register D.
            //  Cycles: 8
            //  Bytes: 2
            //  Flags: None affected.
            cpu->d = opcode[1];
            cpu->pc+=2;
            cycles = 8;
            break;

        case 0x17:
        {
            //  RLA : Rotate register A left through carry.
            //  C <- [7 <- 0] <- C
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: 0
            //      N: 0
            //      H: 0
            //      C: Set according to result.
            unsigned char temp_a = cpu->a;
            cpu->a = (temp_a << 1) | (cpu->flags.c);
            cpu->flags.z = 0;
            cpu->flags.n = 0;
            cpu->flags.h = 0;
            cpu->flags.c = temp_a >> 7;
            cpu->pc+=1;
            cycles = 4;
            break;
        }

        case 0x18:
            //  JR  i8 : Relative Jump by adding i8 to the address of the instruction following the JR.
            //  To clarify, an operand of 0 is equivalent to no jumping.
            //  Cycles: 12
            //  Bytes: 2
            //  Flags: None affected.
            cpu->pc += (char)opcode[1];
            cpu->pc+=2;
            cycles = 12;
            break;

        case 0x19:
        {
            //  ADD  HL,DE : Add the value in DE to HL.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags:
            //      N: 0
            //      H: Set if overflow from bit 11.
            //      C: Set if overflow from bit 15.
            unsigned int hl = ((cpu->h<<8) | cpu->l);
            unsigned int de = ((cpu->d<<8) | cpu->e);
            unsigned int result = hl + de;
            cpu->h = (result & 0Xffff) >> 8;
            cpu->l = (result & 0Xff);
            cpu->flags.n = 0;
            cpu->flags.h = (((hl & 0xfff) + (de & 0xfff)) & 0x1000) == 0x1000;
            cpu->flags.c = result > 0xffff;
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x1a:
        {
            //  LD  A,(DE) : Load value in register A from the byte pointed to by register DE.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            unsigned short address=(cpu->d<<8) | cpu->e;
            cpu->a = memory_read(cpu, address);
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x1b:
            //  DEC  DE : Decrement value in register DE by 1.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            cpu->e--;
			if (cpu->e == 0xff)
				cpu->d--;
            cpu->pc+=1;
            cycles = 8;
            break;

        case 0x1c:
            //  INC  E : Increment value in register E by 1.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            cpu->flags.h = (((cpu->e &0xf) + 1) & 0x10) == 0x10;
            cpu->e++;
            cpu->flags.z=(cpu->e == 0);
            cpu->flags.n=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x1d:
            //  DEC  E : Decrement value in register E by 1.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            cpu->flags.h = (((cpu->e &0xf) - 1) & 0x10) == 0x10;
            cpu->e--;
            cpu->flags.z=(cpu->e == 0);
            cpu->flags.n=1;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x1e:
            //  LD  E,u8 : Load value u8 into register E.
            //  Cycles: 8
            //  Bytes: 2
            //  Flags: None affected.
            cpu->e = opcode[1];
            cpu->pc+=2;
            cycles = 8;
            break;

        case 0x1f:
        {
            //  RRA : Rotate register A right through carry.
            //  C -> [7 -> 0] -> C
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: 0
            //      N: 0
            //      H: 0
            //      C: Set according to result.
            unsigned char temp_a = cpu->a;
            cpu->a = (temp_a >> 1) | (cpu->flags.c << 7);
            cpu->flags.z = 0;
            cpu->flags.n = 0;
            cpu->flags.h = 0;
            cpu->flags.c = temp_a & 1;
            cpu->pc+=1;
            cycles = 4;
            break;
        }

        case 0x20:
            //  JR  NZ,i8 : Relative Jump by adding i8 to the current address if condition NZ is met.
            //  Cycles: 12 taken / 8 untaken
            //  Bytes: 2
            //  Flags: None affected.
            if (cpu->flags.z == 0)
            {
                cpu->pc+= 2 + (char)opcode[1];
                cycles = 12;
            }
            else
            {
                cpu->pc+= 2;
                cycles = 8;
            }
            break;

        case 0x21:
            //  LD  HL,u16 : Load value u16 into register HL.
            //  Cycles: 12
            //  Bytes: 3
            //  Flags: None affected.
            cpu->h = opcode[2];
            cpu->l = opcode[1];
            cpu->pc+=3;
            cycles = 12;
            break;

        case 0x22:
        {
            //  LD  (HL+),A : Store value in register A into the byte pointed to by register HL and increment HL.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            unsigned short address=(cpu->h<<8) | cpu->l;
            memory_write(cpu, address, cpu->a);
            cpu->l++;
			if (cpu->l == 0)
				cpu->h++;
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x23:
            //  INC  HL : Increment value in register HL by 1.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            cpu->l++;
			if (cpu->l == 0)
				cpu->h++;
            cpu->pc+=1;
            cycles = 8;
            break;

        case 0x24:
            //  INC  H : Increment value in register H by 1.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            cpu->flags.h = (((cpu->h &0xf) + 1) & 0x10) == 0x10;
            cpu->h++;
            cpu->flags.z=(cpu->h == 0);
            cpu->flags.n=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x25:
            //  DEC  H : Decrement value in register H by 1.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            cpu->flags.h = (((cpu->h &0xf) - 1) & 0x10) == 0x10;
            cpu->h--;
            cpu->flags.z=(cpu->h == 0);
            cpu->flags.n=1;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x26:
            //  LD  H,u8 : Load value u8 into register H.
            //  Cycles: 8
            //  Bytes: 2
            //  Flags: None affected.
            cpu->h = opcode[1];
            cpu->pc+=2;
            cycles = 8;
            break;

        case 0x27:
        {
            //  DAA : Decimal Adjust Accumulator
            //  to get a correct BCD representation after an arithmetic instruction.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      H: 0
            //      C: Set or reset depending on the operation.
            unsigned char correction = 0;
            if (cpu->flags.h || (!cpu->flags.n && (cpu->a & 0xf) > 9))
                correction |= 0x6;
            if (cpu->flags.c || (!cpu->flags.n && cpu->a > 0x99))
            {
                correction |= 0x60;
                cpu->flags.c = 1;
            }
            else cpu->flags.c = 0;
            cpu->a = cpu->a+(cpu->flags.n ? -correction : correction);
            cpu->a &= 0xff;
            cpu->flags.z = cpu->a == 0 ? 1 : 0;
            cpu->flags.h = 0;
            cpu->pc+=1;
            cycles = 4;
            break;
        }

        case 0x28:
            //  JR  Z,i8 : Relative Jump by adding i8 to the current address if condition Z is met.
            //  Cycles: 12 taken / 8 untaken
            //  Bytes: 2
            //  Flags: None affected.
            if (cpu->flags.z == 1)
            {
                cpu->pc+= 2 + (char)opcode[1];
                cycles = 12;
            }
            else
            {
                cpu->pc+= 2;
                cycles = 8;
            }
            break;

        case 0x29:
        {
            //  ADD  HL,HL : Add the value in HL to HL.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags:
            //      N: 0
            //      H: Set if overflow from bit 11.
            //      C: Set if overflow from bit 15.
            unsigned int hl = ((cpu->h<<8) | cpu->l);
            unsigned int result = hl + hl;
            cpu->h = (result & 0Xffff) >> 8;
            cpu->l = (result & 0Xff);
            cpu->flags.n = 0;
            cpu->flags.h = (((hl & 0xfff) + (hl & 0xfff)) & 0x1000) == 0x1000;
            cpu->flags.c = result > 0xffff;
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x2a:
        {
            //  LD  A,(HL+) : Load value in register A from the byte pointed to by register HL and increment HL.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            unsigned short address=(cpu->h<<8) | cpu->l;
            cpu->a = memory_read(cpu, address);
            cpu->l++;
			if (cpu->l == 0)
				cpu->h++;
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x2b:
            //  DEC  HL : Decrement value in register HL by 1.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            cpu->l--;
			if (cpu->l == 0xff)
				cpu->h--;
            cpu->pc+=1;
            cycles = 8;
            break;

        case 0x2c:
            //  INC  L : Increment value in register L by 1.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            cpu->flags.h = (((cpu->l &0xf) + 1) & 0x10) == 0x10;
            cpu->l++;
            cpu->flags.z=(cpu->l == 0);
            cpu->flags.n=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x2d:
            //  DEC  L : Decrement value in register L by 1.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            cpu->flags.h = (((cpu->l &0xf) - 1) & 0x10) == 0x10;
            cpu->l--;
            cpu->flags.z=(cpu->l == 0);
            cpu->flags.n=1;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x2e:
            //  LD  L,u8 : Load value u8 into register L.
            //  Cycles: 8
            //  Bytes: 2
            //  Flags: None affected.
            cpu->l = opcode[1];
            cpu->pc+=2;
            cycles = 8;
            break;

        case 0x2f:
            //  CPL : CompLement accumulator (A = ~A).
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      N: 1
            //      H: 1
            cpu->a = ~cpu->a;
            cpu->flags.n=1;
            cpu->flags.h=1;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x30:
            //  JR  NC,i8 : Relative Jump by adding i8 to the current address if condition NC is met.
            //  Cycles: 12 taken / 8 untaken
            //  Bytes: 2
            //  Flags: None affected.
            if (cpu->flags.c == 0)
            {
                cpu->pc+= 2 + (char)opcode[1];
                cycles = 12;
            }
            else
            {
                cpu->pc+= 2;
                cycles = 8;
            }
            break;

        case 0x31:
            //  LD  SP,u16 : Load value u16 into register SP.
            //  Cycles: 12
            //  Bytes: 3
            //  Flags: None affected.
            cpu->sp = (opcode[2]<<8) | opcode[1];
            cpu->pc+=3;
            cycles = 12;
            break;

        case 0x32:
        {
            //  LD  (HL-),A : Store value in register A into the byte pointed to by register HL and decrement HL.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            unsigned short address=(cpu->h<<8) | cpu->l;
            memory_write(cpu, address, cpu->a);
            cpu->l--;
			if (cpu->l == 0xff)
				cpu->h--;
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x33:
            //  INC  SP : Increment value in register SP by 1.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            cpu->sp++;
            cpu->pc+=1;
            cycles = 8;
            break;

        case 0x34:
        {
            //  INC  (HL) : Increment the byte pointed to by HL by 1.
            //  Cycles: 12
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            unsigned short address = (cpu->h<<8) | cpu->l;
            unsigned char value = memory_read(cpu, address);
            memory_write(cpu, address, value+1);
            cpu->flags.h = (((value &0xf) + 1) & 0x10) == 0x10;
            cpu->flags.z=(memory_read(cpu, address) == 0);
            cpu->flags.n=0;
            cpu->pc+=1;
            cycles = 12;
            break;
        }

        case 0x35:
        {
            //  DEC  (HL) : Decrement the byte pointed to by HL by 1.
            //  Cycles: 12
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            unsigned short address = (cpu->h<<8) | cpu->l;
            unsigned char value = memory_read(cpu, address);
            memory_write(cpu, address, value-1);
            cpu->flags.h = (((value &0xf) - 1) & 0x10) == 0x10;
            cpu->flags.z=(memory_read(cpu, address) == 0);
            cpu->flags.n=1;
            cpu->pc+=1;
            cycles = 12;
            break;
        }

        case 0x36:
        {
            //  LD  (HL),u8 : Store value u8 into the byte pointed to by register HL.
            //  Cycles: 12
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if overflow from bit 3.
            unsigned short address = (cpu->h<<8) | cpu->l;
            memory_write(cpu, address, opcode[1]);
            cpu->pc+=2;
            cycles = 12;
            break;
        }

        case 0x37:
            //  SCF : Set Carry Flag.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      N: 0
            //      H: 0
            //      C: 1
            cpu->flags.n = 0;
            cpu->flags.h = 0;
            cpu->flags.c = 1;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x38:
            //  JR  C,i8 : Relative Jump by adding i8 to the current address if condition C is met.
            //  Cycles: 12 taken / 8 untaken
            //  Bytes: 2
            //  Flags: None affected.
            if (cpu->flags.c == 1)
            {
                cpu->pc+= 2 + (char)opcode[1];
                cycles = 12;
            }
            else
            {
                cpu->pc+= 2;
                cycles = 8;
            }
            break;

        case 0x39:
        {
            //  ADD  HL,SP : Add the value in SP to HL.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags:
            //      N: 0
            //      H: Set if overflow from bit 11.
            //      C: Set if overflow from bit 15.
            unsigned int hl = ((cpu->h<<8) | cpu->l);
            unsigned int result = cpu->sp + hl;
            cpu->h = (result & 0Xffff) >> 8;
            cpu->l = (result & 0Xff);
            cpu->flags.n = 0;
            cpu->flags.h = (((cpu->sp & 0xfff) + (hl & 0xfff)) & 0x1000) == 0x1000;
            cpu->flags.c = result > 0xffff;
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x3a:
        {
            //  LD  A,(HL-) : Load value in register A from the byte pointed to by register HL and Decrement HL.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            unsigned short address=(cpu->h<<8) | cpu->l;
            cpu->a = memory_read(cpu, address);
            cpu->l--;
			if (cpu->l == 0xff)
				cpu->h--;
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x3b:
            //  DEC  SP : Decrement value in register SP by 1.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            cpu->sp--;
            cpu->pc+=1;
            cycles = 8;
            break;

        case 0x3c:
            //  INC  A : Increment value in register A by 1.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            cpu->flags.h = (((cpu->a &0xf) + 1) & 0x10) == 0x10;
            cpu->a++;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x3d:
            //  DEC  A : Decrement value in register A by 1.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            cpu->flags.h = (((cpu->a &0xf) - 1) & 0x10) == 0x10;
            cpu->a--;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=1;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x3e:
            //  LD  A,u8 : Load value u8 into register A.
            //  Cycles: 8
            //  Bytes: 2
            //  Flags: None affected.
            cpu->a = opcode[1];
            cpu->pc+=2;
            cycles = 8;
            break;

        case 0x3f:
            //  CCF : Complement Carry Flag.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      N: 0
            //      H: 0
            //      C: Inverted.
            cpu->flags.n = 0;
            cpu->flags.h = 0;
            cpu->flags.c = ~cpu->flags.c & 1;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x40:
            //  LD  B,B : Load (copy) value in register B into register B.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->b = cpu->b;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x41:
            //  LD  B,C : Load (copy) value in register C into register B.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->b = cpu->c;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x42:
            //  LD  B,D : Load (copy) value in register D into register B.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->b = cpu->d;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x43:
            //  LD  B,E : Load (copy) value in register E into register B.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->b = cpu->e;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x44:
            //  LD  B,H : Load (copy) value in register H into register B.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->b = cpu->h;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x45:
            //  LD  B,L : Load (copy) value in register L into register B.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->b = cpu->l;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x46:
        {
            //  LD  B,(HL) : Load value in register B from the byte pointed to by register HL.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            unsigned short address=(cpu->h<<8) | cpu->l;
            cpu->b = memory_read(cpu, address);
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x47:
            //  LD  B,A : Load (copy) value in register A into register B.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->b = cpu->a;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x48:
            //  LD  C,B : Load (copy) value in register B into register C.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->c = cpu->b;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x49:
            //  LD  C,C : Load (copy) value in register C into register C.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->c = cpu->c;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x4a:
            //  LD  C,D : Load (copy) value in register D into register C.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->c = cpu->d;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x4b:
            //  LD  C,E : Load (copy) value in register E into register C.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->c = cpu->e;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x4c:
            //  LD  C,H : Load (copy) value in register H into register C.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->c = cpu->h;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x4d:
            //  LD  C,L : Load (copy) value in register L into register C.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->c = cpu->l;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x4e:
        {
            //  LD  C,(HL) : Load value in register C from the byte pointed to by register HL.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            unsigned short address=(cpu->h<<8) | cpu->l;
            cpu->c = memory_read(cpu, address);
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x4f:
            //  LD  C,A : Load (copy) value in register A into register C.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->c = cpu->a;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x50:
            //  LD  D,B : Load (copy) value in register B into register D.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->d = cpu->b;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x51:
            //  LD  D,C : Load (copy) value in register C into register D.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->d = cpu->c;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x52:
            //  LD  D,D : Load (copy) value in register D into register D.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->d = cpu->d;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x53:
            //  LD  D,E : Load (copy) value in register E into register D.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->d = cpu->e;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x54:
            //  LD  D,H : Load (copy) value in register H into register D.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->d = cpu->h;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x55:
            //  LD  D,L : Load (copy) value in register L into register D.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->d = cpu->l;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x56:
        {
            //  LD  D,(HL) : Load value in register D from the byte pointed to by register HL.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            unsigned short address=(cpu->h<<8) | cpu->l;
            cpu->d = memory_read(cpu, address);
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x57:
            //  LD  D,A : Load (copy) value in register A into register D.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->d = cpu->a;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x58:
            //  LD  E,B : Load (copy) value in register B into register E.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->e = cpu->b;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x59:
            //  LD  E,C : Load (copy) value in register C into register E.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->e = cpu->c;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x5a:
            //  LD  E,D : Load (copy) value in register D into register E.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->e = cpu->d;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x5b:
            //  LD  E,E : Load (copy) value in register E into register E.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->e = cpu->e;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x5c:
            //  LD  E,H : Load (copy) value in register H into register E.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->e = cpu->h;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x5d:
            //  LD  E,L : Load (copy) value in register L into register E.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->e = cpu->l;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x5e:
        {
            //  LD  E,(HL) : Load value in register E from the byte pointed to by register HL.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            unsigned short address=(cpu->h<<8) | cpu->l;
            cpu->e = memory_read(cpu, address);
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x5f:
            //  LD  E,A : Load (copy) value in register A into register E.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->e = cpu->a;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x60:
            //  LD  H,B : Load (copy) value in register B into register H.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->h = cpu->b;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x61:
            //  LD  H,C : Load (copy) value in register C into register H.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->h = cpu->c;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x62:
            //  LD  H,D : Load (copy) value in register D into register H.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->h = cpu->d;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x63:
            //  LD  H,E : Load (copy) value in register E into register H.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->h = cpu->e;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x64:
            //  LD  H,H : Load (copy) value in register H into register H.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->h = cpu->h;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x65:
            //  LD  H,L : Load (copy) value in register L into register H.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->h = cpu->l;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x66:
        {
            //  LD  H,(HL) : Load value in register H from the byte pointed to by register HL.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            unsigned short address=(cpu->h<<8) | cpu->l;
            cpu->h = memory_read(cpu, address);
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x67:
            //  LD  H,A : Load (copy) value in register A into register H.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->h = cpu->a;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x68:
            //  LD  L,B : Load (copy) value in register B into register L.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->l = cpu->b;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x69:
            //  LD  L,C : Load (copy) value in register C into register L.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->l = cpu->c;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x6a:
            //  LD  L,D : Load (copy) value in register D into register L.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->l = cpu->d;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x6b:
            //  LD  L,E : Load (copy) value in register E into register L.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->l = cpu->e;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x6c:
            //  LD  L,H : Load (copy) value in register H into register L.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->l = cpu->h;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x6d:
            //  LD  L,L : Load (copy) value in register L into register L.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->l = cpu->l;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x6e:
        {
            //  LD  L,(HL) : Load value in register L from the byte pointed to by register HL.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            unsigned short address=(cpu->h<<8) | cpu->l;
            cpu->l = memory_read(cpu, address);
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x6f:
            //  LD  L,A : Load (copy) value in register A into register L.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->l = cpu->a;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x70:
        {
            //  LD  (HL),B : Store value in register B into the byte pointed to by register HL.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            unsigned short address=(cpu->h<<8) | cpu->l;
            memory_write(cpu, address, cpu->b);
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x71:
        {
            //  LD  (HL),C : Store value in register C into the byte pointed to by register HL.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            unsigned short address=(cpu->h<<8) | cpu->l;
            memory_write(cpu, address, cpu->c);
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x72:
        {
            //  LD  (HL),D : Store value in register D into the byte pointed to by register HL.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            unsigned short address=(cpu->h<<8) | cpu->l;
            memory_write(cpu, address, cpu->d);
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x73:
        {
            //  LD  (HL),E : Store value in register E into the byte pointed to by register HL.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            unsigned short address=(cpu->h<<8) | cpu->l;
            memory_write(cpu, address, cpu->e);
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x74:
        {
            //  LD  (HL),H : Store value in register H into the byte pointed to by register HL.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            unsigned short address=(cpu->h<<8) | cpu->l;
            memory_write(cpu, address, cpu->h);
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x75:
        {
            //  LD  (HL),L : Store value in register L into the byte pointed to by register HL.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            unsigned short address=(cpu->h<<8) | cpu->l;
            memory_write(cpu, address, cpu->l);
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x76:
            //  HALT : Enter CPU low-power consumption mode until an interrupt occurs. The exact behavior of this instruction depends on the state of the IME flag.
            //       IME set: The CPU enters low-power mode until after an interrupt is about to be serviced. The handler is executed normally, and the CPU resumes execution after the HALT when that returns.
            //       IME not set:  The behavior depends on whether an interrupt is pending (i.e. ‘[IE] & [IF]’ is non-zero).
            //                  None pending: As soon as an interrupt becomes pending, the CPU resumes execution. This is like the above, except that the handler is not called.
            //                  Some pending: The CPU continues execution after the HALT, but the byte after it is read twice in a row (PC is not incremented, due to a hardware bug).
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->flags.halt = 1;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x77:
        {
            //  LD  (HL),A : Store value in register A into the byte pointed to by register HL.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            unsigned short address=(cpu->h<<8) | cpu->l;
            memory_write(cpu, address, cpu->a);
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x78:
            //  LD  A,B : Load (copy) value in register B into register A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->a = cpu->b;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x79:
            //  LD  A,C : Load (copy) value in register C into register A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->a = cpu->c;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x7a:
            //  LD  A,D : Load (copy) value in register D into register A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->a = cpu->d;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x7b:
            //  LD  A,E : Load (copy) value in register E into register A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->a = cpu->e;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x7c:
            //  LD  A,H : Load (copy) value in register H into register A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->a = cpu->h;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x7d:
            //  LD  A,L : Load (copy) value in register L into register A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->a = cpu->l;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x7e:
        {
            //  LD  A,(HL) : Load value in register A from the byte pointed to by register HL.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            unsigned short address=(cpu->h<<8) | cpu->l;
            cpu->a = memory_read(cpu, address);
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x7f:
            //  LD  A,A : Load (copy) value in register A into register A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->a = cpu->a;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x80:
            //  ADD  A,B : Add the value in B to A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            //      C: Set if overflow from bit 7.
            cpu->flags.h = (((cpu->a &0xf) + (cpu->b &0xf)) & 0x10) == 0x10;
            cpu->flags.c = (cpu->a + cpu->b) > 0xff;
            cpu->a += cpu->b;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x81:
            //  ADD  A,C : Add the value in C to A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            //      C: Set if overflow from bit 7.
            cpu->flags.h = (((cpu->a &0xf) + (cpu->c &0xf)) & 0x10) == 0x10;
            cpu->flags.c = (cpu->a + cpu->c) > 0xff;
            cpu->a += cpu->c;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x82:
            //  ADD  A,D : Add the value in D to A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            //      C: Set if overflow from bit 7.
            cpu->flags.h = (((cpu->a &0xf) + (cpu->d &0xf)) & 0x10) == 0x10;
            cpu->flags.c = (cpu->a + cpu->d) > 0xff;
            cpu->a += cpu->d;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x83:
            //  ADD  A,E : Add the value in E to A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            //      C: Set if overflow from bit 7.
            cpu->flags.h = (((cpu->a &0xf) + (cpu->e &0xf)) & 0x10) == 0x10;
            cpu->flags.c = (cpu->a + cpu->e) > 0xff;
            cpu->a += cpu->e;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x84:
            //  ADD  A,H : Add the value in H to A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            //      C: Set if overflow from bit 7.
            cpu->flags.h = (((cpu->a &0xf) + (cpu->h &0xf)) & 0x10) == 0x10;
            cpu->flags.c = (cpu->a + cpu->h) > 0xff;
            cpu->a += cpu->h;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x85:
            //  ADD  A,L : Add the value in L to A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            //      C: Set if overflow from bit 7.
            cpu->flags.h = (((cpu->a &0xf) + (cpu->l &0xf)) & 0x10) == 0x10;
            cpu->flags.c = (cpu->a + cpu->l) > 0xff;
            cpu->a += cpu->l;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x86:
        {
            //  ADD  A,(HL) : Add the byte pointed to by HL to A.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            //      C: Set if overflow from bit 7.
            unsigned short address=(cpu->h<<8) | cpu->l;
            unsigned char value = memory_read(cpu, address);
            cpu->flags.h = (((cpu->a &0xf) + (value &0xf)) & 0x10) == 0x10;
            cpu->flags.c = (cpu->a + value) > 0xff;
            cpu->a += value;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x87:
            //  ADD  A,A : Add the value in A to A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            //      C: Set if overflow from bit 7.
            cpu->flags.h = (((cpu->a &0xf) + (cpu->a &0xf)) & 0x10) == 0x10;
            cpu->flags.c = (cpu->a + cpu->a) > 0xff;
            cpu->a += cpu->a;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x88:
        {
            //  ADC  A,B : Add the value in B plus the carry flag to A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            //      C: Set if overflow from bit 7.
            unsigned char result = cpu->a + cpu->b + cpu->flags.c;
            cpu->flags.h = (((cpu->a &0xf) + (cpu->b &0xf) + cpu->flags.c) & 0x10) == 0x10;
            cpu->flags.c = (cpu->a + cpu->b + cpu->flags.c) > 0xff;
            cpu->a = result;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->pc+=1;
            cycles = 4;
            break;
        }

        case 0x89:
        {
            //  ADC  A,C : Add the value in C plus the carry flag to A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            //      C: Set if overflow from bit 7.
            unsigned char result = cpu->a + cpu->c + cpu->flags.c;
            cpu->flags.h = (((cpu->a &0xf) + (cpu->c &0xf) + cpu->flags.c) & 0x10) == 0x10;
            cpu->flags.c = (cpu->a + cpu->c + cpu->flags.c) > 0xff;
            cpu->a = result;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->pc+=1;
            cycles = 4;
            break;
        }

        case 0x8a:
        {
            //  ADC  A,D : Add the value in D plus the carry flag to A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            //      C: Set if overflow from bit 7.
            unsigned char result = cpu->a + cpu->d + cpu->flags.c;
            cpu->flags.h = (((cpu->a &0xf) + (cpu->d &0xf) + cpu->flags.c) & 0x10) == 0x10;
            cpu->flags.c = (cpu->a + cpu->d + cpu->flags.c) > 0xff;
            cpu->a = result;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->pc+=1;
            cycles = 4;
            break;
        }

        case 0x8b:
        {
            //  ADC  A,E : Add the value in E plus the carry flag to A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            //      C: Set if overflow from bit 7.
            unsigned char result = cpu->a + cpu->e + cpu->flags.c;
            cpu->flags.h = (((cpu->a &0xf) + (cpu->e &0xf) + cpu->flags.c) & 0x10) == 0x10;
            cpu->flags.c = (cpu->a + cpu->e + cpu->flags.c) > 0xff;
            cpu->a = result;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->pc+=1;
            cycles = 4;
            break;
        }

        case 0x8c:
        {
            //  ADC  A,H : Add the value in H plus the carry flag to A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            //      C: Set if overflow from bit 7.
            unsigned char result = cpu->a + cpu->h + cpu->flags.c;
            cpu->flags.h = (((cpu->a &0xf) + (cpu->h &0xf) + cpu->flags.c) & 0x10) == 0x10;
            cpu->flags.c = (cpu->a + cpu->h + cpu->flags.c) > 0xff;
            cpu->a = result;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->pc+=1;
            cycles = 4;
            break;
        }

        case 0x8d:
        {
            //  ADC  A,L : Add the value in L plus the carry flag to A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            //      C: Set if overflow from bit 7.
            unsigned char result = cpu->a + cpu->l + cpu->flags.c;
            cpu->flags.h = (((cpu->a &0xf) + (cpu->l &0xf) + cpu->flags.c) & 0x10) == 0x10;
            cpu->flags.c = (cpu->a + cpu->l + cpu->flags.c) > 0xff;
            cpu->a = result;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->pc+=1;
            cycles = 4;
            break;
        }

        case 0x8e:
        {
            //  ADC  A,(HL) : Add the byte pointed to by HL plus the carry flag to A.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            //      C: Set if overflow from bit 7.
            unsigned short address=(cpu->h<<8) | cpu->l;
            unsigned char value = memory_read(cpu, address);
            unsigned char result = cpu->a + value + cpu->flags.c;
            cpu->flags.h = (((cpu->a &0xf) + (value &0xf) + cpu->flags.c) & 0x10) == 0x10;
            cpu->flags.c = (cpu->a + value + cpu->flags.c) > 0xff;
            cpu->a = result;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x8f:
        {
            //  ADC  A,A : Add the value in A plus the carry flag to A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            //      C: Set if overflow from bit 7.
            unsigned char result = cpu->a + cpu->a + cpu->flags.c;
            cpu->flags.h = (((cpu->a &0xf) + (cpu->a &0xf) + cpu->flags.c) & 0x10) == 0x10;
            cpu->flags.c = (cpu->a + cpu->a + cpu->flags.c) > 0xff;
            cpu->a = result;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->pc+=1;
            cycles = 4;
            break;
        }

        case 0x90:
            //  SUB  A,B : Subtract the value in B from A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (set if B > A).
            cpu->flags.h = (cpu->b &0xf) > (cpu->a &0xf);
            cpu->flags.c = cpu->b > cpu->a;
            cpu->a -= cpu->b;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=1;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x91:
            //  SUB  A,C : Subtract the value in C from A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (set if C > A).
            cpu->flags.h = (cpu->c &0xf) > (cpu->a &0xf);
            cpu->flags.c = cpu->c > cpu->a;
            cpu->a -= cpu->c;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=1;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x92:
            //  SUB  A,D : Subtract the value in D from A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (set if D > A).
            cpu->flags.h = (cpu->d &0xf) > (cpu->a &0xf);
            cpu->flags.c = cpu->d > cpu->a;
            cpu->a -= cpu->d;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=1;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x93:
            //  SUB  A,E : Subtract the value in E from A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (set if E > A).
            cpu->flags.h = (cpu->e &0xf) > (cpu->a &0xf);
            cpu->flags.c = cpu->e > cpu->a;
            cpu->a -= cpu->e;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=1;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x94:
            //  SUB  A,H : Subtract the value in H from A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (set if H > A).
            cpu->flags.h = (cpu->h &0xf) > (cpu->a &0xf);
            cpu->flags.c = cpu->h > cpu->a;
            cpu->a -= cpu->h;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=1;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x95:
            //  SUB  A,L : Subtract the value in L from A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (set if L > A).
            cpu->flags.h = (cpu->l &0xf) > (cpu->a &0xf);
            cpu->flags.c = cpu->l > cpu->a;
            cpu->a -= cpu->l;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=1;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x96:
        {
            //  SUB  A,(HL) : Subtract the byte pointed to by HL from A.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (set if (HL) > A).
            unsigned short address=(cpu->h<<8) | cpu->l;
            unsigned char value = memory_read(cpu, address);
            cpu->flags.h = (value &0xf) > (cpu->a &0xf);
            cpu->flags.c = value > cpu->a;
            cpu->a -= value;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=1;
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x97:
            //  SUB  A,A : Subtract the value in A from A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (set if A > A).
            cpu->flags.h = (cpu->a &0xf) > (cpu->a &0xf);
            cpu->flags.c = cpu->a > cpu->a;
            cpu->a -= cpu->a;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=1;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0x98:
        {
            //  SBC  A,B : Subtract the value in B and the carry flag from A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (set if B > A).
            unsigned char result = cpu->a - (cpu->b + cpu->flags.c);
            cpu->flags.h = ((cpu->b &0xf) + cpu->flags.c) > (cpu->a &0xf);
            cpu->flags.c = (cpu->b + cpu->flags.c) > cpu->a;
            cpu->a = result;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=1;
            cpu->pc+=1;
            cycles = 4;
            break;
        }

        case 0x99:
        {
            //  SBC  A,C : Subtract the value in C and the carry flag from A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (set if C > A).
            unsigned char result = cpu->a - (cpu->c + cpu->flags.c);
            cpu->flags.h = ((cpu->c &0xf) + cpu->flags.c) > (cpu->a &0xf);
            cpu->flags.c = (cpu->c + cpu->flags.c) > cpu->a;
            cpu->a = result;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=1;
            cpu->pc+=1;
            cycles = 4;
            break;
        }

        case 0x9a:
        {
            //  SBC  A,D : Subtract the value in D and the carry flag from A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (set if D > A).
            unsigned char result = cpu->a - (cpu->d + cpu->flags.c);
            cpu->flags.h = ((cpu->d &0xf) + cpu->flags.c) > (cpu->a &0xf);
            cpu->flags.c = (cpu->d + cpu->flags.c) > cpu->a;
            cpu->a = result;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=1;
            cpu->pc+=1;
            cycles = 4;
            break;
        }

        case 0x9b:
        {
            //  SBC  A,E : Subtract the value in E and the carry flag from A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (set if E > A).
            unsigned char result = cpu->a - (cpu->e + cpu->flags.c);
            cpu->flags.h = ((cpu->e &0xf) + cpu->flags.c) > (cpu->a &0xf);
            cpu->flags.c = (cpu->e + cpu->flags.c) > cpu->a;
            cpu->a = result;;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=1;
            cpu->pc+=1;
            cycles = 4;
            break;
        }

        case 0x9c:
        {
            //  SBC  A,H : Subtract the value in H and the carry flag from A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (set if H > A).
            unsigned char result = cpu->a - (cpu->h + cpu->flags.c);
            cpu->flags.h = ((cpu->h &0xf) + cpu->flags.c) > (cpu->a &0xf);
            cpu->flags.c = (cpu->h + cpu->flags.c) > cpu->a;
            cpu->a = result;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=1;
            cpu->pc+=1;
            cycles = 4;
            break;
        }

        case 0x9d:
        {
            //  SBC  A,L : Subtract the value in L and the carry flag from A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (set if L > A).
            unsigned char result = cpu->a - (cpu->l + cpu->flags.c);
            cpu->flags.h = ((cpu->l &0xf) + cpu->flags.c) > (cpu->a &0xf);
            cpu->flags.c = (cpu->l + cpu->flags.c) > cpu->a;
            cpu->a = result;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=1;
            cpu->pc+=1;
            cycles = 4;
            break;
        }

        case 0x9e:
        {
            //  SBC  A,(HL) : Subtract the byte pointed to by HL and the carry flag from A.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (set if (HL) > A).
            unsigned short address=(cpu->h<<8) | cpu->l;
            unsigned char value = memory_read(cpu, address);
            unsigned char result = cpu->a - (value + cpu->flags.c);
            cpu->flags.h = ((value &0xf) + cpu->flags.c) > (cpu->a &0xf);
            cpu->flags.c = (value + cpu->flags.c) > cpu->a;
            cpu->a = result;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=1;
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0x9f:
        {
            //  SBC  A,A : Subtract the value in A and the carry flag from A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (set if A > A).
            unsigned char result = cpu->a - (cpu->a + cpu->flags.c);
            cpu->flags.h = ((cpu->a &0xf) + cpu->flags.c) > (cpu->a &0xf);
            cpu->flags.c = (cpu->a + cpu->flags.c) > cpu->a;
            cpu->a = result;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=1;
            cpu->pc+=1;
            cycles = 4;
            break;
        }

        case 0xa0:
            //  AND  A,B : Bitwise AND between the value in B and A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 1
            //      C: 0
            cpu->a = cpu->a & cpu->b;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=1;
            cpu->flags.c=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xa1:
            //  AND  A,C : Bitwise AND between the value in C and A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 1
            //      C: 0
            cpu->a = cpu->a & cpu->c;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=1;
            cpu->flags.c=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xa2:
            //  AND  A,D : Bitwise AND between the value in D and A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 1
            //      C: 0
            cpu->a = cpu->a & cpu->d;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=1;
            cpu->flags.c=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xa3:
            //  AND  A,E : Bitwise AND between the value in E and A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 1
            //      C: 0
            cpu->a = cpu->a & cpu->e;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=1;
            cpu->flags.c=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xa4:
            //  AND  A,H : Bitwise AND between the value in H and A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 1
            //      C: 0
            cpu->a = cpu->a & cpu->h;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=1;
            cpu->flags.c=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xa5:
            //  AND  A,L : Bitwise AND between the value in L and A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 1
            //      C: 0
            cpu->a = cpu->a & cpu->l;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=1;
            cpu->flags.c=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xa6:
        {
            //  AND  A,(HL) : Bitwise AND between the byte pointed to by HL and A.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 1
            //      C: 0
            unsigned short address=(cpu->h<<8) | cpu->l;
            unsigned char value = memory_read(cpu, address);
            cpu->a = cpu->a & value;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=1;
            cpu->flags.c=0;
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0xa7:
            //  AND  A,A : Bitwise AND between the value in A and A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 1
            //      C: 0
            cpu->a = cpu->a & cpu->a;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=1;
            cpu->flags.c=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xa8:
            //  XOR  A,B : Bitwise XOR between the value in B and A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 0
            //      C: 0
            cpu->a = cpu->a ^ cpu->b;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=0;
            cpu->flags.c=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xa9:
            //  XOR  A,C : Bitwise XOR between the value in C and A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 0
            //      C: 0
            cpu->a = cpu->a ^ cpu->c;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=0;
            cpu->flags.c=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xaa:
            //  XOR  A,D : Bitwise XOR between the value in D and A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 0
            //      C: 0
            cpu->a = cpu->a ^ cpu->d;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=0;
            cpu->flags.c=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xab:
            //  XOR  A,E : Bitwise XOR between the value in E and A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 0
            //      C: 0
            cpu->a = cpu->a ^ cpu->e;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=0;
            cpu->flags.c=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xac:
            //  XOR  A,H : Bitwise XOR between the value in H and A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 0
            //      C: 0
            cpu->a = cpu->a ^ cpu->h;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=0;
            cpu->flags.c=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xad:
            //  XOR  A,L : Bitwise XOR between the value in L and A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 0
            //      C: 0
            cpu->a = cpu->a ^ cpu->l;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=0;
            cpu->flags.c=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xae:
        {
            //  XOR  A,(HL) : Bitwise XOR between the byte pointed to by HL and A.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 0
            //      C: 0
            unsigned short address=(cpu->h<<8) | cpu->l;
            unsigned char value = memory_read(cpu, address);
            cpu->a = cpu->a ^ value;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=0;
            cpu->flags.c=0;
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0xaf:
            //  XOR  A,A : Bitwise XOR between the value in A and A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 0
            //      C: 0
            cpu->a = cpu->a ^ cpu->a;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=0;
            cpu->flags.c=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xb0:
            //  OR  A,B : Bitwise OR between the value in B and A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 0
            //      C: 0
            cpu->a = cpu->a | cpu->b;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=0;
            cpu->flags.c=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xb1:
            //  OR  A,C : Bitwise OR between the value in C and A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 0
            //      C: 0
            cpu->a = cpu->a | cpu->c;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=0;
            cpu->flags.c=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xb2:
            //  OR  A,D : Bitwise OR between the value in D and A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 0
            //      C: 0
            cpu->a = cpu->a | cpu->d;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=0;
            cpu->flags.c=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xb3:
            //  OR  A,E : Bitwise OR between the value in E and A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 0
            //      C: 0
            cpu->a = cpu->a | cpu->e;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=0;
            cpu->flags.c=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xb4:
            //  OR  A,H : Bitwise OR between the value in H and A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 0
            //      C: 0
            cpu->a = cpu->a | cpu->h;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=0;
            cpu->flags.c=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xb5:
            //  OR  A,L : Bitwise OR between the value in L and A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 0
            //      C: 0
            cpu->a = cpu->a | cpu->l;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=0;
            cpu->flags.c=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xb6:
        {
            //  0R  A,(HL) : Bitwise OR between the byte pointed to by HL and A.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 0
            //      C: 0
            unsigned short address=(cpu->h<<8) | cpu->l;
            unsigned char value = memory_read(cpu, address);
            cpu->a = cpu->a | value;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=0;
            cpu->flags.c=0;
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0xb7:
            //  OR  A,A : Bitwise OR between the value in A and A.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 0
            //      C: 0
            cpu->a = cpu->a | cpu->a;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=0;
            cpu->flags.c=0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xb8:
            //  CP  A,B : Subtract the value in B from A and set flags accordingly, but don't store the result. This is useful for ComParing values.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (i.e. if r8 > A).
            cpu->flags.z = (cpu->a == cpu->b);
            cpu->flags.n = 1;
            cpu->flags.h = (cpu->a & 0xf) < (cpu->b & 0xf);
            cpu->flags.c = cpu->a < cpu->b;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xb9:
            //  CP  A,C : Subtract the value in C from A and set flags accordingly, but don't store the result. This is useful for ComParing values.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (i.e. if r8 > A).
            cpu->flags.z = (cpu->a == cpu->c);
            cpu->flags.n = 1;
            cpu->flags.h = (cpu->a & 0xf) < (cpu->c & 0xf);
            cpu->flags.c = cpu->a < cpu->c;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xba:
            //  CP  A,D : Subtract the value in D from A and set flags accordingly, but don't store the result. This is useful for ComParing values.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (i.e. if r8 > A).
            cpu->flags.z = (cpu->a == cpu->d);
            cpu->flags.n = 1;
            cpu->flags.h = (cpu->a & 0xf) < (cpu->d & 0xf);
            cpu->flags.c = cpu->a < cpu->d;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xbb:
            //  CP  A,E : Subtract the value in E from A and set flags accordingly, but don't store the result. This is useful for ComParing values.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (i.e. if r8 > A).
            cpu->flags.z = (cpu->a == cpu->e);
            cpu->flags.n = 1;
            cpu->flags.h = (cpu->a & 0xf) < (cpu->e & 0xf);
            cpu->flags.c = cpu->a < cpu->e;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xbc:
            //  CP  A,H : Subtract the value in H from A and set flags accordingly, but don't store the result. This is useful for ComParing values.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (i.e. if r8 > A).
            cpu->flags.z = (cpu->a == cpu->h);
            cpu->flags.n = 1;
            cpu->flags.h = (cpu->a & 0xf) < (cpu->h & 0xf);
            cpu->flags.c = cpu->a < cpu->h;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xbd:
            //  CP  A,L : Subtract the value in L from A and set flags accordingly, but don't store the result. This is useful for ComParing values.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (i.e. if r8 > A).
            cpu->flags.z = (cpu->a == cpu->l);
            cpu->flags.n = 1;
            cpu->flags.h = (cpu->a & 0xf) < (cpu->l & 0xf);
            cpu->flags.c = cpu->a < cpu->l;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xbe:
        {
            //  CP  A,(HL) : Subtract the byte pointed to by HL from A and set flags accordingly, but don't store the result.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (i.e. if r8 > A).
            unsigned short address=(cpu->h<<8) | cpu->l;
            unsigned char value = memory_read(cpu, address);
            cpu->flags.z = (cpu->a == value);
            cpu->flags.n = 1;
            cpu->flags.h = (cpu->a & 0xf) < (value & 0xf);
            cpu->flags.c = cpu->a < value;
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0xbf:
            //  CP  A,A : Subtract the value in A from A and set flags accordingly, but don't store the result. This is useful for ComParing values.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (i.e. if r8 > A).
            cpu->flags.z = (cpu->a == cpu->a);
            cpu->flags.n = 1;
            cpu->flags.h = (cpu->a & 0xf) < (cpu->a & 0xf);
            cpu->flags.c = cpu->a < cpu->a;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xc0:
            //  RET  NZ : Return from subroutine if condition NZ is met.
            //  Cycles: 20 taken / 8 untaken
            //  Bytes: 1
            //  Flags: None affected.
            if (cpu->flags.z == 0)
            {
               cpu->pc = memory_read(cpu, cpu->sp) | (memory_read(cpu, cpu->sp + 1)<<8);
               cpu->sp += 2;
               cycles = 20;
            }
            else
            {
            cpu->pc+=1;
            cycles = 8;
            }
            break;

        case 0xc1:
            //  POP  BC : Pop register BC from the stack.
            //  Cycles: 12
            //  Bytes: 1
            //  Flags: None affected.
            cpu->c = memory_read(cpu, cpu->sp);
            cpu->sp++;
            cpu->b = memory_read(cpu, cpu->sp);
            cpu->sp++;
            cpu->pc+=1;
            cycles = 12;
            break;

        case 0xc2:
            //  JP  NZ,u16 : Jump to address u16 if condition NZ is met.
            //  Cycles: 16 taken / 12 untaken
            //  Bytes: 3
            //  Flags: None affected.
            if (cpu->flags.z == 0)
            {
               cpu->pc = (opcode[2] << 8 ) | opcode[1];
               cycles = 16;
            }
            else
            {
            cpu->pc+=3;
            cycles = 12;
            }
            break;

        case 0xc3:
            //  JP  u16 : Jump to address u16; effectively, store u16 into PC.
            //  Cycles: 16
            //  Bytes: 3
            //  Flags: None affected.
            cpu->pc = (opcode[2] << 8 ) | opcode[1];
            cycles = 16;
            break;

        case 0xc4:
            //  CALL  NZ,u16 : Call address u16 if condition NZ is met.
            //  Cycles: 24 taken / 12 untaken
            //  Bytes: 3
            //  Flags: None affected.
            if (cpu->flags.z == 0)
            {
               cpu->pc+=3;
               cpu->sp--;
               memory_write(cpu, cpu->sp, cpu->pc >> 8);
               cpu->sp--;
               memory_write(cpu, cpu->sp, cpu->pc & 0xff);
               cpu->pc = (opcode[2] << 8 ) | opcode[1];
               cycles = 24;
            }
            else
            {
            cpu->pc+=3;
            cycles = 12;
            }
            break;

        case 0xc5:
            //  PUSH  BC : Push register BC into the stack.
            //  Cycles: 16
            //  Bytes: 1
            //  Flags: None affected.
            cpu->sp--;
            memory_write(cpu, cpu->sp, cpu->b);
            cpu->sp--;
            memory_write(cpu, cpu->sp, cpu->c);
            cpu->pc+=1;
            cycles = 16;
            break;

        case 0xc6:
            //  ADD  A,u8 : Add the value u8 to A.
            //  Cycles: 8
            //  Bytes: 2
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            //      C: Set if overflow from bit 7.
            cpu->flags.h = (((cpu->a &0xf) + (opcode[1] &0xf)) & 0x10) == 0x10;
            cpu->flags.c = (cpu->a + opcode[1]) > 0xff;
            cpu->a += opcode[1];
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->pc+=2;
            cycles = 8;
            break;

        case 0xc7:
            //  RST  00h : Call address 00h. This is a shorter and faster equivalent to CALL for suitable values of 00h.
            //  Cycles: 16
            //  Bytes: 1
            //  Flags: None affected.
            cpu->pc+=1;
            cpu->sp--;
            memory_write(cpu, cpu->sp, cpu->pc >> 8);
            cpu->sp--;
            memory_write(cpu, cpu->sp, cpu->pc & 0xff);
            cpu->pc = 0x0000;
            cycles = 16;
            break;

        case 0xc8:
            //  RET  Z : Return from subroutine if condition Z is met.
            //  Cycles: 20 taken / 8 untaken
            //  Bytes: 1
            //  Flags: None affected.
            if (cpu->flags.z == 1)
            {
               cpu->pc = memory_read(cpu, cpu->sp) | (memory_read(cpu, cpu->sp + 1)<<8);
               cpu->sp += 2;
               cycles = 20;
            }
            else
            {
            cpu->pc+=1;
            cycles = 8;
            }
            break;

        case 0xc9:
            //  RET : Return from subroutine.
            //  Cycles: 16
            //  Bytes: 1
            //  Flags: None affected.
            cpu->pc = memory_read(cpu, cpu->sp) | (memory_read(cpu, cpu->sp + 1)<<8);
            cpu->sp += 2;
            cycles = 16;
            break;

        case 0xca:
            //  JP  Z,u16 : Jump to address u16 if condition Z is met.
            //  Cycles: 16 taken / 12 untaken
            //  Bytes: 3
            //  Flags: None affected.
            if (cpu->flags.z == 1)
            {
               cpu->pc = (opcode[2] << 8 ) | opcode[1];
               cycles = 16;
            }
            else
            {
            cpu->pc+=3;
            cycles = 12;
            }
            break;

        case 0xcc:
            //  CALL  Z,u16 : Call address u16 if condition Z is met.
            //  Cycles: 24 taken / 12 untaken
            //  Bytes: 3
            //  Flags: None affected.
            if (cpu->flags.z == 1)
            {
               cpu->pc+=3;
               cpu->sp--;
               memory_write(cpu, cpu->sp, cpu->pc >> 8);
               cpu->sp--;
               memory_write(cpu, cpu->sp, cpu->pc & 0xff);
               cpu->pc = (opcode[2] << 8 ) | opcode[1];
               cycles = 24;
            }
            else
            {
            cpu->pc+=3;
            cycles = 12;
            }
            break;

        case 0xcd:
            //  CALL  u16 : Call address u16. This pushes the address of the instruction after the CALL on the stack,
            //  such that RET can pop it later; then, it executes an implicit JP u16.
            //  Cycles: 24
            //  Bytes: 3
            //  Flags: None affected.
            cpu->pc+=3;
            cpu->sp--;
            memory_write(cpu, cpu->sp, cpu->pc >> 8);
            cpu->sp--;
            memory_write(cpu, cpu->sp, cpu->pc & 0xff);
            cpu->pc = (opcode[2] << 8 ) | opcode[1];
            cycles = 24;
            break;

        case 0xce:
        {
            //  ADC  A,u8 : Add the value u8 plus the carry flag to A.
            //  Cycles: 8
            //  Bytes: 2
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: Set if overflow from bit 3.
            //      C: Set if overflow from bit 7.
            unsigned char result = cpu->a + opcode[1] + cpu->flags.c;
            cpu->flags.h = (((cpu->a &0xf) + (opcode[1] &0xf) + cpu->flags.c) & 0x10) == 0x10;
            cpu->flags.c = (cpu->a + opcode[1] + cpu->flags.c) > 0xff;
            cpu->a = result;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->pc+=2;
            cycles = 8;
            break;
        }

        case 0xcf:
            //  RST  08h : Call address 08h. This is a shorter and faster equivalent to CALL for suitable values of 08h.
            //  Cycles: 16
            //  Bytes: 1
            //  Flags: None affected.
            cpu->pc+=1;
            cpu->sp--;
            memory_write(cpu, cpu->sp, cpu->pc >> 8);
            cpu->sp--;
            memory_write(cpu, cpu->sp, cpu->pc & 0xff);
            cpu->pc = 0x0008;
            cycles = 16;
            break;

        case 0xd0:
            //  RET  NC : Return from subroutine if condition NC is met.
            //  Cycles: 20 taken / 8 untaken
            //  Bytes: 1
            //  Flags: None affected.
            if (cpu->flags.c == 0)
            {
               cpu->pc = memory_read(cpu, cpu->sp) | (memory_read(cpu, cpu->sp + 1)<<8);
               cpu->sp += 2;
               cycles = 20;
            }
            else
            {
            cpu->pc+=1;
            cycles = 8;
            }
            break;

        case 0xd1:
            //  POP  DE : Pop register DE from the stack.
            //  Cycles: 12
            //  Bytes: 1
            //  Flags: None affected.
            cpu->e = memory_read(cpu, cpu->sp);
            cpu->sp++;
            cpu->d = memory_read(cpu, cpu->sp);
            cpu->sp++;
            cpu->pc+=1;
            cycles = 12;
            break;

        case 0xd2:
            //  JP  NC,u16 : Jump to address u16 if condition NC is met.
            //  Cycles: 16 taken / 12 untaken
            //  Bytes: 3
            //  Flags: None affected.
            if (cpu->flags.c == 0)
            {
               cpu->pc = (opcode[2] << 8 ) | opcode[1];
               cycles = 16;
            }
            else
            {
            cpu->pc+=3;
            cycles = 12;
            }
            break;

        case 0xd4:
            //  CALL  NC,u16 : Call address u16 if condition NC is met.
            //  Cycles: 24 taken / 12 untaken
            //  Bytes: 3
            //  Flags: None affected.
            if (cpu->flags.c == 0)
            {
               cpu->pc+=3;
               cpu->sp--;
               memory_write(cpu, cpu->sp, cpu->pc >> 8);
               cpu->sp--;
               memory_write(cpu, cpu->sp, cpu->pc & 0xff);
               cpu->pc = (opcode[2] << 8 ) | opcode[1];
               cycles = 24;
            }
            else
            {
            cpu->pc+=3;
            cycles = 12;
            }
            break;

        case 0xd5:
            //  PUSH  DE : Push register DE into the stack.
            //  Cycles: 16
            //  Bytes: 1
            //  Flags: None affected.
            cpu->sp--;
            memory_write(cpu, cpu->sp, cpu->d);
            cpu->sp--;
            memory_write(cpu, cpu->sp, cpu->e);
            cpu->pc+=1;
            cycles = 16;
            break;

        case 0xd6:
            //  SUB  A,u8 : Subtract the value u8 from A.
            //  Cycles: 8
            //  Bytes: 2
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (set if A > A).
            cpu->flags.h = (cpu->a &0xf) < (opcode[1] &0xf);
            cpu->flags.c = cpu->a < opcode[1];
            cpu->a -= opcode[1];
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=1;
            cpu->pc+=2;
            cycles = 8;
            break;

        case 0xd7:
            //  RST  10h : Call address 10h. This is a shorter and faster equivalent to CALL for suitable values of 10h.
            //  Cycles: 16
            //  Bytes: 1
            //  Flags: None affected.
            cpu->pc+=1;
            cpu->sp--;
            memory_write(cpu, cpu->sp, cpu->pc >> 8);
            cpu->sp--;
            memory_write(cpu, cpu->sp, cpu->pc & 0xff);
            cpu->pc = 0x0010;
            cycles = 16;
            break;

        case 0xd8:
            //  RET  C : Return from subroutine if condition C is met.
            //  Cycles: 20 taken / 8 untaken
            //  Bytes: 1
            //  Flags: None affected.
            if (cpu->flags.c == 1)
            {
               cpu->pc = memory_read(cpu, cpu->sp) | (memory_read(cpu, cpu->sp + 1)<<8);
               cpu->sp += 2;
               cycles = 20;
            }
            else
            {
            cpu->pc+=1;
            cycles = 8;
            }
            break;

        case 0xd9:
            //  RETI : Return from subroutine and enable interrupts.
            //  This is basically equivalent to executing EI then RET,
            //  meaning that IME is set right after this instruction.
            //  Cycles: 16
            //  Bytes: 1
            //  Flags: None affected.
            cpu->pc = memory_read(cpu, cpu->sp) | (memory_read(cpu, cpu->sp + 1)<<8);
            cpu->sp += 2;
            cpu->flags.interrupts_enabled = 1;
            cycles = 16;
            break;

        case 0xda:
            //  JP  C,u16 : Jump to address u16 if condition C is met.
            //  Cycles: 16 taken / 12 untaken
            //  Bytes: 3
            //  Flags: None affected.
            if (cpu->flags.c == 1)
            {
               cpu->pc = (opcode[2] << 8 ) | opcode[1];
               cycles = 16;
            }
            else
            {
            cpu->pc+=3;
            cycles = 12;
            }
            break;

        case 0xdc:
            //  CALL  C,u16 : Call address u16 if condition C is met.
            //  Cycles: 24 taken / 12 untaken
            //  Bytes: 3
            //  Flags: None affected.
            if (cpu->flags.c == 1)
            {
               cpu->pc+=3;
               cpu->sp--;
               memory_write(cpu, cpu->sp, cpu->pc >> 8);
               cpu->sp--;
               memory_write(cpu, cpu->sp, cpu->pc & 0xff);
               cpu->pc = (opcode[2] << 8 ) | opcode[1];
               cycles = 24;
            }
            else
            {
            cpu->pc+=3;
            cycles = 12;
            }
            break;

        case 0xde:
        {
            //  SBC  A,u8 : Subtract the value u8 and the carry flag from A.
            //  Cycles: 8
            //  Bytes: 2
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (set if H > A).
            unsigned char result = cpu->a - (opcode[1] + cpu->flags.c);
            cpu->flags.h = ((opcode[1] &0xf) + cpu->flags.c) > (cpu->a &0xf);
            cpu->flags.c = (opcode[1] + cpu->flags.c) > cpu->a;
            cpu->a = result;
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=1;
            cpu->pc+=2;
            cycles = 8;
            break;
        }

        case 0xdf:
            //  RST  18h : Call address 18h. This is a shorter and faster equivalent to CALL for suitable values of 18h.
            //  Cycles: 16
            //  Bytes: 1
            //  Flags: None affected.
            cpu->pc+=1;
            cpu->sp--;
            memory_write(cpu, cpu->sp, cpu->pc >> 8);
            cpu->sp--;
            memory_write(cpu, cpu->sp, cpu->pc & 0xff);
            cpu->pc = 0x0018;
            cycles = 16;
            break;

        case 0xe0:
        {
            //  LD  (FF00+u8),A : write to io-port u8 (memory FF00+u8)
            //  provided the address is between $FF00 and $FFFF.
            //  Cycles: 12
            //  Bytes: 2
            //  Flags: None affected.
            unsigned short address = 0xff00 + opcode[1];
            memory_write(cpu, address, cpu->a);
            cpu->pc+=2;
            cycles = 12;
            break;
        }

        case 0xe1:
            //  POP  HL : Pop register HL from the stack.
            //  Cycles: 12
            //  Bytes: 1
            //  Flags: None affected.
            cpu->l = memory_read(cpu, cpu->sp);
            cpu->sp++;
            cpu->h = memory_read(cpu, cpu->sp);
            cpu->sp++;
            cpu->pc+=1;
            cycles = 12;
            break;

        case 0xe2:
        {
            //  LD  (FF00+C),A : write to io-port C (memory FF00+C)
            //  provided the address is between $FF00 and $FFFF.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            unsigned short address = 0xff00 + cpu->c;
            memory_write(cpu, address, cpu->a);
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0xe5:
            //  PUSH  HL : Push register HL into the stack.
            //  Cycles: 16
            //  Bytes: 1
            //  Flags: None affected.
            cpu->sp--;
            memory_write(cpu, cpu->sp, cpu->h);
            cpu->sp--;
            memory_write(cpu, cpu->sp, cpu->l);
            cpu->pc+=1;
            cycles = 16;
            break;

        case 0xe6:
            //  AND  A,u8 : Bitwise AND between the value u8 and A.
            //  Cycles: 8
            //  Bytes: 2
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 1
            //      C: 0
            cpu->a = cpu->a & opcode[1];
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=1;
            cpu->flags.c=0;
            cpu->pc+=2;
            cycles = 8;
            break;

        case 0xe7:
            //  RST  20h : Call address 20h. This is a shorter and faster equivalent to CALL for suitable values of 20h.
            //  Cycles: 16
            //  Bytes: 1
            //  Flags: None affected.
            cpu->pc+=1;
            cpu->sp--;
            memory_write(cpu, cpu->sp, cpu->pc >> 8);
            cpu->sp--;
            memory_write(cpu, cpu->sp, cpu->pc & 0xff);
            cpu->pc = 0x0020;
            cycles = 16;
            break;

        case 0xe8:
            //  ADD  SP,i8 : Add the signed value i8 to SP.
            //  Cycles: 16
            //  Bytes: 2
            //  Flags:
            //      Z: 0
            //      N: 0
            //      H: Set if overflow from bit 3.
            //      C: Set if overflow from bit 7.
            cpu->flags.h = (((cpu->sp &0xf) + (opcode[1] &0xf)) & 0x10) == 0x10;
            cpu->flags.c = ((cpu->sp & 0xff) + opcode[1]) > 0xff;
            cpu->sp += (char)opcode[1];
            cpu->flags.z=0;
            cpu->flags.n=0;
            cpu->pc+=2;
            cycles = 16;
            break;

        case 0xe9:
            //  JP  HL : Jump to address in HL;
            //  effectively, load PC with value in register HL.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->pc = (cpu->h << 8 ) | cpu->l;
            cycles = 4;
            break;

        case 0xea:
        {
            //  LD  (u16),A : Store value in register A into the byte at address u16.
            //  Cycles: 16
            //  Bytes: 3
            //  Flags: None affected.
            unsigned short address = (opcode[2] << 8) | opcode[1];
            memory_write(cpu, address, cpu->a);
            cpu->pc+=3;
            cycles = 16;
            break;
        }

        case 0xee:
            //  XOR  A,u8 : Bitwise XOR between the value u8 and A.
            //  Cycles: 8
            //  Bytes: 2
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 0
            //      C: 0
            cpu->a = cpu->a ^ opcode[1];
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=0;
            cpu->flags.c=0;
            cpu->pc+=2;
            cycles = 8;
            break;

        case 0xef:
            //  RST  28h : Call address 28h. This is a shorter and faster equivalent to CALL for suitable values of 28h.
            //  Cycles: 16
            //  Bytes: 1
            //  Flags: None affected.
            cpu->pc+=1;
            cpu->sp--;
            memory_write(cpu, cpu->sp, cpu->pc >> 8);
            cpu->sp--;
            memory_write(cpu, cpu->sp, cpu->pc & 0xff);
            cpu->pc = 0x0028;
            cycles = 16;
            break;

        case 0xf0:
        {
            //  LD  A,(FF00+u8) : read from io-port u8 (memory FF00+u8)
            //  provided the address is between $FF00 and $FFFF.
            //  Cycles: 12
            //  Bytes: 2
            //  Flags: None affected.
            unsigned short address = 0xff00 + opcode[1];
            cpu->a = memory_read(cpu, address);
            cpu->pc+=2;
            cycles = 12;
            break;
        }

        case 0xf1:
        {
            //  POP  AF : Pop register AF from the stack.
            //  Cycles: 12
            //  Bytes: 1
            //  //  Flags:
            //      Z: Set from bit 7 of the popped low byte.
            //      N: Set from bit 6 of the popped low byte.
            //      H: Set from bit 5 of the popped low byte.
            //      C: Set from bit 4 of the popped low byte.
            unsigned char flags = memory_read(cpu, cpu->sp);
            cpu->flags.z = (flags >> 7) & 1;
            cpu->flags.n = (flags >> 6) & 1;
            cpu->flags.h = (flags >> 5) & 1;
            cpu->flags.c = (flags >> 4) & 1;
            cpu->sp++;
            cpu->a = memory_read(cpu, cpu->sp);
            cpu->sp++;
            cpu->pc+=1;
            cycles = 12;
            break;
        }

        case 0xf2:
        {
            //  LD  A,(FF00+C) : read from io-port C (memory FF00+C)
            //  provided the address is between $FF00 and $FFFF.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            unsigned short address = 0xff00 + cpu->c;
            cpu->a = memory_read(cpu, address);
            cpu->pc+=1;
            cycles = 8;
            break;
        }

        case 0xf3:
            //  DI : Disable Interrupts by clearing the IME flag.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->flags.interrupts_enabled = 0;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xf5:
        {
            //  PUSH  AF : Push register AF into the stack.
            //  Cycles: 16
            //  Bytes: 1
            //  Flags: None affected.
            unsigned char flags = (cpu->flags.z << 7) | (cpu->flags.n << 6) | (cpu->flags.h << 5) | (cpu->flags.c << 4);
            cpu->sp--;
            memory_write(cpu, cpu->sp, cpu->a);
            cpu->sp--;
            memory_write(cpu, cpu->sp, flags);
            cpu->pc+=1;
            cycles = 16;
            break;
        }

        case 0xf6:
            //  OR  A,u8 : Bitwise OR between the value u8 and A.
            //  Cycles: 8
            //  Bytes: 2
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 0
            //      H: 0
            //      C: 0
            cpu->a = cpu->a | opcode[1];
            cpu->flags.z=(cpu->a == 0);
            cpu->flags.n=0;
            cpu->flags.h=0;
            cpu->flags.c=0;
            cpu->pc+=2;
            cycles = 8;
            break;

        case 0xf7:
            //  RST  30h : Call address 30h. This is a shorter and faster equivalent to CALL for suitable values of 30h.
            //  Cycles: 16
            //  Bytes: 1
            //  Flags: None affected.
            cpu->pc+=1;
            cpu->sp--;
            memory_write(cpu, cpu->sp, cpu->pc >> 8);
            cpu->sp--;
            memory_write(cpu, cpu->sp, cpu->pc & 0xff);
            cpu->pc = 0x0030;
            cycles = 16;
            break;

        case 0xf8:
        {
            //  LD  HL,SP+i8 : Add the signed value i8 to SP and store the result in HL.
            //  Cycles: 12
            //  Bytes: 2
            //  Flags:
            //      Z: 0
            //      N: 0
            //      H: Set if overflow from bit 3.
            //      C: Set if overflow from bit 7.
            unsigned short result = cpu->sp + (char)opcode[1];
            cpu->h = result >> 8;
            cpu->l = result & 0xff;
            cpu->flags.h = (((cpu->sp & 0xf) + (opcode[1] & 0xf)) & 0x10) == 0x10;
            cpu->flags.c = ((cpu->sp & 0xff) + opcode[1]) > 0xff;
            cpu->flags.z=0;
            cpu->flags.n=0;
            cpu->pc+=2;
            cycles = 12;
            break;
        }

        case 0xf9:
            //  LD  SP,HL : Load register HL into register SP.
            //  Cycles: 8
            //  Bytes: 1
            //  Flags: None affected.
            cpu->sp = (cpu->h<<8) | cpu->l;
            cpu->pc+=1;
            cycles = 8;
            break;

        case 0xfa:
        {
            //  LD  A,(u16) : Load value in register A from the byte at address u16.
            //  Cycles: 16
            //  Bytes: 3
            //  Flags: None affected.
            unsigned short address = (opcode[2] << 8) | opcode[1];
            cpu->a = memory_read(cpu, address);
            cpu->pc+=3;
            cycles = 16;
            break;
        }

        case 0xfb:
            //  IE : Enable Interrupts by setting the IME flag.
            //  The flag is only set after the instruction following EI.
            //  Cycles: 4
            //  Bytes: 1
            //  Flags: None affected.
            cpu->flags.interrupts_enabled = 1;
            cpu->pc+=1;
            cycles = 4;
            break;

        case 0xfe:
            //  CP  A,u8 : Subtract the value u8 from A and set flags accordingly, but don't store the result. This is useful for ComParing values.
            //  Cycles: 8
            //  Bytes: 2
            //  Flags:
            //      Z: Set if result is 0.
            //      N: 1
            //      H: Set if borrow from bit 4.
            //      C: Set if borrow (i.e. if r8 > A).
            cpu->flags.z = (cpu->a == opcode[1]);
            cpu->flags.n = 1;
            cpu->flags.h = (cpu->a & 0xf) < (opcode[1] & 0xf);
            cpu->flags.c = cpu->a < opcode[1];
            cpu->pc+=2;
            cycles = 8;
            break;

        case 0xff:
            //  RST  38h : Call address 38h. This is a shorter and faster equivalent to CALL for suitable values of 38h.
            //  Cycles: 16
            //  Bytes: 1
            //  Flags: None affected.
            cpu->pc+=1;
            cpu->sp--;
            memory_write(cpu, cpu->sp, cpu->pc >> 8);
            cpu->sp--;
            memory_write(cpu, cpu->sp, cpu->pc & 0xff);
            cpu->pc = 0x0038;
            cycles = 16;
            break;

        case 0xcb:
            //  CB-Prefixed OpCodes
            switch (opcode[1])
            {
                //  RLC
                case 0x00: cycles = rlc(cpu, &cpu->b); break;
                case 0x01: cycles = rlc(cpu, &cpu->c); break;
                case 0x02: cycles = rlc(cpu, &cpu->d); break;
                case 0x03: cycles = rlc(cpu, &cpu->e); break;
                case 0x04: cycles = rlc(cpu, &cpu->h); break;
                case 0x05: cycles = rlc(cpu, &cpu->l); break;
                case 0x06:
                {
                    //  RLC (HL) : Rotate the byte pointed to by HL left.
                    //  C <- [7 <- 0] <- [7]
                    //  Cycles: 16
                    //  Bytes: 2
                    //  Flags:
                    //      Z: Set if result is 0.
                    //      N: 0
                    //      H: 0
                    //      C: Set according to result.
                    unsigned short address = (cpu->h<<8) | cpu->l;
                    unsigned char value = memory_read(cpu, address);
                    unsigned char temp = value;
                    value = (temp << 1) | (temp >> 7);
                    memory_write(cpu, address, value);
                    cpu->flags.z = (value == 0);
                    cpu->flags.n = 0;
                    cpu->flags.h = 0;
                    cpu->flags.c = temp >> 7;
                    cpu->pc+=2;
                    cycles = 16;
                    break;
                }
                case 0x07: cycles = rlc(cpu, &cpu->a); break;


                //  RRC
                case 0x08: cycles = rrc(cpu, &cpu->b); break;
                case 0x09: cycles = rrc(cpu, &cpu->c); break;
                case 0x0a: cycles = rrc(cpu, &cpu->d); break;
                case 0x0b: cycles = rrc(cpu, &cpu->e); break;
                case 0x0c: cycles = rrc(cpu, &cpu->h); break;
                case 0x0d: cycles = rrc(cpu, &cpu->l); break;
                case 0x0e:
                {
                    //  RRC (HL) : Rotate the byte pointed to by HL right.
                    //  [0] -> [7 -> 0] -> C
                    //  Cycles: 16
                    //  Bytes: 2
                    //  Flags:
                    //      Z: Set if result is 0.
                    //      N: 0
                    //      H: 0
                    //      C: Set according to result.
                    unsigned short address = (cpu->h<<8) | cpu->l;
                    unsigned char value = memory_read(cpu, address);
                    unsigned char temp = value;
                    value = (temp >> 1) | (temp << 7);
                    memory_write(cpu, address, value);
                    cpu->flags.z = (value == 0);;
                    cpu->flags.n = 0;
                    cpu->flags.h = 0;
                    cpu->flags.c = temp & 1;
                    cpu->pc+=2;
                    cycles = 16;
                    break;
                }
                case 0x0f: cycles = rrc(cpu, &cpu->a); break;


                //  RL
                case 0x10: cycles = rl(cpu, &cpu->b); break;
                case 0x11: cycles = rl(cpu, &cpu->c); break;
                case 0x12: cycles = rl(cpu, &cpu->d); break;
                case 0x13: cycles = rl(cpu, &cpu->e); break;
                case 0x14: cycles = rl(cpu, &cpu->h); break;
                case 0x15: cycles = rl(cpu, &cpu->l); break;
                case 0x16:
                {
                    //  RL (HL) : Rotate the byte pointed to by HL left through carry.
                    //  C <- [7 <- 0] <- C
                    //  Cycles: 16
                    //  Bytes: 2
                    //  Flags:
                    //      Z: Set if result is 0.
                    //      N: 0
                    //      H: 0
                    //      C: Set according to result.
                    unsigned short address = (cpu->h<<8) | cpu->l;
                    unsigned char value = memory_read(cpu, address);
                    unsigned char temp = value;
                    value = (temp << 1) | (cpu->flags.c);
                    memory_write(cpu, address, value);
                    cpu->flags.z = (value == 0);
                    cpu->flags.n = 0;
                    cpu->flags.h = 0;
                    cpu->flags.c = temp >> 7;
                    cpu->pc+=2;
                    cycles = 16;
                    break;
                }
                case 0x17: cycles = rl(cpu, &cpu->a); break;


                //  RR
                case 0x18: cycles = rr(cpu, &cpu->b); break;
                case 0x19: cycles = rr(cpu, &cpu->c); break;
                case 0x1a: cycles = rr(cpu, &cpu->d); break;
                case 0x1b: cycles = rr(cpu, &cpu->e); break;
                case 0x1c: cycles = rr(cpu, &cpu->h); break;
                case 0x1d: cycles = rr(cpu, &cpu->l); break;
                case 0x1e:
                {
                    //  RR (HL) : Rotate the byte pointed to by HL right through carry.
                    //  C -> [7 -> 0] -> C
                    //  Cycles: 16
                    //  Bytes: 2
                    //  Flags:
                    //      Z: Set if result is 0.
                    //      N: 0
                    //      H: 0
                    //      C: Set according to result.
                    unsigned short address = (cpu->h<<8) | cpu->l;
                    unsigned char value = memory_read(cpu, address);
                    unsigned char temp = value;
                    value = (temp >> 1) | (cpu->flags.c << 7);
                    memory_write(cpu, address, value);
                    cpu->flags.z = (value == 0);;
                    cpu->flags.n = 0;
                    cpu->flags.h = 0;
                    cpu->flags.c = temp & 1;
                    cpu->pc+=2;
                    cycles = 16;
                    break;
                }
                case 0x1f: cycles = rr(cpu, &cpu->a); break;


                //  SLA
                case 0x20: cycles = sla(cpu, &cpu->b); break;
                case 0x21: cycles = sla(cpu, &cpu->c); break;
                case 0x22: cycles = sla(cpu, &cpu->d); break;
                case 0x23: cycles = sla(cpu, &cpu->e); break;
                case 0x24: cycles = sla(cpu, &cpu->h); break;
                case 0x25: cycles = sla(cpu, &cpu->l); break;
                case 0x26:
                {
                    //  SLA (HL) : Shift Left Arithmetically the byte pointed to by HL.
                    //  C <- [7 <- 0] <- 0
                    //  Cycles: 16
                    //  Bytes: 2
                    //  Flags:
                    //      Z: Set if result is 0.
                    //      N: 0
                    //      H: 0
                    //      C: Set according to result.
                    unsigned short address = (cpu->h<<8) | cpu->l;
                    unsigned char value = memory_read(cpu, address);
                    unsigned char temp = value;
                    value = (temp << 1);
                    memory_write(cpu, address, value);
                    cpu->flags.z = (value == 0);
                    cpu->flags.n = 0;
                    cpu->flags.h = 0;
                    cpu->flags.c = temp >> 7;
                    cpu->pc+=2;
                    cycles = 16;
                    break;
                }
                case 0x27: cycles = sla(cpu, &cpu->a); break;


                //  SRA
                case 0x28: cycles = sra(cpu, &cpu->b); break;
                case 0x29: cycles = sra(cpu, &cpu->c); break;
                case 0x2a: cycles = sra(cpu, &cpu->d); break;
                case 0x2b: cycles = sra(cpu, &cpu->e); break;
                case 0x2c: cycles = sra(cpu, &cpu->h); break;
                case 0x2d: cycles = sra(cpu, &cpu->l); break;
                case 0x2e:
                {
                    //  SRA (HL) : Shift Right Arithmetically the byte pointed to by HL.
                    //  [7] -> [7 -> 0] -> C
                    //  Cycles: 16
                    //  Bytes: 2
                    //  Flags:
                    //      Z: Set if result is 0.
                    //      N: 0
                    //      H: 0
                    //      C: Set according to result.
                    unsigned short address = (cpu->h<<8) | cpu->l;
                    unsigned char value = memory_read(cpu, address);
                    unsigned char temp = value;
                    value = (temp >> 1) | (temp & 0x80);
                    memory_write(cpu, address, value);
                    cpu->flags.z = (value == 0);;
                    cpu->flags.n = 0;
                    cpu->flags.h = 0;
                    cpu->flags.c = temp & 1;
                    cpu->pc+=2;
                    cycles = 16;
                    break;
                }
                case 0x2f: cycles = sra(cpu, &cpu->a); break;


                //  SWAP
                case 0x30: cycles = swap_nibble(cpu, &cpu->b); break;
                case 0x31: cycles = swap_nibble(cpu, &cpu->c); break;
                case 0x32: cycles = swap_nibble(cpu, &cpu->d); break;
                case 0x33: cycles = swap_nibble(cpu, &cpu->e); break;
                case 0x34: cycles = swap_nibble(cpu, &cpu->h); break;
                case 0x35: cycles = swap_nibble(cpu, &cpu->l); break;
                case 0x36:
                {
                    //  SWAP (HL) : Swap the upper 4 bits in the byte pointed by HL and the lower 4 ones.
                    //  Cycles: 16
                    //  Bytes: 2
                    //  Flags:
                    //      Z: Set if result is 0.
                    //      N: 0
                    //      H: 0
                    //      C: 0
                    unsigned short address = (cpu->h<<8) | cpu->l;
                    unsigned char value = memory_read(cpu, address);
                    value = (value << 4 ) | (value >> 4);
                    memory_write(cpu, address, value);
                    cpu->flags.z = (value == 0);
                    cpu->flags.n = 0;
                    cpu->flags.h = 0;
                    cpu->flags.c = 0;
                    cpu->pc+=2;
                    cycles = 16;
                    break;
                }
                case 0x37: cycles = swap_nibble(cpu, &cpu->a); break;


                //  SRL
                case 0x38: cycles = srl(cpu, &cpu->b); break;
                case 0x39: cycles = srl(cpu, &cpu->c); break;
                case 0x3a: cycles = srl(cpu, &cpu->d); break;
                case 0x3b: cycles = srl(cpu, &cpu->e); break;
                case 0x3c: cycles = srl(cpu, &cpu->h); break;
                case 0x3d: cycles = srl(cpu, &cpu->l); break;
                case 0x3e:
                {
                    //  SRL (HL) : Shift Right Logically the byte pointed to by HL.
                    //  0 -> [7 -> 0] -> C
                    //  Cycles: 16
                    //  Bytes: 2
                    //  Flags:
                    //      Z: Set if result is 0.
                    //      N: 0
                    //      H: 0
                    //      C: Set according to result.
                    unsigned short address = (cpu->h<<8) | cpu->l;
                    unsigned char value = memory_read(cpu, address);
                    unsigned char temp = value;
                    value = temp >> 1;
                    memory_write(cpu, address, value);
                    cpu->flags.z = (value == 0);;
                    cpu->flags.n = 0;
                    cpu->flags.h = 0;
                    cpu->flags.c = temp & 1;
                    cpu->pc+=2;
                    cycles = 16;
                    break;
                }
                case 0x3f: cycles = srl(cpu, &cpu->a); break;


                //  BIT 0
                case 0x40: cycles = bit(cpu, &cpu->b, 0); break;
                case 0x41: cycles = bit(cpu, &cpu->c, 0); break;
                case 0x42: cycles = bit(cpu, &cpu->d, 0); break;
                case 0x43: cycles = bit(cpu, &cpu->e, 0); break;
                case 0x44: cycles = bit(cpu, &cpu->h, 0); break;
                case 0x45: cycles = bit(cpu, &cpu->l, 0); break;
                case 0x46: cycles = bit_hl(cpu, 0); break;
                case 0x47: cycles = bit(cpu, &cpu->a, 0); break;
                //  BIT 1
                case 0x48: cycles = bit(cpu, &cpu->b, 1); break;
                case 0x49: cycles = bit(cpu, &cpu->c, 1); break;
                case 0x4a: cycles = bit(cpu, &cpu->d, 1); break;
                case 0x4b: cycles = bit(cpu, &cpu->e, 1); break;
                case 0x4c: cycles = bit(cpu, &cpu->h, 1); break;
                case 0x4d: cycles = bit(cpu, &cpu->l, 1); break;
                case 0x4e: cycles = bit_hl(cpu, 1); break;
                case 0x4f: cycles = bit(cpu, &cpu->a, 1); break;
                //  BIT 2
                case 0x50: cycles = bit(cpu, &cpu->b, 2); break;
                case 0x51: cycles = bit(cpu, &cpu->c, 2); break;
                case 0x52: cycles = bit(cpu, &cpu->d, 2); break;
                case 0x53: cycles = bit(cpu, &cpu->e, 2); break;
                case 0x54: cycles = bit(cpu, &cpu->h, 2); break;
                case 0x55: cycles = bit(cpu, &cpu->l, 2); break;
                case 0x56: cycles = bit_hl(cpu, 2); break;
                case 0x57: cycles = bit(cpu, &cpu->a, 2); break;
                //  BIT 3
                case 0x58: cycles = bit(cpu, &cpu->b, 3); break;
                case 0x59: cycles = bit(cpu, &cpu->c, 3); break;
                case 0x5a: cycles = bit(cpu, &cpu->d, 3); break;
                case 0x5b: cycles = bit(cpu, &cpu->e, 3); break;
                case 0x5c: cycles = bit(cpu, &cpu->h, 3); break;
                case 0x5d: cycles = bit(cpu, &cpu->l, 3); break;
                case 0x5e: cycles = bit_hl(cpu, 3); break;
                case 0x5f: cycles = bit(cpu, &cpu->a, 3); break;
                //  BIT 4
                case 0x60: cycles = bit(cpu, &cpu->b, 4); break;
                case 0x61: cycles = bit(cpu, &cpu->c, 4); break;
                case 0x62: cycles = bit(cpu, &cpu->d, 4); break;
                case 0x63: cycles = bit(cpu, &cpu->e, 4); break;
                case 0x64: cycles = bit(cpu, &cpu->h, 4); break;
                case 0x65: cycles = bit(cpu, &cpu->l, 4); break;
                case 0x66: cycles = bit_hl(cpu, 4); break;
                case 0x67: cycles = bit(cpu, &cpu->a, 4); break;
                //  BIT 5
                case 0x68: cycles = bit(cpu, &cpu->b, 5); break;
                case 0x69: cycles = bit(cpu, &cpu->c, 5); break;
                case 0x6a: cycles = bit(cpu, &cpu->d, 5); break;
                case 0x6b: cycles = bit(cpu, &cpu->e, 5); break;
                case 0x6c: cycles = bit(cpu, &cpu->h, 5); break;
                case 0x6d: cycles = bit(cpu, &cpu->l, 5); break;
                case 0x6e: cycles = bit_hl(cpu, 5); break;
                case 0x6f: cycles = bit(cpu, &cpu->a, 5); break;
                //  BIT 6
                case 0x70: cycles = bit(cpu, &cpu->b, 6); break;
                case 0x71: cycles = bit(cpu, &cpu->c, 6); break;
                case 0x72: cycles = bit(cpu, &cpu->d, 6); break;
                case 0x73: cycles = bit(cpu, &cpu->e, 6); break;
                case 0x74: cycles = bit(cpu, &cpu->h, 6); break;
                case 0x75: cycles = bit(cpu, &cpu->l, 6); break;
                case 0x76: cycles = bit_hl(cpu, 6); break;
                case 0x77: cycles = bit(cpu, &cpu->a, 6); break;
                //  BIT 7
                case 0x78: cycles = bit(cpu, &cpu->b, 7); break;
                case 0x79: cycles = bit(cpu, &cpu->c, 7); break;
                case 0x7a: cycles = bit(cpu, &cpu->d, 7); break;
                case 0x7b: cycles = bit(cpu, &cpu->e, 7); break;
                case 0x7c: cycles = bit(cpu, &cpu->h, 7); break;
                case 0x7d: cycles = bit(cpu, &cpu->l, 7); break;
                case 0x7e: cycles = bit_hl(cpu, 7); break;
                case 0x7f: cycles = bit(cpu, &cpu->a, 7); break;


                //  RES 0
                case 0x80: cycles = res_bit(cpu, &cpu->b, 0); break;
                case 0x81: cycles = res_bit(cpu, &cpu->c, 0); break;
                case 0x82: cycles = res_bit(cpu, &cpu->d, 0); break;
                case 0x83: cycles = res_bit(cpu, &cpu->e, 0); break;
                case 0x84: cycles = res_bit(cpu, &cpu->h, 0); break;
                case 0x85: cycles = res_bit(cpu, &cpu->l, 0); break;
                case 0x86: cycles = res_bit_hl(cpu, 0); break;
                case 0x87: cycles = res_bit(cpu, &cpu->a, 0); break;
                //  RES 1
                case 0x88: cycles = res_bit(cpu, &cpu->b, 1); break;
                case 0x89: cycles = res_bit(cpu, &cpu->c, 1); break;
                case 0x8a: cycles = res_bit(cpu, &cpu->d, 1); break;
                case 0x8b: cycles = res_bit(cpu, &cpu->e, 1); break;
                case 0x8c: cycles = res_bit(cpu, &cpu->h, 1); break;
                case 0x8d: cycles = res_bit(cpu, &cpu->l, 1); break;
                case 0x8e: cycles = res_bit_hl(cpu, 1); break;
                case 0x8f: cycles = res_bit(cpu, &cpu->a, 1); break;
                //  RES 2
                case 0x90: cycles = res_bit(cpu, &cpu->b, 2); break;
                case 0x91: cycles = res_bit(cpu, &cpu->c, 2); break;
                case 0x92: cycles = res_bit(cpu, &cpu->d, 2); break;
                case 0x93: cycles = res_bit(cpu, &cpu->e, 2); break;
                case 0x94: cycles = res_bit(cpu, &cpu->h, 2); break;
                case 0x95: cycles = res_bit(cpu, &cpu->l, 2); break;
                case 0x96: cycles = res_bit_hl(cpu, 2); break;
                case 0x97: cycles = res_bit(cpu, &cpu->a, 2); break;
                //  RES 3
                case 0x98: cycles = res_bit(cpu, &cpu->b, 3); break;
                case 0x99: cycles = res_bit(cpu, &cpu->c, 3); break;
                case 0x9a: cycles = res_bit(cpu, &cpu->d, 3); break;
                case 0x9b: cycles = res_bit(cpu, &cpu->e, 3); break;
                case 0x9c: cycles = res_bit(cpu, &cpu->h, 3); break;
                case 0x9d: cycles = res_bit(cpu, &cpu->l, 3); break;
                case 0x9e: cycles = res_bit_hl(cpu, 3); break;
                case 0x9f: cycles = res_bit(cpu, &cpu->a, 3); break;
                //  RES 4
                case 0xa0: cycles = res_bit(cpu, &cpu->b, 4); break;
                case 0xa1: cycles = res_bit(cpu, &cpu->c, 4); break;
                case 0xa2: cycles = res_bit(cpu, &cpu->d, 4); break;
                case 0xa3: cycles = res_bit(cpu, &cpu->e, 4); break;
                case 0xa4: cycles = res_bit(cpu, &cpu->h, 4); break;
                case 0xa5: cycles = res_bit(cpu, &cpu->l, 4); break;
                case 0xa6: cycles = res_bit_hl(cpu, 4); break;
                case 0xa7: cycles = res_bit(cpu, &cpu->a, 4); break;
                //  RES 5
                case 0xa8: cycles = res_bit(cpu, &cpu->b, 5); break;
                case 0xa9: cycles = res_bit(cpu, &cpu->c, 5); break;
                case 0xaa: cycles = res_bit(cpu, &cpu->d, 5); break;
                case 0xab: cycles = res_bit(cpu, &cpu->e, 5); break;
                case 0xac: cycles = res_bit(cpu, &cpu->h, 5); break;
                case 0xad: cycles = res_bit(cpu, &cpu->l, 5); break;
                case 0xae: cycles = res_bit_hl(cpu, 5); break;
                case 0xaf: cycles = res_bit(cpu, &cpu->a, 5); break;
                //  RES 6
                case 0xb0: cycles = res_bit(cpu, &cpu->b, 6); break;
                case 0xb1: cycles = res_bit(cpu, &cpu->c, 6); break;
                case 0xb2: cycles = res_bit(cpu, &cpu->d, 6); break;
                case 0xb3: cycles = res_bit(cpu, &cpu->e, 6); break;
                case 0xb4: cycles = res_bit(cpu, &cpu->h, 6); break;
                case 0xb5: cycles = res_bit(cpu, &cpu->l, 6); break;
                case 0xb6: cycles = res_bit_hl(cpu, 6); break;
                case 0xb7: cycles = res_bit(cpu, &cpu->a, 6); break;
                //  RES 7
                case 0xb8: cycles = res_bit(cpu, &cpu->b, 7); break;
                case 0xb9: cycles = res_bit(cpu, &cpu->c, 7); break;
                case 0xba: cycles = res_bit(cpu, &cpu->d, 7); break;
                case 0xbb: cycles = res_bit(cpu, &cpu->e, 7); break;
                case 0xbc: cycles = res_bit(cpu, &cpu->h, 7); break;
                case 0xbd: cycles = res_bit(cpu, &cpu->l, 7); break;
                case 0xbe: cycles = res_bit_hl(cpu, 7); break;
                case 0xbf: cycles = res_bit(cpu, &cpu->a, 7); break;


                //  SET 0
                case 0xc0: cycles = set_bit(cpu, &cpu->b, 0); break;
                case 0xc1: cycles = set_bit(cpu, &cpu->c, 0); break;
                case 0xc2: cycles = set_bit(cpu, &cpu->d, 0); break;
                case 0xc3: cycles = set_bit(cpu, &cpu->e, 0); break;
                case 0xc4: cycles = set_bit(cpu, &cpu->h, 0); break;
                case 0xc5: cycles = set_bit(cpu, &cpu->l, 0); break;
                case 0xc6: cycles = set_bit_hl(cpu, 0); break;
                case 0xc7: cycles = set_bit(cpu, &cpu->a, 0); break;
                //  SET 1
                case 0xc8: cycles = set_bit(cpu, &cpu->b, 1); break;
                case 0xc9: cycles = set_bit(cpu, &cpu->c, 1); break;
                case 0xca: cycles = set_bit(cpu, &cpu->d, 1); break;
                case 0xcb: cycles = set_bit(cpu, &cpu->e, 1); break;
                case 0xcc: cycles = set_bit(cpu, &cpu->h, 1); break;
                case 0xcd: cycles = set_bit(cpu, &cpu->l, 1); break;
                case 0xce: cycles = set_bit_hl(cpu, 1); break;
                case 0xcf: cycles = set_bit(cpu, &cpu->a, 1); break;
                //  SET 2
                case 0xd0: cycles = set_bit(cpu, &cpu->b, 2); break;
                case 0xd1: cycles = set_bit(cpu, &cpu->c, 2); break;
                case 0xd2: cycles = set_bit(cpu, &cpu->d, 2); break;
                case 0xd3: cycles = set_bit(cpu, &cpu->e, 2); break;
                case 0xd4: cycles = set_bit(cpu, &cpu->h, 2); break;
                case 0xd5: cycles = set_bit(cpu, &cpu->l, 2); break;
                case 0xd6: cycles = set_bit_hl(cpu, 2); break;
                case 0xd7: cycles = set_bit(cpu, &cpu->a, 2); break;
                //  SET 3
                case 0xd8: cycles = set_bit(cpu, &cpu->b, 3); break;
                case 0xd9: cycles = set_bit(cpu, &cpu->c, 3); break;
                case 0xda: cycles = set_bit(cpu, &cpu->d, 3); break;
                case 0xdb: cycles = set_bit(cpu, &cpu->e, 3); break;
                case 0xdc: cycles = set_bit(cpu, &cpu->h, 3); break;
                case 0xdd: cycles = set_bit(cpu, &cpu->l, 3); break;
                case 0xde: cycles = set_bit_hl(cpu, 3); break;
                case 0xdf: cycles = set_bit(cpu, &cpu->a, 3); break;
                //  SET 4
                case 0xe0: cycles = set_bit(cpu, &cpu->b, 4); break;
                case 0xe1: cycles = set_bit(cpu, &cpu->c, 4); break;
                case 0xe2: cycles = set_bit(cpu, &cpu->d, 4); break;
                case 0xe3: cycles = set_bit(cpu, &cpu->e, 4); break;
                case 0xe4: cycles = set_bit(cpu, &cpu->h, 4); break;
                case 0xe5: cycles = set_bit(cpu, &cpu->l, 4); break;
                case 0xe6: cycles = set_bit_hl(cpu, 4); break;
                case 0xe7: cycles = set_bit(cpu, &cpu->a, 4); break;
                //  SET 5
                case 0xe8: cycles = set_bit(cpu, &cpu->b, 5); break;
                case 0xe9: cycles = set_bit(cpu, &cpu->c, 5); break;
                case 0xea: cycles = set_bit(cpu, &cpu->d, 5); break;
                case 0xeb: cycles = set_bit(cpu, &cpu->e, 5); break;
                case 0xec: cycles = set_bit(cpu, &cpu->h, 5); break;
                case 0xed: cycles = set_bit(cpu, &cpu->l, 5); break;
                case 0xee: cycles = set_bit_hl(cpu, 5); break;
                case 0xef: cycles = set_bit(cpu, &cpu->a, 5); break;
                //  SET 6
                case 0xf0: cycles = set_bit(cpu, &cpu->b, 6); break;
                case 0xf1: cycles = set_bit(cpu, &cpu->c, 6); break;
                case 0xf2: cycles = set_bit(cpu, &cpu->d, 6); break;
                case 0xf3: cycles = set_bit(cpu, &cpu->e, 6); break;
                case 0xf4: cycles = set_bit(cpu, &cpu->h, 6); break;
                case 0xf5: cycles = set_bit(cpu, &cpu->l, 6); break;
                case 0xf6: cycles = set_bit_hl(cpu, 6); break;
                case 0xf7: cycles = set_bit(cpu, &cpu->a, 6); break;
                //  SET 7
                case 0xf8: cycles = set_bit(cpu, &cpu->b, 7); break;
                case 0xf9: cycles = set_bit(cpu, &cpu->c, 7); break;
                case 0xfa: cycles = set_bit(cpu, &cpu->d, 7); break;
                case 0xfb: cycles = set_bit(cpu, &cpu->e, 7); break;
                case 0xfc: cycles = set_bit(cpu, &cpu->h, 7); break;
                case 0xfd: cycles = set_bit(cpu, &cpu->l, 7); break;
                case 0xfe: cycles = set_bit_hl(cpu, 7); break;
                case 0xff: cycles = set_bit(cpu, &cpu->a, 7); break;

                default:
                    printf("Unsupported CB-prefixed opcode: 0x%02x at 0x%04x\n\n\n", opcode[1], cpu->pc);
                    exit(0);
                    break;
            }
            break;

        default:
            printf("Unsupported opcode: 0x%02x at 0x%04x\n\n\n", *opcode, cpu->pc);
            exit(0);
            cpu->pc++;
            break;
    }
    return cycles;
}




void print_flags(CPU_FLAGS2 *flag)
{
    printf(" cpu_flag: \n all_flags = %d \n Z = %d \n N = %d \n H = %d  \n C = %d  \n PAD = %d"
    ,flag->all_flags, flag->z, flag->n, flag->h, flag->c, flag->pad);

}

void test_flags(void)
{
    CPU_FLAGS2 flag;
    flag.all_flags=128;
    printf("\n after flag.all_flags = 128 --> 0B10000000 \n\n");
    print_flags(&flag);
    flag.z=1;
    printf("\n\n\n after flag.z = 1 flag.all_flags is supposed to be 128 --> 0B10000000 \n\n");
    print_flags(&flag);
    printf("\n\n on my machine flag.all_flags is 144 --> 0B10010000 \n\n");
    printf(" my machine is low endian \n");
}


int rlc(CPU *cpu, unsigned char *reg)
{
    //  RLC reg : Rotate register reg left.
    //  C <- [7 <- 0] <- [7]
    //  Cycles: 8
    //  Bytes: 2
    //  Flags:
    //      Z: Set if result is 0.
    //      N: 0
    //      H: 0
    //      C: Set according to result.
    unsigned char temp = *reg;
    *reg = (temp << 1) | (temp >> 7);
    cpu->flags.z = (*reg == 0);
    cpu->flags.n = 0;
    cpu->flags.h = 0;
    cpu->flags.c = temp >> 7;
    cpu->pc+=2;
    return 8;
}

int rrc(CPU *cpu, unsigned char *reg)
{
    //  RRC reg : Rotate register reg right.
    //  [0] -> [7 -> 0] -> C
    //  Cycles: 8
    //  Bytes: 2
    //  Flags:
    //      Z: Set if result is 0.
    //      N: 0
    //      H: 0
    //      C: Set according to result.
     unsigned char temp = *reg;
     *reg = (temp >> 1) | (temp << 7);
     cpu->flags.z = (*reg == 0);;
     cpu->flags.n = 0;
     cpu->flags.h = 0;
     cpu->flags.c = temp & 1;
     cpu->pc+=2;
     return 8;
}

int rl(CPU *cpu, unsigned char *reg)
{
    //  RL reg : Rotate bits in register reg left through carry.
    //  C <- [7 <- 0] <- C
    //  Cycles: 8
    //  Bytes: 2
    //  Flags:
    //      Z: Set if result is 0.
    //      N: 0
    //      H: 0
    //      C: Set according to result.
    unsigned char temp = *reg;
    *reg = (temp << 1) | (cpu->flags.c);
    cpu->flags.z = (*reg == 0);
    cpu->flags.n = 0;
    cpu->flags.h = 0;
    cpu->flags.c = temp >> 7;
    cpu->pc+=2;
    return 8;
}

int rr(CPU *cpu, unsigned char *reg)
{
    //  RR reg : Rotate bits in register reg right through carry.
    //  C -> [7 -> 0] -> C
    //  Cycles: 8
    //  Bytes: 2
    //  Flags:
    //      Z: Set if result is 0.
    //      N: 0
    //      H: 0
    //      C: Set according to result.
    unsigned char temp = *reg;
    *reg = (temp >> 1) | (cpu->flags.c << 7);
    cpu->flags.z = (*reg == 0);
    cpu->flags.n = 0;
    cpu->flags.h = 0;
    cpu->flags.c = temp & 1;
    cpu->pc+=2;
    return 8;
}

int sla(CPU *cpu, unsigned char *reg)
{
    //  SLA reg : Shift Left Arithmetically register reg.
    //  C <- [7 <- 0] <- 0
    //  Cycles: 8
    //  Bytes: 2
    //  Flags:
    //      Z: Set if result is 0.
    //      N: 0
    //      H: 0
    //      C: Set according to result.
    unsigned char temp = *reg;
    *reg = (temp << 1);
    cpu->flags.z = (*reg == 0);
    cpu->flags.n = 0;
    cpu->flags.h = 0;
    cpu->flags.c = temp >> 7;
    cpu->pc+=2;
    return 8;
}

int sra(CPU *cpu, unsigned char *reg)
{
    //  SRA reg : Shift Right Arithmetically register reg.
    //  [7] -> [7 -> 0] -> C
    //  Cycles: 8
    //  Bytes: 2
    //  Flags:
    //      Z: Set if result is 0.
    //      N: 0
    //      H: 0
    //      C: Set according to result.
     unsigned char temp = *reg;
     *reg = (temp >> 1) | (temp & 0x80);
     cpu->flags.z = (*reg == 0);;
     cpu->flags.n = 0;
     cpu->flags.h = 0;
     cpu->flags.c = temp & 1;
     cpu->pc+=2;
     return 8;
}

int srl(CPU *cpu, unsigned char *reg)
{
    //  SRL reg : Shift Right Logically register reg.
    //  0 -> [7 -> 0] -> C
    //  Cycles: 8
    //  Bytes: 2
    //  Flags:
    //      Z: Set if result is 0.
    //      N: 0
    //      H: 0
    //      C: Set according to result.
     unsigned char temp = *reg;
     *reg = temp >> 1;
     cpu->flags.z = (*reg == 0);;
     cpu->flags.n = 0;
     cpu->flags.h = 0;
     cpu->flags.c = temp & 1;
     cpu->pc+=2;
     return 8;
}

int swap_nibble(CPU *cpu, unsigned char *reg)
{
    //  SWAP reg : Swap the upper 4 bits in register reg and the lower 4 ones.
    //  Cycles: 8
    //  Bytes: 2
    //  Flags:
    //      Z: Set if result is 0.
    //      N: 0
    //      H: 0
    //      C: 0
     *reg = (*reg << 4 ) | (*reg >> 4);
     cpu->flags.z = (*reg == 0);;
     cpu->flags.n = 0;
     cpu->flags.h = 0;
     cpu->flags.c = 0;
     cpu->pc+=2;
     return 8;
}

int bit(CPU *cpu, unsigned char *reg, int bit_num)
{
    //  BIT u3,reg : Test bit u3 in register reg, set the zero flag if bit not set.
    //  Cycles: 8
    //  Bytes: 2
    //  Flags:
    //      Z: Set if the selected bit is 0.
    //      N: 0
    //      H: 1
    cpu->flags.z = !(*reg & (1 << bit_num));
    cpu->flags.n = 0;
    cpu->flags.h = 1;
    cpu->pc+=2;
    return 8;
}

int bit_hl(CPU *cpu, int bit_num)
{
    //  BIT u3,(HL) : Test bit u3 in the byte pointed by HL, set the zero flag if bit not set.
    //  Cycles: 12
    //  Bytes: 2
    //  Flags:
    //      Z: Set if the selected bit is 0.
    //      N: 0
    //      H: 1
    unsigned short address = (cpu->h<<8) | cpu->l;
    unsigned char value = memory_read(cpu, address);
    cpu->flags.z = !(value & (1 << bit_num));
    cpu->flags.n = 0;
    cpu->flags.h = 1;
    cpu->pc+=2;
    return 12;
}

int res_bit(CPU *cpu, unsigned char *reg, int bit_num)
{
    //  BIT u3,reg : Set bit u3 in register reg to 0. Bit 0 is the rightmost one, bit 7 the leftmost one.
    //  Cycles: 8
    //  Bytes: 2
    //  Flags: None affected.
    *reg =  *reg & ~(1 << bit_num);
    cpu->pc+=2;
    return 8;
}

int res_bit_hl(CPU *cpu, int bit_num)
{
    //  BIT u3,reg : Set bit u3 in the byte pointed by HL to 0. Bit 0 is the rightmost one, bit 7 the leftmost one.
    //  Cycles: 16
    //  Bytes: 2
    //  Flags: None affected.
    unsigned short address = (cpu->h<<8) | cpu->l;
    unsigned char value = memory_read(cpu, address);
    value =  value & ~(1 << bit_num);
    memory_write(cpu, address, value);
    cpu->pc+=2;
    return 16;
}

int set_bit(CPU *cpu, unsigned char *reg, int bit_num)
{
    //  BIT u3,reg : Set bit u3 in register reg to 1. Bit 0 is the rightmost one, bit 7 the leftmost one.
    //  Cycles: 8
    //  Bytes: 2
    //  Flags: None affected.
    *reg =  *reg | (1 << bit_num);
    cpu->pc+=2;
    return 8;
}

int set_bit_hl(CPU *cpu, int bit_num)
{
    //  BIT u3,reg : Set bit u3 in the byte pointed by HL to 1. Bit 0 is the rightmost one, bit 7 the leftmost one.
    //  Cycles: 16
    //  Bytes: 2
    //  Flags: None affected.
    unsigned short address = (cpu->h<<8) | cpu->l;
    unsigned char value = memory_read(cpu, address);
    value =  value | (1 << bit_num);
    memory_write(cpu, address, value);
    cpu->pc+=2;
    return 16;
}


void cpu_init(CPU *cpu)
{
    cpu->pc = 0x100;
    cpu->a = 0x01;
    cpu->flags.z = 1;
    cpu->flags.n = 0;
    cpu->flags.h = 1;
    cpu->flags.c = 1;
    cpu->flags.interrupts_enabled = 1;
    cpu->flags.halt = 0;
    cpu->b = 0x00;
    cpu->c = 0x13;
    cpu->d = 0x00;
    cpu->e = 0xd8;
    cpu->h = 0x01;
    cpu->l = 0x4d;
    cpu->sp =0xfffe;
    cpu->memory[0xff05] = 0x00;  //  TIMA
    cpu->memory[0xff06] = 0x00;  //  TMA
    cpu->memory[0xff07] = 0x00;  //  TAC
    cpu->memory[0xff10] = 0x80;  //  NR10
    cpu->memory[0xff11] = 0xBf;  //  NR11
    cpu->memory[0xff12] = 0xf3;  //  NR12
    cpu->memory[0xff14] = 0xbf;  //  NR14
    cpu->memory[0xff16] = 0x3f;  //  NR21
    cpu->memory[0xff17] = 0x00;  //  NR22
    cpu->memory[0xff19] = 0xbf;  //  NR24
    cpu->memory[0xff1a] = 0x7f;  //  NR30
    cpu->memory[0xff1b] = 0xff;  //  NR31
    cpu->memory[0xff1c] = 0x9f;  //  NR32
    cpu->memory[0xff1e] = 0xbf;  //  NR33
    cpu->memory[0xff20] = 0xff;  //  NR41
    cpu->memory[0xff21] = 0x00;  //  NR42
    cpu->memory[0xff22] = 0x00;  //  NR43
    cpu->memory[0xff23] = 0xbf;  //  NR30 ****
    cpu->memory[0xff24] = 0x77;  //  NR50
    cpu->memory[0xff25] = 0xf3;  //  NR51
    cpu->memory[0xff26] = 0xf1;  //  NR52
    cpu->memory[0xff40] = 0x91;  //  LCDC
    cpu->memory[0xff42] = 0x00;  //  SCY
    cpu->memory[0xff43] = 0x00;  //  SCX
    cpu->memory[0xff45] = 0x00;  //  LYC
    cpu->memory[0xff47] = 0xfc;  //  BGP
    cpu->memory[0xff48] = 0xff;  //  BGP0
    cpu->memory[0xff49] = 0xff;  //  BGP1
    cpu->memory[0xff4a] = 0x00;  //  WY
    cpu->memory[0xff4b] = 0x00;  //  WX
    cpu->memory[0xffff] = 0x00;  //  EI
}


