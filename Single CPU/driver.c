#include "opcode.h"
#include "queue.h"
#include "sim_structs.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

//CPU modules
void CPU(const struct PCB* head);
void Decode(unsigned int instruction, struct Decode_block* block, struct CPU* CPU);
void Dispatcher(struct CPU* CPU, const struct PCB* head);
void Execute(struct CPU* CPU, struct Decode_block* block);
unsigned int Fetch(const unsigned int* program_counter);
void Loader(const char* file_name, struct Queue* newQ);
const unsigned int* Memory(const unsigned int address, const char* location, const char* mode);
const unsigned int* Scheduler(struct Queue* newQ, struct Queue* readyQ);

//helper modules
void Decode_B_I_FORM(unsigned int instruction, struct Decode_block* block, struct CPU* CPU);
void Decode_JUMP_FORM(unsigned int instruction, struct Decode_block* block, struct CPU* CPU);
void Decode_IO_FORM(unsigned int instruction, struct Decode_block* block, struct CPU* CPU);
void Decode_ART_FORM(unsigned int instruction, struct Decode_block* block, struct CPU* CPU);
void StripCard(const char* card, struct Queue* newQ);

int main(){
	//start timer 
	clock_t totalStart, totalEnd;
	totalStart = clock();

	//create and allocate memory to queue pointers
	struct Queue* newQ = malloc(sizeof(struct Queue));
	struct Queue* waitQ = malloc(sizeof(struct Queue));
	struct Queue* readyQ = malloc(sizeof(struct Queue));
	
	//load Program-File.txt into disk and create jobs
  	Loader("Program-File.txt", newQ);
  	
  	//loop until all queues are empty
  	while((newQ->head != NULL) || (readyQ->head != NULL) || (waitQ->head != NULL)){
    	Scheduler(newQ, readyQ);
    	CPU(readyQ->head);
    	DeleteJob(readyQ);
    	//WaitForInterrupt();
  	}
  	
  	//stop total timer and output total execution time
  	totalEnd = clock();
  	printf("Total elapsed time for execution was %u.\n", totalEnd - totalStart);
  	exit(0);
}

void CPU(const struct PCB* head){
	//start job timer
	clock_t jobStart, jobEnd;
	jobStart = clock();

  	//CPU and Decoding block set up
  	static struct CPU CPU;
  	static struct CPU* CPU_ptr = &CPU;
  	struct Decode_block* block = malloc(sizeof(struct Decode_block));
  	
  	//set CPU info for next job
  	Dispatcher(CPU_ptr, head);
  	do{
  		//get next instruction
    	CPU.CPU_registers.i_register = Fetch(CPU.program_counter);
    
    	Decode(CPU.CPU_registers.i_register, block, CPU_ptr);
    	Execute(CPU_ptr, block);
    	++(CPU_ptr->program_counter);
  	}while(CPU.program_counter != head->program_end + 1);
  	
  	//free decode_block space
  	free(block);
  	
  	//print job info
  	jobEnd = clock();
  	printf("Job #%u, Job Size %u, Execution Time %ums\n", head->jobID, head->jobTotalSize, jobEnd - jobStart);
  	
  	//SCAN RAM
  	//printf("Job %u results\n", head->jobID);
  	//for(int i = 0; i < head->jobTotalSize; ++i)
  	//	printf("%u\n", *(Memory(i, "RAM", "r")));
  	//printf("\n");
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

void Loader(const char* file_name, struct Queue* newQ){
  //open program file
  FILE* file_ptr = fopen(file_name, "r");

  //string to hold next line
  char next_line[30];
  
  while(!feof(file_ptr)){
    //parse next line
    fgets(next_line, 30, file_ptr);
    
    //check line for control card
    if(next_line[1] != 'x'){
      StripCard(next_line, newQ);
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
    if(mode != "wNew"){

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
  	if(mode != "wNew"){
    	if(mode == "r"){
      		datum_location = RAM + address;
    	}
    	else{
      		datum_location = RAM + nextWriteRAM;
      		RAM[nextWriteRAM] = address;
      		nextWriteRAM = (nextWriteRAM + 1) % sizeof(RAM);
    	}
    }
    else{
    	datum_location = RAM;
    	nextWriteRAM = 0;
    }
  }
  
  return datum_location;
}

const unsigned int* Scheduler(struct Queue* newQ, struct Queue* readyQ){
  //cursor to point to beginning of program on Disk
  const unsigned int* instruct_cursor = newQ->head->program_disk_start;
  
  //announce to Memory a new job
  Memory(0, "RAM", "wNew");
  
  //set first instruction into RAM and change program start pointer
  newQ->head->program_start = Memory(*(instruct_cursor), "RAM", "w");
  ++instruct_cursor;

  //loop until all instructions and buffers are inserted into RAM
  for(unsigned int count = 0; count != newQ->head->jobTotalSize - 1; ++count){
    Memory((*instruct_cursor), "RAM", "w");
    ++instruct_cursor;
  }

  //recalculate location pointers in head
  newQ->head->program_counter = newQ->head->program_start;
  newQ->head->program_end = newQ->head->program_start + newQ->head->jobInstructionSize - 1;

  //recalculate buffer pointers in head
  newQ->head->in_buff_ptr = newQ->head->program_end + 1;
  newQ->head->out_buff_ptr = newQ->head->in_buff_ptr + newQ->head->inBuffSize;
  newQ->head->temp_buff_ptr = newQ->head->out_buff_ptr + newQ->head->outBuffSize;
  
  //move job from newQ to readyQ (non-priority)
  MoveJob(newQ, readyQ);
  //move job from newQ to readyQ (priority)
  //MoveHighestPJob(newQ, readyQ);
}

void StripCard(const char* card, struct Queue* newQ){
  char* card_cursor;
  
  //if JOB, create PCB for process
  if((card_cursor = strstr(card, "JOB")) != NULL){
  	//temp pointer for old tail
  	struct PCB* old_tail = newQ->tail;
  	
    //make new PCB
    struct PCB* new_job = malloc(sizeof(struct PCB));
    
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
    
    //add job to newQ
    AddJob(newQ, new_job);
    
    //if there was no older job, set defualt values
    if(old_tail == NULL){
    	newQ->tail->program_disk_start = Memory(0, "Disk", "wNew");
    	newQ->tail->program_start = newQ->tail->program_disk_start;
    	newQ->tail->program_counter = newQ->tail->program_start;
    	newQ->tail->program_end = newQ->tail->program_start + newQ->tail->jobInstructionSize - 1;
    }
    //else, set program pointers based on older job
    else{
    	newQ->tail->program_disk_start = old_tail->program_start + old_tail->jobTotalSize;
    	newQ->tail->program_start = old_tail->program_start + old_tail->jobTotalSize;
    	newQ->tail->program_counter = newQ->tail->program_start;
    	newQ->tail->program_end = newQ->tail->program_start + newQ->tail->jobInstructionSize - 1;
    }
  }

  //if Data, fill in buffer info
  else if((card_cursor = strstr(card, "Data")) != NULL){
    //record card values in PCB
    unsigned int reads = 0;
    card_cursor += 5;
    while(reads < 3){
      if(reads == 0)
	newQ->tail->inBuffSize = strtoul(card_cursor, &card_cursor, 16);
      else if(reads == 1)
	newQ->tail->outBuffSize = strtoul(card_cursor, &card_cursor, 16);
      else
	newQ->tail->tempBuffSize = strtoul(card_cursor, &card_cursor, 16);
      ++reads;
    }

    //set buffer pointers and total size
    newQ->tail->jobTotalSize = newQ->tail->jobInstructionSize + newQ->tail->inBuffSize + newQ->tail->outBuffSize + newQ->tail->tempBuffSize;
    newQ->tail->in_buff_ptr = newQ->tail->program_end + 1;
    newQ->tail->out_buff_ptr = newQ->tail->in_buff_ptr + newQ->tail->inBuffSize;
    newQ->tail->temp_buff_ptr = newQ->tail->out_buff_ptr + newQ->tail->outBuffSize;
  }

  //if END, do nothing
  else
    ;
}
