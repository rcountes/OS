#ifndef QUEUE_H
#define QUEUE_H

#include "sim_structs.h"
//queue is a double ended, generic queue used to implement
//the new, waiting, and ready queues used by the CPU modules

//QUEUE DEFINITION
struct Queue{
	//pointers head and tail of queue
	struct PCB* head;
	struct PCB* tail;
	
	//QUEUE METADATA
	//highest priority in queue
	unsigned int highestPriority;
	//number of jobs in queue
	unsigned int size;
};

//QUEUE MANIPULATORS
void AddJob(struct Queue* Q, struct PCB* job);
void AddJobPriority(struct Queue* Q, struct PCB* job);
void DeleteJob(struct Queue* Q, const unsigned int jobID);
void MoveJob(struct Queue* oldQ, struct Queue* newQ);
void MoveHighestPJob(struct Queue* oldQ, struct Queue* newQ);

//job is being added to queue
void AddJob(struct Queue* Q, struct PCB* job){
	if(Q->size == 0){
		//set head and tail
		Q->head = job;
		Q->tail = job;
		
		//set metadata
		Q->highestPriority = job->jobPriority;
		Q->size = 1;
	}
	else{
		//append job and change tail
		Q->tail->next_job = job;
		Q->tail = job;
		//ensures tail doesnt point outside of queue
		Q->tail->next_job = NULL;
		
		//adjust metadata
		if(Q->highestPriority < job->jobPriority)
			Q->highestPriority = job->jobPriority;
		++(Q->size);
	}
}

//job is being added to queue with priority in mind
void AddJobPriority(struct Queue* Q, struct PCB* job){
	if(Q->head == NULL){
		//set head and tail
		Q->head = job;
		Q->tail = job;
		
		//set metadata
		Q->highestPriority = job->jobPriority;
		Q->size = 1;
	}
	else{
		//ptrs for current and previous job
		struct PCB* prev_job = NULL;
		struct PCB* curr_job = Q->head;
		
		//walk job chain until lower priority found
		while((curr_job != NULL) && (curr_job->jobPriority > job->jobPriority)){
			prev_job = curr_job;
			curr_job = curr_job->next_job;
		}
		
		//insert mid job chain if not at the end of the queue
		if(curr_job != NULL){
			//check to make sure prev_job exists
			//only false curr_job is still the head job
			if(prev_job != NULL)
				prev_job->next_job = job;
			else{
				Q->head = job;
				Q->highestPriority = job->jobPriority;
			}
			
			job->next_job = curr_job;
		}
		//new job becomes new tail
		else{
			Q->tail->next_job = job;
			Q->tail = job;
			//ensures tail doesnt point outside of queue
			Q->tail->next_job = NULL;
		}

		++(Q->size);
	}
}

//job is being removed completely
void DeleteJob(struct Queue* Q, const unsigned int jobID){
	if(Q->size == 1){
		//deallocate space given to PCB
		free(Q->head);
		
		//set head and tail to NULL pointers
		Q->head = NULL;
		Q->tail = NULL;
		
		//reset metadata
		Q->highestPriority = 0;
		Q->size = 0;
	}
	else{
		//pointer for job to clear
		struct PCB* prev_job = NULL;
		struct PCB* clear_job = Q->head;
		
		//find the correct job in the chain
		while(clear_job->jobID != jobID){
			prev_job = clear_job;
			clear_job = clear_job->next_job;
		}
		
		//check if clear_job is head of queue
		if(clear_job == Q->head)
			Q->head = clear_job->next_job;
		//link prev_job to next_job
		else{
			prev_job->next_job = clear_job->next_job;
		}
		
		//temp data for next head of queue and old priority
		unsigned int oldJobPriority = clear_job->jobPriority;
		
		//deallocate space given to PCB
		free(clear_job);
		
		//adjust metadata
		if(oldJobPriority == Q->highestPriority){
			unsigned int newHighest = 0;
			struct PCB* cursor = Q->head;
			while(cursor != NULL){
				if(newHighest < cursor->jobPriority)
					newHighest = cursor->jobPriority;
				cursor = cursor->next_job;
			}
			Q->highestPriority = newHighest;
		}
		
		--(Q->size);
	}
}

//job moved from head of oldQ to tail of newQ
void MoveJob(struct Queue* oldQ, struct Queue* newQ){
	//temp value for oldQ head
	struct PCB* oldQ_new_head = oldQ->head->next_job;
	
	//add job to newQ
	AddJob(newQ, oldQ->head);
	
	//remove and adjust oldQ data
	
	//if oldQ had only one job left, set pointers to NULL
	// and priority/size to 0
	if(oldQ->size == 1){
		oldQ->head = NULL;
		oldQ->tail = NULL;
		oldQ->highestPriority = 0;
		oldQ->size = 0;
	}
	else{
		//temp data for old head priority
		unsigned int oldHeadPriority = oldQ->head->jobPriority;
		
		//move head over one
		oldQ->head = oldQ_new_head;
		
		//adjust metadata
		
		if(oldHeadPriority == oldQ->highestPriority){
			unsigned int newHighest = 0;
			struct PCB* cursor = oldQ->head;
			while(cursor != NULL){
				if(newHighest < cursor->jobPriority)
					newHighest = cursor->jobPriority;
				cursor = cursor->next_job;
			}
			oldQ->highestPriority = newHighest;
		}
		
		--(oldQ->size);
	}
}

//find and move the highest priority job in oldQ to new Q
void MoveHighestPJob(struct Queue* oldQ, struct Queue* newQ){
	//ensure oldQ isnt empty
	if(oldQ->head != NULL){
		//FIND HIGHEST PRIORITY JOB IN OLDQ
		//pointers for highest priority job and one possibly in front of it
		struct PCB* prev_job = NULL;
		struct PCB* moving_job = oldQ->head;
		
		while(moving_job->jobPriority != oldQ->highestPriority){
			prev_job = moving_job;
			moving_job = moving_job->next_job;
		}
		//check for if first job was highest priority job
		if(prev_job != NULL)
			prev_job->next_job = moving_job->next_job;
		else
			oldQ->head = moving_job->next_job;
			
		//ADD MOVING_JOB TO NEWQ
		AddJobPriority(newQ, moving_job);
		
		//CHANGE OLDQ METADATA
		unsigned int newHighest = 0;
		struct PCB* cursor = oldQ->head;
		while(cursor != NULL){
			if(newHighest < cursor->jobPriority)
				newHighest = cursor->jobPriority;
			cursor = cursor->next_job;
		}
		oldQ->highestPriority = newHighest;
	}
}
#endif
