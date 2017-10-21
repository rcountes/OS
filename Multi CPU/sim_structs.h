#ifndef SIM_STRUCTS_H
#define SIM_STRUCTS_H
struct Register{
	unsigned int data;
	unsigned int* address;
	
	unsigned int isData;
	unsigned int isAddress;
};

struct Register_set{
  	unsigned int i_register;
  	struct Register reg_set[16];
};

struct CPU{
	//job info
	unsigned int currentJobID;
  	unsigned int* current_job_start;
  	unsigned int* current_job_end;
  	unsigned int* program_counter;
  	
  	//cpu registers
  	struct Register_set CPU_registers;
  	
  	//cache
  	unsigned int cache[100];
};

struct Decode_block{
  	unsigned int format;
  	unsigned int opcode;

  	//B_I_FORM: 	reg1 == B-reg,	reg2 == D-reg,	reg3 == NULL,	address == address
  	//JUMP_FORM: 	reg1 == NULL,	reg2 == NULL,	reg3 == NULL,	address == address
  	//IO_FORM:	reg1 == reg1,	reg2 == reg2,	reg3 == NULL,	address == address
  	//ART_FORM:	reg1 == S-reg1,	reg2 == S-reg2, reg3 == D-reg,	address == 0
  	struct Register* reg1, * reg2, * reg3;
  	unsigned int address;
};

//clear block data
void ZeroBlock(struct Decode_block* block){
	block->format = 0;
	block->opcode = 0;
	block->reg1 = NULL;
	block->reg2 = NULL;
	block->reg3 = NULL;
	block->address = 0;
}

struct PCB{
  	unsigned int jobID;
  	//job size in words
  	unsigned int jobInstructionSize;
  	unsigned int jobTotalSize;
  	unsigned int jobPriority;

  	//program_counter points to the current instruction
  	//program_start points to the first instruction of the process
  	//program_end points to the last instruction of the process
  	const unsigned int* program_counter;
  	const unsigned int* program_start;
  	const unsigned int* program_end;
  	
  	//program_disk_start is permanently bound to the start of a job when inserted into the disk
  	const unsigned int const* program_disk_start;

  	//pointers and sizes to/of data buffers
  	const unsigned int* in_buff_ptr;
  	const unsigned int* out_buff_ptr;
  	const unsigned int* temp_buff_ptr;
  	unsigned int inBuffSize, outBuffSize, tempBuffSize;

  	//register images
  	struct Register_set reg_image;
  
  	//pointer to next PCB
  	struct PCB* next_job;
};

//DECODING MASKS
//format
const unsigned int ART_FORM = 0x00000000;
const unsigned int B_I_FORM = 0x40000000;
const unsigned int JUMP_FORM = 0x80000000;
const unsigned int IO_FORM = 0xC0000000;
const unsigned int FORM_MASK = 0xC0000000;

//opcode
const unsigned int RD = 0x00000000;
const unsigned int WR = 0x01000000;
const unsigned int ST = 0x02000000;
const unsigned int LW = 0x03000000;
const unsigned int MOV = 0x04000000;
const unsigned int ADD = 0x05000000;
const unsigned int SUB = 0x06000000;
const unsigned int MUL = 0x07000000;
const unsigned int DIV = 0x08000000;
const unsigned int AND = 0x09000000;
const unsigned int OR = 0x0A000000;
const unsigned int MOVI = 0x0B000000;
const unsigned int ADDI = 0x0C000000;
const unsigned int MULI = 0x0D000000;
const unsigned int DIVI = 0x0E000000;
const unsigned int LDI = 0x0F000000;
const unsigned int SLT = 0x10000000;
const unsigned int SLTI = 0x11000000;
const unsigned int HLT = 0x12000000;
const unsigned int NOP = 0x13000000;
const unsigned int JMP = 0x14000000;
const unsigned int BEQ = 0x15000000;
const unsigned int BNE = 0x16000000;
const unsigned int BEZ = 0x17000000;
const unsigned int BNZ = 0x18000000;
const unsigned int BGZ = 0x19000000;
const unsigned int BLZ = 0x1A000000;
const unsigned int OPCODE_MASK = 0x3F000000;

//registers
const unsigned int REG1_MASK = 0x00F00000;
const unsigned int REG2_MASK = 0x000F0000;
const unsigned int REG3_MASK = 0x0000F000;

//register shifts
const unsigned int REG1_RSHIFT = 20;
const unsigned int REG2_RSHIFT = 16;
const unsigned int REG3_RSHIFT = 12;

//addresses
const unsigned int B_I_ADDRESS = 0x0000FFFF;
const unsigned int JUMP_ADDRESS = 0x00FFFFFF;
const unsigned int IO_ADDRESS = 0x0000FFFF;
#endif
