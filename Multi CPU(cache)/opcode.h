#ifndef OPCODE_H
#define OPCODE_H

#include<stdlib.h>
#include<stdio.h>
#include "sim_structs.h"
//execution controllers
void Execute_B_I_FORM(struct CPU* CPU, struct Decode_block* block);
void Execute_JUMP_FORM(struct CPU* CPU, struct Decode_block* block);
void Execute_IO_FORM(struct CPU* CPU, struct Decode_block* block);
void Execute_ART_FORM(struct CPU* CPU, struct Decode_block* block);

//opcode functions
void op_RD(struct CPU* CPU, struct Decode_block* block);
void op_WR(struct CPU* CPU, struct Decode_block* block);
void op_ST(struct CPU* CPU, struct Decode_block* block);
void op_LW(struct CPU* CPU, struct Decode_block* block);
void op_MOV(struct CPU* CPU, struct Decode_block* block);
void op_ADD(struct CPU* CPU, struct Decode_block* block);
void op_SUB(struct CPU* CPU, struct Decode_block* block);
void op_MUL(struct CPU* CPU, struct Decode_block* block);
void op_DIV(struct CPU* CPU, struct Decode_block* block);
void op_AND(struct CPU* CPU, struct Decode_block* block);
void op_OR(struct CPU* CPU, struct Decode_block* block);
void op_MOVI(struct CPU* CPU, struct Decode_block* block);
void op_ADDI(struct CPU* CPU, struct Decode_block* block);
void op_MULI(struct CPU* CPU, struct Decode_block* block);
void op_DIVI(struct CPU* CPU, struct Decode_block* block);
void op_LDI(struct CPU* CPU, struct Decode_block* block);
void op_SLT(struct CPU* CPU, struct Decode_block* block);
void op_SLTI(struct CPU* CPU, struct Decode_block* block);
void op_HLT(struct CPU* CPU, struct Decode_block* block);
void op_NOP(struct CPU* CPU, struct Decode_block* block);
void op_JMP(struct CPU* CPU, struct Decode_block* block);
void op_BEQ(struct CPU* CPU, struct Decode_block* block);
void op_BNE(struct CPU* CPU, struct Decode_block* block);
void op_BEZ(struct CPU* CPU, struct Decode_block* block);
void op_BNZ(struct CPU* CPU, struct Decode_block* block);
void op_BGZ(struct CPU* CPU, struct Decode_block* block);
void op_BLZ(struct CPU* CPU, struct Decode_block* block);

void Execute_B_I_FORM(struct CPU* CPU, struct Decode_block* block){
  if(block->opcode == ST)
    op_ST(CPU, block);
  else if(block->opcode == LW)
    op_LW(CPU, block);
  else if(block->opcode == MOVI)
    op_MOVI(CPU, block);
  else if(block->opcode == ADDI)
    op_ADDI(CPU, block);
  else if(block->opcode == MULI)
    op_MULI(CPU, block);
  else if(block->opcode == DIVI)
    op_DIVI(CPU, block);
  else if(block->opcode == LDI)
    op_LDI(CPU, block);
  else if(block->opcode == SLTI)
    op_SLTI(CPU, block);
  else if(block->opcode == BEQ)
    op_BEQ(CPU, block);
  else if(block->opcode == BNE)
    op_BNE(CPU, block);
  else if(block->opcode == BEZ)
    op_BEZ(CPU, block);
  else if(block->opcode == BNZ)
    op_BNZ(CPU, block);
  else if(block->opcode == BGZ)
    op_BGZ(CPU, block);
  else
    op_BLZ(CPU, block);
}

void Execute_JUMP_FORM(struct CPU* CPU, struct Decode_block* block){
  if(block->opcode == HLT)
    op_HLT(CPU, block);
  else
    ;
}

void Execute_IO_FORM(struct CPU* CPU, struct Decode_block* block){
  if(block->opcode == WR)
    op_WR(CPU, block);
  else
    op_RD(CPU, block);
}

void Execute_ART_FORM(struct CPU* CPU, struct Decode_block* block){
  if(block->opcode == MOV)
    op_MOV(CPU, block);
  else if(block->opcode == ADD)
    op_ADD(CPU, block);
  else if(block->opcode == SUB)
    op_SUB(CPU, block);
  else if(block->opcode == MUL)
    op_MUL(CPU, block);
  else if(block->opcode == DIV)
    op_DIV(CPU, block);
  else if(block->opcode == AND)
    op_AND(CPU, block);
  else if(block->opcode == OR)
    op_OR(CPU, block);
  else
    op_SLT(CPU, block);
}

void op_RD(struct CPU* CPU, struct Decode_block* block){
  //store the contents of reg2 into reg1
  if(block->address == 0){
  	if(block->reg2->isAddress)
  		block->reg1->data = *(block->reg2->address);
  	else
  		block->reg1->data = block->reg2->data;
  }
    
  //store data at start of program + address offset int reg1
  else
    block->reg1->data = *(CPU->current_job_start_cache + (block->address / 4));
    
  //set reg1 markers
  block->reg1->isData = 1;
  block->reg1->isAddress = 0;
}

void op_WR(struct CPU* CPU, struct Decode_block* block){
	if(block->address == 0)
		*(block->reg2->address) = block->reg1->data;
	else
		*(CPU->current_job_start_cache + (block->address / 4)) = block->reg1->data;
}

void op_ST(struct CPU* CPU, struct Decode_block* block){
	//store reg1 data at address pointed to by reg2 address
	*(block->reg2->address) = block->reg1->data;
}

void op_LW(struct CPU* CPU, struct Decode_block* block){
	block->reg2->data = *(block->reg1->address + block->address);
	block->reg2->isData = 1;
	block->reg2->isAddress = 0;
}

void op_MOV(struct CPU* CPU, struct Decode_block* block){
	block->reg1->data = block->reg2->data;
	/*
	//reg1 and reg2 are both 0 which means S-reg is accumulator
	if(((CPU->CPU_registers.i_register & REG1_MASK) == 0) && ((CPU->CPU_registers.i_register & REG2_MASK) == 0))
		block->reg3->data = block->reg2->data;
	else if(((CPU->CPU_registers.i_register & REG1_MASK) != 0) && ((CPU->CPU_registers.i_register & REG2_MASK) == 0))
		block->reg3->data = block->reg1->data;
	else
		block->reg3->data = block->reg2->data;
	*/
}

void op_ADD(struct CPU* CPU, struct Decode_block* block){
	block->reg3->data = block->reg1->data + block->reg2->data;
}

void op_SUB(struct CPU* CPU, struct Decode_block* block){
}

void op_MUL(struct CPU* CPU, struct Decode_block* block){
}

void op_DIV(struct CPU* CPU, struct Decode_block* block){
	block->reg3->data = (block->reg1->data / block->reg2->data);
}

void op_AND(struct CPU* CPU, struct Decode_block* block){
}

void op_OR(struct CPU* CPU, struct Decode_block* block){
}

void op_MOVI(struct CPU* CPU, struct Decode_block* block){
  //if address and base register are 0, set reg1 to zero
  if((block->address == 0) && ((CPU->CPU_registers.i_register & REG1_MASK) == 0)){
    block->reg2->data = 0;
    block->reg2->isData = 1;
  	block->reg2->isAddress = 0;
  }
  else if((block->address == 0) && ((CPU->CPU_registers.i_register & REG1_MASK) != 0)){
  	if(block->reg1->isData){
  		block->reg2->data = block->reg1->data;
  		block->reg2->isData = 1;
  		block->reg2->isAddress = 0;
  	}
  	else{
  		block->reg2->data = *(block->reg1->address);
  		block->reg2->isData = 0;
  		block->reg2->isAddress = 1;
  	}
  }
  else
  	block->reg2->data = block->address;
}

void op_ADDI(struct CPU* CPU, struct Decode_block* block){
	if(block->reg2->isData){
		block->reg2->data += block->address;
	}
	else
		block->reg2->address += (block->address / 4);
}

void op_MULI(struct CPU* CPU, struct Decode_block* block){
}

void op_DIVI(struct CPU* CPU, struct Decode_block* block){
}

void op_LDI(struct CPU* CPU, struct Decode_block* block){
	if(block->address == 0){
		if(block->reg1->isData){
  			block->reg2->data = block->reg1->data;
  			block->reg2->isData = 1;
  			block->reg2->isAddress = 0;
  		}
  		else{
  			block->reg2->address = block->reg1->address;
  			block->reg2->isData = 0;
  			block->reg2->isAddress = 1;
  		}
  	}
  	else{
  		block->reg2->address = CPU->current_job_start_cache + (block->address / 4);
  		block->reg2->isData = 0;
  		block->reg2->isAddress = 1;
  	}
}

void op_SLT(struct CPU* CPU, struct Decode_block* block){
	unsigned int boolean;
	if(block->reg1->isData){
		if(block->reg2->isData)
			boolean = (block->reg1->data < block->reg2->data);
		else
			boolean = (block->reg1->data < *(block->reg2->address));
	}
	else{
		if(block->reg2->isData)
			boolean = *(block->reg1->address) < block->reg2->data;
		else
			boolean = *(block->reg1->address) < *(block->reg2->address);
	}
	block->reg3->data = boolean;
	block->reg3->isData = 1;
	block->reg3->isAddress = 0;
}

void op_SLTI(struct CPU* CPU, struct Decode_block* block){
}

void op_HLT(struct CPU* CPU, struct Decode_block* block){
	;
}

void op_NOP(struct CPU* CPU, struct Decode_block* block){
}

void op_JMP(struct CPU* CPU, struct Decode_block* block){
}

void op_BEQ(struct CPU* CPU, struct Decode_block* block){
	unsigned int boolean;
	if(block->reg1->isData){
		if(block->reg2->isData)
			boolean = (block->reg1->data == block->reg2->data);
		else
			boolean = (block->reg1->data == *(block->reg2->address));
	}
	else{
		if(block->reg2->isData)
			boolean = (*(block->reg1->address) == block->reg2->data);
		else
			boolean = (*(block->reg1->address) == *(block->reg2->address));
	}
	
	if(boolean)
		CPU->program_counter = (CPU->current_job_start_cache + ((block->address / 4) - 1));
	else
		;
}

void op_BNE(struct CPU* CPU, struct Decode_block* block){
	unsigned int boolean;
	if(block->reg1->isData){
		if(block->reg2->isData)
			boolean = (block->reg1->data != block->reg2->data);
		else
			boolean = (block->reg1->data != *(block->reg2->address));
	}
	else{
		if(block->reg2->isData)
			boolean = (*(block->reg1->address) != block->reg2->data);
		else
			boolean = (*(block->reg1->address) != *(block->reg2->address));
	}
	
	if(boolean)
		CPU->program_counter = (CPU->current_job_start_cache + ((block->address / 4) - 1));
	else
		;
}

void op_BEZ(struct CPU* CPU, struct Decode_block* block){
}

void op_BNZ(struct CPU* CPU, struct Decode_block* block){
}

void op_BGZ(struct CPU* CPU, struct Decode_block* block){
}

void op_BLZ(struct CPU* CPU, struct Decode_block* block){
}
#endif
