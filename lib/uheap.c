#include <inc/lib.h>

struct heap

{
	uint32 VA;
	uint32 size;
	uint32 f;
} heap[(USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE];

uint32 current = USER_HEAP_START;
uint32 kheapSize = (USER_HEAP_MAX - USER_HEAP_START);
int found_frames = 0;

int next_fit(uint32 loop_add, uint32 end_add, int no_of_page) {
	// Declaration..
	int loop_back_flag = 0;
	int increment_flag = 0;
	int final_add;
	int no_of_used_pages = 0;
	int pages = no_of_page;
	//cprintf("size....................... %d \n",no_of_page);
	int size = no_of_page * PAGE_SIZE;
	uint32 The_NUN = (end_add - USER_HEAP_START) / PAGE_SIZE;

	//Next Fit..
	for (int i = (loop_add - USER_HEAP_START) / PAGE_SIZE; i <= The_NUN; i++) {

		if (no_of_page == 0) {

			//cprintf("Done222....!\n");
			found_frames = 1;
			final_add = loop_add;
			current=loop_add-size;
			//The_NUN = (loop_add - USER_HEAP_START) / PAGE_SIZE;
			//cprintf("%x\n",final_add);
			return final_add;
		}
		if (heap[(loop_add - USER_HEAP_START) / PAGE_SIZE].f == 0) {
			no_of_page--;
			increment_flag = 0;
		}
		if (heap[(loop_add - USER_HEAP_START) / PAGE_SIZE].f == 1){

			no_of_used_pages =
					(heap[(loop_add - USER_HEAP_START) / PAGE_SIZE].size)
							/ PAGE_SIZE;
			//cprintf(" loop_add %x \n",loop_add);
			//cprintf(" here %d \n",no_of_used_pages);
			loop_add = loop_add + (heap[(loop_add - USER_HEAP_START) / PAGE_SIZE].size);
			//i = ((loop_add) - USER_HEAP_START) / PAGE_SIZE;
			no_of_page = pages;
			//current = loop_add;
			increment_flag = 1;
		}
		if ((loop_add) == USER_HEAP_MAX) {
			//cprintf("LoooooooP Back.......................\n");
			loop_back_flag = 1;
			break;
		}

		if(increment_flag==1)
				{
					continue;
				}
				else
				{
					loop_add = loop_add + (4*1024);
				}

	}
	if (loop_back_flag == 1) {
		loop_add=USER_HEAP_START;
		The_NUN=(current - USER_HEAP_START) / PAGE_SIZE;
		for (int i = (loop_add - USER_HEAP_START) / PAGE_SIZE; i <= The_NUN; i++) {

				if (no_of_page == 0) {

					//cprintf("Done222....!\n");
					found_frames = 1;
					final_add = loop_add;
					current=loop_add-size;
					//The_NUN = (loop_add - USER_HEAP_START) / PAGE_SIZE;
					//cprintf("%x\n",final_add);
					return final_add;
				}
				if (heap[(loop_add - USER_HEAP_START) / PAGE_SIZE].f == 0) {
					no_of_page--;
					increment_flag = 0;
				}
				if (heap[(loop_add - USER_HEAP_START) / PAGE_SIZE].f == 1){

					no_of_used_pages =
							(heap[(loop_add - USER_HEAP_START) / PAGE_SIZE].size)
									/ PAGE_SIZE;
					//cprintf(" loop_add %x \n",loop_add);
					//cprintf(" here %d \n",no_of_used_pages);
					loop_add = loop_add + (heap[(loop_add - USER_HEAP_START) / PAGE_SIZE].size);
					//i = ((loop_add) - USER_HEAP_START) / PAGE_SIZE;
					no_of_page = pages;
					//current = loop_add;
					increment_flag = 1;
				}
				if ((loop_add) == USER_HEAP_MAX) {
					//cprintf("LoooooooP Back.......................\n");
					loop_back_flag = 1;
					break;
				}

				if(increment_flag==1)
						{
							continue;
						}
						else
						{
							loop_add = loop_add + (4*1024);
						}

			}
	}
	if (found_frames == 0) {
		return 0;
	}
	return 0;
}

int allocation(int loop_add, int no_of_page) {

	int End = (loop_add - USER_HEAP_START) / PAGE_SIZE;

	int return_add = current;
	int size = no_of_page * PAGE_SIZE;

	for (int i = ((current) - USER_HEAP_START) / PAGE_SIZE; i < End; i++) {

		heap[(current - USER_HEAP_START) / PAGE_SIZE].f = 1;
		heap[(current - USER_HEAP_START) / PAGE_SIZE].size = size;
		heap[(current - USER_HEAP_START) / PAGE_SIZE].VA = return_add;

		current = current + (4 * 1024);
	}
	kheapSize = kheapSize - size;
	//cprintf("%x from allocation\n",return_add);
	sys_allocateMem(return_add,size);
	return return_add;
}

void* malloc(uint32 size) {

	//TODO: [PROJECT 2022 - [9] User Heap malloc()] [User Side]

	if (current == USER_HEAP_MAX) {
		current = USER_HEAP_START;
	}

	size = ROUNDUP(size, PAGE_SIZE);
	int no_of_page = size / PAGE_SIZE;

	if (kheapSize < size) {
		//cprintf("boooooo.. \n");
		return NULL;
	}

	int loop_add = next_fit(current, USER_HEAP_MAX, no_of_page);

	//if there are no frames found return 0
	if (loop_add == 0) {
		return NULL;
	}
	int return_add = allocation(loop_add, no_of_page);

	// check if there enough space in the mem or not if not return null
	if (return_add == 0) {
		return NULL;
	} else {
		return (void*) return_add;
	}

	return NULL;
}

void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable) {
	panic("smalloc() is not required ..!!");
	return NULL;
}

void* sget(int32 ownerEnvID, char *sharedVarName) {
	panic("sget() is not required ..!!");
	return 0;
}

// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from page file and main memory then switch back to the user again.
//
//	We can use sys_freeMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls freeMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the freeMem function is empty, make sure to implement it.

void free(void* virtual_address) {
	//TODO: [PROJECT 2022 - [11] User Heap free()] [User Side]
	// Write your code here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");

	//you should get the size of the given allocation using its address
	//you need to call sys_freeMem()
	//refer to the project presentation and documentation for details

	uint32 st_add = (uint32) virtual_address;
	int size = heap[(st_add - USER_HEAP_START) / PAGE_SIZE].size;

	uint32 begin = heap[(st_add - USER_HEAP_START) / PAGE_SIZE].VA;
	int end = size / PAGE_SIZE;


	sys_freeMem(begin, size);
	for (int i = 0; i < end; i++)
	{
		heap[(begin - USER_HEAP_START) / PAGE_SIZE].VA = 0;
		heap[(begin - USER_HEAP_START) / PAGE_SIZE].size = 0;
		heap[(begin - USER_HEAP_START) / PAGE_SIZE].f = 0;

		begin += PAGE_SIZE;
	}
	kheapSize = kheapSize + size;

}


void sfree(void* virtual_address)
{
	panic("sfree() is not requried ..!!");
}


//===============
// [2] realloc():
//===============

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_moveMem(uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		which switches to the kernel mode, calls moveMem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		in "memory_manager.c", then switch back to the user mode here
//	the moveMem function is empty, make sure to implement it.
uint32 alloc_next=USER_HEAP_START;//pointer to virtual address to allocate in ...
uint32 index=0,max=(USER_HEAP_MAX-USER_HEAP_START)/PAGE_SIZE,last_index=0;
uint32 uheap[(USER_HEAP_MAX-USER_HEAP_START)/PAGE_SIZE];//indexing by number of pages in user heap // hold size


uint32 delete_uheap(uint32 va)
{
	index=(va-USER_HEAP_START)/PAGE_SIZE;
	uint32 size=uheap[index];
	uint32 end=((va+size)-USER_HEAP_START)/PAGE_SIZE;
	//-----------------------------------
	for(;index<end;index++)
	{
		uheap[index]=0;
	}
	return size;
}



void *realloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT 2022 - BONUS3] User Heap Realloc [User Side]
	// Write your code here, remove the panic and write your code
	//panic("realloc() is not implemented yet...!!");


	return NULL;
}
