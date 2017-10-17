#include "opcode.h"
#include "sim_structs.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

void CPU(const struct PCB* head);
void Decode(unsigned int instruction, struct Decode_block* block, struct CPU* CPU);
void Decode_B_I_FORM(unsigned int instruction, struct Decode_block* block, struct CPU* CPU);
void Decode_JUMP_FORM(unsigned int instruction, struct Decode_block* block, struct CPU* CPU);
void Decode_IO_FORM(unsigned int instruction, struct Decode_block* block, struct CPU* CPU);
void Decode_ART_FORM(unsigned int instruction, struct Decode_block* block, struct CPU* CPU);
void Dispatcher(struct CPU* CPU, const struct PCB* head);
void Execute(struct CPU* CPU, struct Decode_block* block);
unsigned int Fetch(const unsigned int* program_counter);
void Loader(const char* file_name, struct PCB* job_queue);
const unsigned int* Memory(const unsigned int address, const char* location, const char* mode);
const unsigned int* Scheduler(struct PCB* head);
struct PCB* StripCard(const char* card, struct PCB* latest_job);

int main(){
  //create initial Process, which serves as starting point for process queue
  struct PCB init;
  struct PCB* current_job = &init;
  
  //set dummy information about init !!need for correct pointer assignment to new jobs!!
  init.program_start = Memory(0, "Disk", NULL);
  init.jobID = 0;
  init.jobInstructionSize = 0;
  init.jobTotalSize = 0;
  
  Loader("Program-File.txt", current_job);
  current_job = current_job->next_job;
  while(current_job != NULL){
    Scheduler(current_job);
    CPU(current_job);
    current_job = current_job->next_job;
    //WaitForInterrupt();
  }
  exit(0);
}

void CPU(const struct PCB* head){
  //CPU set up
  static struct CPU CPU;
  static struct CPU* CPU_ptr = &CPU;
  printf("Running Job #%u\n", head->jobID);
  //set CPU info for next job
  Dispatcher(CPU_ptr, head);
  do{
    CPU.CPU_registers.i_register = Fetch(CPU.program_counter);
    
    struct Decode_block* block = malloc(sizeof(struct Decode_block));
    Decode(CPU.CPU_registers.i_register, block, CPU_ptr);
    Execute(CPU_ptr, block);
    free(block);
    ++(CPU_ptr->program_counter);
  }while(CPU.program_counter != head->program_end + 1);
}

void Decode(unsigned int instruction, struct Decode_block* block, struct CPU* CPU){
  //parse format and decode further
  unsigned int form = instruction & FORM_MASK;
  if(form == B_I_FORM){
    block->format = B_I_FORM;
    Decode_B_I_FORM(instruction, block, CPU);
  }
  else if(form == JUMP_FORM){
    block->format = JUMP_FORM;
    Decode_JUMP_FORM(instruction, block, CPU);
  }
  else if(form == IO_FORM){
    block->format = IO_FORM;
    Decode_IO_FORM(instruction, block, CPU);
  }
  else{
    block->format = ART_FORM;
    Decode_ART_FORM(instruction, block, CPU);
  }
}

void Decode_B_I_FORM(unsigned int instruction, struct Decode_block* block, struct CPU* CPU){
  //parse opcode
  unsigned int code = instruction & OPCODE_MASK;
  if(code == ST)
    block->opcode = ST;
  else if(code == LW)
    block->opcode = LW;
  else if(code == MOVI)
    block->opcode = MOVI;
  else if(code == ADDI)
    block->opcode = ADDI;
  else if(code == MULI)
    block->opcode = MULI;
  else if(code == DIVI)
    block->opcode = DIVI;
  else if(code == LDI)
    block->opcode = LDI;
  else if(code == SLTI)
    block->opcode = SLTI;
  else if(code == BEQ)
    block->opcode = BEQ;
  else if(code == BNE)
    block->opcode = BNE;
  else if(code == BEZ)
    block->opcode = BEZ;
  else if(code == BNZ)
    block->opcode = BNZ;
  else if(code == BGZ)
    block->opcode = BGZ;
  else
    block->opcode = BLZ;

  //B-reg
  block->reg1 = CPU->CPU_registers.reg_set;
  block->reg1 += ((instruction & REG1_MASK) >> REG1_RSHIFT);

  //D-reg
  block->reg2 = CPU->CPU_registers.reg_set;
  block->reg2 += ((instruction & REG2_MASK) >> REG2_RSHIFT);

  //reg3
  block->reg3 = NULL;
  
  //address in words away from current program_counter
  block->address = (instruction & B_I_ADDRESS);
}

void Decode_JUMP_FORM(unsigned int instruction, struct Decode_block* block, struct CPU* CPU){
  //parse opcode
  unsigned int code = instruction & OPCODE_MASK;
  if(code == HLT)
    block->opcode = HLT;
  else
    block->opcode = JMP;

  //reg1 unused
  block->reg1 = NULL;

  //reg2 unused
  block->reg2 = NULL;

  //reg3 unused
  block->reg3 = NULL;

  //address of jump
  block->address = instruction & JUMP_ADDRESS;
}

void Decode_IO_FORM(unsigned int instruction, struct Decode_block* block, struct CPU* CPU){
  //parse opcode
  unsigned int code = instruction & OPCODE_MASK;
  if(code == WR)
    block->opcode = WR;
  else
    block->opcode = RD;

  //Reg1
  block->reg1 = CPU->CPU_registers.reg_set;
  block->reg1 += ((instruction & REG1_MASK) >> REG1_RSHIFT);

  //Reg2
  block->reg2 = CPU->CPU_registers.reg_set;
  block->reg2 += ((instruction & REG2_MASK) >> REG2_RSHIFT);

  //reg3 unused
  block->reg3 = NULL;

  //address in words away from current program_counter
  block->address = (instruction & IO_ADDRESS);
}

void Decode_ART_FORM(unsigned int instruction, struct Decode_block* block, struct CPU* CPU){
  //parse opcode
  unsigned int code = instruction & OPCODE_MASK;
  if(code == MOV)
      block->opcode = MOV;
    else if(code == ADD)
      block->opcode = ADD;
    else if(code == SUB)
      block->opcode = SUB;
    else if(code == MUL)
      block->opcode = MUL;
    else if(code == DIV)
      block->opcode = DIV;
    else if(code == AND)
      block->opcode = AND;
    else if(code == OR)
      block->opcode = OR;
    else
      block->opcode = SLT;

  //S-reg 1
  block->reg1 = CPU->CPU_registers.reg_set;
  block->reg1 += ((instruction & REG1_MASK) >> REG1_RSHIFT);

  //S-reg 2
  block->reg2 = CPU->CPU_registers.reg_set;
  block->reg2 += ((instruction & REG2_MASK) >> REG2_RSHIFT);

  //D-reg 3
  block->reg3 = CPU->CPU_registers.reg_set;
  block->reg3 += ((instruction & REG3_MASK) >> REG3_RSHIFT);
}

void Dispatcher(struct CPU* CPU_ptr, const struct PCB* head){
  CPU_ptr->currentJobID = head->jobID;
  CPU_ptr->current_job_start = head->program_start;
  CPU_ptr->current_job_end = head->program_end;
  CPU_ptr->program_counter = head->program_counter;
  
  //reset registers
  for(int i = 0; i != 16; ++i){
  	CPU_ptr->CPU_registers.reg_set[i].data = 0;
  	CPU_ptr->CPU_registers.reg_set[i].isData = 1;
  	CPU_ptr->CPU_registers.reg_set[i].isAddress = 0;
  }
}

void Execute(struct CPU* CPU, struct Decode_block* block){
  if(block->format == B_I_FORM)
    Execute_B_I_FORM(CPU, block);
  else if(block->format == JUMP_FORM)
    Execute_JUMP_FORM(CPU, block);
  else if(block->format == IO_FORM)
    Execute_IO_FORM(CPU, block);
  else
    Execute_ART_FORM(CPU, block);
}

unsigned int Fetch(const unsigned int* program_counter){
  return *program_counter;
}

void Loader(const char* file_name, struct PCB* job_queue){
  //open program file
  FILE* file_ptr = fopen(file_name, "r");

  //string to hold next line
  char next_line[30];
  
  //used to allow progression down job queue when adding jobs
  struct PCB* latest_job = job_queue;
  
  while(!feof(file_ptr)){
    //parse next line
    fgets(next_line, 30, file_ptr);
    
    //check line for control card
    if(next_line[1] != 'x'){
      latest_job = StripCard(next_line, latest_job);
    }
    else
      Memory(strtoul(next_line, NULL, 16), "Disk", "w");
  }

  //close Program-File
  fclose(file_ptr);
}

const unsigned int* Memory(const unsigned int address, const char* location, const char* mode){
  static unsigned int Disk[2048];
  static unsigned int RAM[1024];

  //maintains the next spot in which a value is to be written into Disk or RAM;
  static unsigned int nextWriteDisk = 0;
  static unsigned int nextWriteRAM = 0;

  //used to return value, but prevent changing it
  const unsigned int* datum_location;
  
  //check to see if Disk or RAM is to be accessed, the if its a read or write request
  if(location == "Disk"){

    //check for NULL mode (asks for first spot, used only by first init)
    if(mode != NULL){

      //check for read or write request
      if(mode == "r"){
	datum_location = Disk + address;
      }
      else{
	datum_location = Disk + nextWriteDisk;
	Disk[nextWriteDisk] = address;
	nextWriteDisk = (nextWriteDisk + 1) % sizeof(Disk);
      }
    }
    else
      datum_location = Disk;
  }
  else{
    if(mode == "r"){
      datum_location = RAM + address;
    }
    else{
      datum_location = RAM + nextWriteRAM;
      RAM[nextWriteRAM] = address;
      nextWriteRAM = (nextWriteRAM + 1) % sizeof(RAM);
    }
  }
  
  return datum_location;
}

const unsigned int* Scheduler(struct PCB* head){
  //cursor to point to beginning of program on Disk
  const unsigned int* instruct_cursor = head->program_start;
  
  //set first instruction into RAM and change program start pointer
  head->program_start = Memory(*(instruct_cursor), "RAM", "w");
  ++instruct_cursor;

  //loop until all instructions and buffers are inserted into RAM
  for(unsigned int count = 0; count != head->jobTotalSize - 1; ++count){
    Memory((*instruct_cursor), "RAM", "w");
    ++instruct_cursor;
  }

  //recalculate location pointers in head
  head->program_counter = head->program_start;
  head->program_end = head->program_start + head->jobInstructionSize - 1;

  //recalculate buffer pointers in head
  head->in_buff_ptr = head->program_end + 1;
  head->out_buff_ptr = head->in_buff_ptr + head->inBuffSize;
  head->temp_buff_ptr = head->out_buff_ptr + head->outBuffSize;
}

struct PCB* StripCard(const char* card, struct PCB* latest_job){
  char* card_cursor;
  
  //if JOB, create PCB for process
  if((card_cursor = strstr(card, "JOB")) != NULL){
    //make new PCB
    struct PCB* new_job = malloc(sizeof(struct PCB));
    latest_job->next_job = new_job;

    //record card values in PCB
    unsigned int reads = 0;
    card_cursor += 4;
    while(reads < 3){
      if(reads == 0)
	new_job->jobID = strtoul(card_cursor, &card_cursor, 16);
      else if(reads == 1)
	new_job->jobInstructionSize = strtoul(card_cursor, &card_cursor, 16);
      else
	new_job->jobPriority = strtoul(card_cursor, &card_cursor, 16);
      ++reads;
    }
    
    //set program pointers
    new_job->program_start = latest_job->program_start + latest_job->jobTotalSize;
    new_job->program_counter = new_job->program_start;
    new_job->program_end = new_job->program_start + new_job->jobInstructionSize - 1;
    
    return new_job;
  }

  //if Data, fill in buffer info
  else if((card_cursor = strstr(card, "Data")) != NULL){
    //record card values in PCB
    unsigned int reads = 0;
    card_cursor += 5;
    while(reads < 3){
      if(reads == 0)
	latest_job->inBuffSize = strtoul(card_cursor, &card_cursor, 16);
      else if(reads == 1)
	latest_job->outBuffSize = strtoul(card_cursor, &card_cursor, 16);
      else
	latest_job->tempBuffSize = strtoul(card_cursor, &card_cursor, 16);
      ++reads;
    }

    //set buffer pointers and total size
    latest_job->jobTotalSize = latest_job->jobInstructionSize + latest_job->inBuffSize + latest_job->outBuffSize + latest_job->tempBuffSize;
    latest_job->in_buff_ptr = latest_job->program_end + 1;
    latest_job->out_buff_ptr = latest_job->in_buff_ptr + latest_job->inBuffSize;
    latest_job->temp_buff_ptr = latest_job->out_buff_ptr + latest_job->outBuffSize;
    
    return latest_job;
  }

  //if END, do nothing
  else
    return latest_job;
}
