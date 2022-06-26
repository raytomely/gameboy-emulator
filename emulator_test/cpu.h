typedef struct
{
    unsigned char z; // ZERO FLAG; set to 1 if current operation results in Zero, or two values match on a CMP operation
    unsigned char n; // SUBTRACT FLAG; set to 1 if a subtraction was performed
    unsigned char h; // HALF CARRY FLAG; set to 1 if a carry occured from the lower nibble in the last operation
    unsigned char c; // CARRY FLAG; set to 1 if a carry occured in the last operation or if A is the smaller value on CP instruction
    unsigned char interrupts_enabled; // Interrupt Master Enable Flag IME (Write Only)
    unsigned char halt;

}CPU_FLAGS;


typedef union
{
    unsigned char all_flags;

    struct
    {
        unsigned char pad : 4;
        unsigned char z : 1; // ZERO FLAG;
        unsigned char n : 1; // SUBTRACT FLAG;
        unsigned char h : 1; // HALF CARRY FLAG;
        unsigned char c : 1; // CARRY FLAG;
    };
}CPU_FLAGS2;


typedef struct
{
    unsigned char a;
    unsigned char b;
    unsigned char c;
    unsigned char d;
    unsigned char e;
    unsigned char f;
    unsigned char h;
    unsigned char l;
    unsigned short pc;
    unsigned short sp;
    unsigned char memory[0xFFFF+1];
    CPU_FLAGS flags;
}CPU;


int cpu_emulation(CPU *cpu);
void print_flags(CPU_FLAGS2 *flag);
void test_flags(void);
int rlc(CPU *cpu, unsigned char *reg);
int rrc(CPU *cpu, unsigned char *reg);
int rl(CPU *cpu, unsigned char *reg);
int rr(CPU *cpu, unsigned char *reg);
int sla(CPU *cpu, unsigned char *reg);
int sra(CPU *cpu, unsigned char *reg);
int srl(CPU *cpu, unsigned char *reg);
int swap_nibble(CPU *cpu, unsigned char *reg);
int bit(CPU *cpu, unsigned char *reg, int bit_num);
int bit_hl(CPU *cpu, int bit_num);
int res_bit(CPU *cpu, unsigned char *reg, int bit_num);
int res_bit_hl(CPU *cpu, int bit_num);
int set_bit(CPU *cpu, unsigned char *reg, int bit_num);
int set_bit_hl(CPU *cpu, int bit_num);
void cpu_init(CPU *cpu);
