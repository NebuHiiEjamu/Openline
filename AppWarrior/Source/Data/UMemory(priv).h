#pragma once

typedef signed long	tag_word;

typedef struct block_header {
	tag_word tag;
	struct block_header *prev;
	struct block_header *next;
} block_header;

typedef struct list_header {
	block_header * rover;
	block_header header;
} list_header;

typedef struct heap_header {
	struct heap_header* 	prev;
	struct heap_header*		next;
} heap_header;

typedef unsigned long mem_size;

typedef void * (*sys_alloc_ptr)(mem_size size);
typedef void (*sys_free_ptr)(void *ptr);

typedef struct pool_options{
	sys_alloc_ptr	sys_alloc_func;
	sys_free_ptr	sys_free_func;
	mem_size		min_heap_size;
	int				always_search_first;
} pool_options;

typedef struct mem_pool_obj {
	list_header		free_list;
	pool_options	options;
	heap_header*	heap_list;
	void*			userData;
} mem_pool_obj;


void _MEInitPool(mem_pool_obj *pool_obj);
int _MEPoolPreallocate(mem_pool_obj *pool_obj, mem_size size);
void _MEPoolPreassign(mem_pool_obj *pool_obj, void *ptr, mem_size size);
void *_MEPoolAlloc(mem_pool_obj *pool_obj, mem_size size);
void *_MEPoolAllocClear(mem_pool_obj *pool_obj, mem_size size);
void *_MEPoolRealloc(mem_pool_obj *pool_obj, void *ptr, mem_size size);
void _MEPoolFree(mem_pool_obj *pool_obj, void *ptr);
int _MEPoolValid(mem_pool_obj * pool_obj, void * ptr);

void *_MESysAlloc(mem_size size);
void _MESysFree(void *ptr);

