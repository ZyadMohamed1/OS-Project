#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>
#include <inc/queue.h>
//2022: NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
#define HEAP_COPY_SIZE (KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE
#define DUMMY_VALUE 999999999
#define FLAG_INIT_VALUE -1


struct frames{

	uint32 *virAddr;
	int saveCounter;

}heapCopyArray[HEAP_COPY_SIZE];

struct heap
{
	uint32  VA;
	uint32 size;
	uint32 f;
}
heap[(KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE];

int lastArrayIndex = 0;
uint32 current = KERNEL_HEAP_START;
uint32 kheapSize = (KERNEL_HEAP_MAX - KERNEL_HEAP_START);

int checkFrame(uint32* virtual_address) {
    uint32 *page_table = NULL;
    get_page_table(ptr_page_directory, virtual_address, &page_table);
    int bit = (page_table[PTX(virtual_address)]) & PERM_PRESENT;
    return bit;
}

void* framesAllocation(int address , int allocationSize){
	struct Frame_Info *frameInfoPointer;
	heapCopyArray[lastArrayIndex].virAddr = (uint32*) address;
	heapCopyArray[lastArrayIndex].saveCounter = ROUNDUP(allocationSize, PAGE_SIZE);
	lastArrayIndex++;

	for(int i = address; i < (address + allocationSize) ; i += PAGE_SIZE)
	{
		frameInfoPointer = NULL;
		allocate_frame(&frameInfoPointer);
		map_frame(ptr_page_directory, frameInfoPointer,(void*)i ,PERM_PRESENT|PERM_WRITEABLE);
	}
	return (void *) address;
}

void* kmalloc(unsigned int size)
{

	//TODO: [PROJECT 2022 - [1] Kernel Heap] kmalloc()
	//TODO: [PROJECT 2022 - BONUS1] Implement a Kernel allocation strategy

	if(isKHeapPlacementStrategyNEXTFIT())
	{


		if(current==KERNEL_HEAP_MAX)
		{
			current=KERNEL_HEAP_START+(2*PAGE_SIZE);
		}

		size = ROUNDUP(size,PAGE_SIZE);
		int no_of_page=size/PAGE_SIZE;
		int increment_flag=0;
		uint32 The_NUN=(KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE;
		int pages=no_of_page;
		int found_frames=0;
		uint32 loop_add=current;
		int final_add;
		int no_of_used_pages=0;

		if(kheapSize < size)
		{
			return NULL;
		}
		int loop_back_flag=0;
		for(int i=(loop_add-KERNEL_HEAP_START)/PAGE_SIZE;i<=The_NUN;i++)
		{
			if((loop_add+PAGE_SIZE) >= KERNEL_HEAP_MAX)
			{
				loop_back_flag=1;
			}
			int bit =checkFrame((void*)loop_add);

			if(no_of_page==0)
			{
				found_frames=1;
				heap[(current-KERNEL_HEAP_START)/PAGE_SIZE].size=size;
				final_add=loop_add;
				The_NUN=(loop_add-KERNEL_HEAP_START)/PAGE_SIZE;
			}
			if(bit==0)
			{
				no_of_page--;
				increment_flag=0;
			}
			if(no_of_page!=0 && bit==1 )
			{
				no_of_used_pages=(heap[(loop_add-KERNEL_HEAP_START)/PAGE_SIZE].size)/PAGE_SIZE;
				loop_add=loop_add+(no_of_used_pages*PAGE_SIZE);
				i=((loop_add)-KERNEL_HEAP_START)/PAGE_SIZE;
				no_of_page=pages;
				current=loop_add;
				increment_flag=1;
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
		if(loop_back_flag==1)
		{
			loop_add=KERNEL_HEAP_START+(2*PAGE_SIZE);
			for(int i=(loop_add-KERNEL_HEAP_START)/PAGE_SIZE;i<=The_NUN;i++)
				{
					int bit =checkFrame((void*)loop_add);
					if(no_of_page==0)
					{
						found_frames=1;
						heap[(current-KERNEL_HEAP_START)/PAGE_SIZE].size=size;
						final_add=loop_add;
						The_NUN=(loop_add-KERNEL_HEAP_START)/PAGE_SIZE;
					}
					if(bit==0)
					{
						no_of_page--;
						increment_flag=0;
					}
					if(no_of_page!=0 && bit==1 )
					{
						no_of_used_pages=(heap[(loop_add-KERNEL_HEAP_START)/PAGE_SIZE].size)/PAGE_SIZE;
						loop_add=loop_add+(no_of_used_pages*PAGE_SIZE);
						i=((loop_add)-KERNEL_HEAP_START)/PAGE_SIZE;
						no_of_page=pages;
						current=loop_add;
						increment_flag=1;
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
			loop_back_flag=0;
		}
		if(found_frames==0)
		{
			return NULL;
		}
		int End=(loop_add-KERNEL_HEAP_START)/PAGE_SIZE;
		no_of_page=pages;
		int return_add=current;

		for(int i=0;i<End;i++)
		{
			if(pages==0)
			{
				kheapSize=kheapSize-size;
				return (void*) return_add;
			}
			 struct Frame_Info *ptr_frame_info = NULL;
			 int value = allocate_frame(&ptr_frame_info);

			 if(value == E_NO_MEM)
			 {
				 return NULL;
			 }
			 else
			 {
				 map_frame(ptr_page_directory, ptr_frame_info, (void*) current, PERM_WRITEABLE);
				 heap[(current-KERNEL_HEAP_START)/PAGE_SIZE].f=1;
				 heap[(current-KERNEL_HEAP_START)/PAGE_SIZE].size=size;
				 heap[(current-KERNEL_HEAP_START)/PAGE_SIZE].VA=current;
				 pages--;
			 }
			 current = current + (4*1024);
		}
		return NULL;
	}
	else if(isKHeapPlacementStrategyBESTFIT())
	{
		uint32 *pagrTablePointer;
		struct Frame_Info *frameInfoPointer;


		int minimumSize = DUMMY_VALUE;

		int freeFrames = 0;
		int freeFramesCounter = 0;

		int allocationStartFlag = FLAG_INIT_VALUE;
		int allocationEndFlag = FLAG_INIT_VALUE;

		int whileCounter = KERNEL_HEAP_START;

		while(whileCounter < KERNEL_HEAP_MAX){
			pagrTablePointer = NULL;
			frameInfoPointer = get_frame_info(ptr_page_directory, (void*)whileCounter, &pagrTablePointer);
			if(frameInfoPointer != NULL){
				if(freeFrames >= size && freeFrames < minimumSize){
					allocationEndFlag = whileCounter;
					allocationStartFlag = allocationEndFlag - (freeFramesCounter*PAGE_SIZE);
					minimumSize = freeFrames;
				 }
				freeFrames = 0;
				freeFramesCounter = 0;
			}
			else{
				freeFrames += PAGE_SIZE;
				freeFramesCounter++;
			}
			whileCounter = whileCounter + PAGE_SIZE;
		}

		if(freeFrames >= size && freeFrames < minimumSize){
			minimumSize = freeFrames;
			allocationEndFlag = whileCounter;
			allocationStartFlag = whileCounter - (freeFramesCounter*PAGE_SIZE);
		}
		if(allocationStartFlag!=-1)
			return framesAllocation(allocationStartFlag,size);
		return NULL;
	}

	return NULL;
}

void kfree(void* virtual_address)
{

	//TODO: [PROJECT 2022 - [2] Kernel Heap] kfree()

	if(isKHeapPlacementStrategyNEXTFIT())
	{

		uint32 st_add = (uint32) virtual_address;
		int size= heap[(st_add-KERNEL_HEAP_START)/PAGE_SIZE].size;
		uint32 begin=heap[(st_add-KERNEL_HEAP_START)/PAGE_SIZE].VA;
		int end=size/PAGE_SIZE;
		uint32 *page_table = NULL;
		 get_page_table(ptr_page_directory, virtual_address, &page_table);
		for(int i=0;i<end;i++)
		{
			unmap_frame(ptr_page_directory,(void*)st_add);
			uint32 entry= page_table[PTX(st_add)];
			entry=entry& 0x00000000;
			heap[(st_add-KERNEL_HEAP_START)/PAGE_SIZE].VA=0;
			heap[(st_add-KERNEL_HEAP_START)/PAGE_SIZE].size=0;
			heap[(st_add-KERNEL_HEAP_START)/PAGE_SIZE].f=0;

			st_add+=PAGE_SIZE;
		}
		kheapSize=kheapSize+size;

	}
	else if(isKHeapPlacementStrategyBESTFIT())
	{
		int pages_count = -1; //allocated pages by this address
		int required_index;
		for(int i = 0; i < lastArrayIndex; i++)
		{
			if(virtual_address == heapCopyArray[i].virAddr)
			{
				pages_count = heapCopyArray[i].saveCounter;
				required_index = i;
				break;
			}
		}

		if(pages_count==-1)
			return;

		for(int i = 0; i < (pages_count/PAGE_SIZE) ; i++)
		{
			unmap_frame(ptr_page_directory, (void *)virtual_address);
			virtual_address += PAGE_SIZE;
		}

		heapCopyArray[required_index].virAddr = (uint32*)-1;
		heapCopyArray[required_index].saveCounter = -1;
	}
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT 2022 - [3] Kernel Heap] kheap_virtual_address()
	// Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");
	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details
	//change this "return" according to your answer


	for(uint32 i = KERNEL_HEAP_START ; i< KERNEL_HEAP_MAX ; i+=PAGE_SIZE)
	{

		uint32* ptr_page_table = NULL;
		get_page_table(ptr_page_directory, (void*)i, &ptr_page_table);
		uint32 physicalAddress = (ptr_page_table[PTX(i)]>>12) * PAGE_SIZE;

		if(physicalAddress == physical_address)
		{
			return i;
		}
	}

	return 0;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT 2022 - [4] Kernel Heap] kheap_physical_address()
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");

	//return the physical address corresponding to given virtual_address
	//refer to the project presentation and documentation for details

	uint32* ptr_page_table = NULL;
	get_page_table(ptr_page_directory, (void*)virtual_address, &ptr_page_table);
	return (ptr_page_table[PTX(virtual_address)]>>12) * PAGE_SIZE;

	//change this "return" according to your answer
	return 0;
}
