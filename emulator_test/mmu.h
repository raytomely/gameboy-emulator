unsigned char memory_read(CPU *cpu, unsigned short address);
void memory_write(CPU *cpu, unsigned short address, unsigned char value);
void dma_oam_transfer(CPU *cpu);
unsigned char read_input(unsigned char val);
int load_rom(CPU *cpu, char* rom_name);
void load_nintindo_logo(CPU *cpu);
void rom_init(CPU *cpu);
void load_mbc_rom(CPU *cpu, char* rom_name);
