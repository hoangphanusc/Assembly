/*
 * mm.c
 * 
 * Description: An implementation of malloc, free, and realloc using explicit free list
 * Skeleton code taken from "Computer Systems - A Programmer s Perspective"  (3rd Edition)
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

#define CHUNKSIZE (1<<12) 
#define WSIZE 4 // == sizeof(void *)
#define DSIZE 8

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define PADDING_SIZE_ALLOC (DSIZE)
#define PADDING_SIZE_FREE (2 * DSIZE)

#define PACK(size, alloc) ((size) | (alloc))

#define GET(p) (*(unsigned int *) (p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

#define HEADER(bp) ((char*)(bp) - WSIZE)
#define FOOTER(bp) ((char*)(bp) + GET_SIZE(HEADER(bp)) - DSIZE)

//Put next and prev ptrs before footer instead of after header
#define NEXT_FREE_PTR(bp) ((char*)FOOTER(bp) - DSIZE)
#define PREV_FREE_PTR(bp) ((char*)FOOTER(bp) - WSIZE)

#define NEXT_BP(bp) ((char *)(bp) + GET_SIZE((char *)(bp) - WSIZE))
#define PREV_BP(bp) ((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE))

static char* heap_listp = NULL;
static char* firstFree = NULL;

static void* extend_heap(size_t words);
static void* coalesce(char* bp);
static void* find_fit(size_t size);
static void* place(void* bp, size_t size);
// static void printHeap();
// static void printFreeList();
// static void addressOf(char* ptr);

// const int toPrint = 1000000;
// const int debug = 1;
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
	/* Create the initial empty heap */
	if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)
		return -1;
	PUT(heap_listp, 0);
	PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));
	PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));
	PUT(heap_listp + (3 * WSIZE), PACK(0, 1));
	heap_listp += (2 * WSIZE);
	/* Extend the empty heap with a free block of CHUNKSIZE bytes */
	char* bp = (char*) extend_heap(CHUNKSIZE/WSIZE);
	if (bp == NULL)
	{
		return -1;
	}
	return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *	 Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
	int extendSize;
	int alignedSize;
	char *bp;

	if(size == 0) return NULL;

	alignedSize = ALIGN(size + DSIZE);

	if((bp = find_fit(alignedSize)) != NULL){
		place(bp, alignedSize);
	}
	else {
		extendSize = (alignedSize > (int) CHUNKSIZE) ? alignedSize : (int) CHUNKSIZE;
		if ((bp = extend_heap(extendSize/WSIZE)) == NULL){
			return NULL;
		}
		place(bp, alignedSize);
	}
	return bp;
}

/*
 * mm_free - Free a block if previously alloc/realloc'd and has not yet been freed.
 */
void mm_free(void *ptr)
{
	if (ptr == NULL) return;
	size_t size = GET_SIZE(HEADER(ptr));
	if (GET_ALLOC(HEADER(ptr))){
		PUT(HEADER(ptr), PACK(size, 0));
		PUT(FOOTER(ptr), PACK(size, 0));
		coalesce(ptr);
	}
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *oldptr, size_t newSize)
{
	size_t oldSize = GET_SIZE(HEADER(oldptr));
	newSize = ALIGN(newSize + DSIZE);

	if (oldptr == NULL){
		return mm_malloc(newSize);
	}
	if (!GET_ALLOC(HEADER(oldptr))){
		return NULL;
	}
	if (newSize == 0){
		free(oldptr);
		return;
	}
	if (newSize < oldSize && (int) oldSize - (int) newSize < 2*DSIZE)
	{
		return oldptr;
	}

	if ((int) oldSize - (int) newSize >= 2*DSIZE){
		PUT(HEADER(oldptr), PACK(newSize, 1));
		PUT(FOOTER(oldptr), PACK(newSize, 1));
		char* nextBP = NEXT_BP(oldptr);
		PUT(HEADER(nextBP), PACK(oldSize - newSize, 0));
		PUT(FOOTER(nextBP), PACK(oldSize - newSize, 0));
		coalesce(nextBP);
		return oldptr;
	}

	int nextSize = GET_SIZE(HEADER(NEXT_BP(oldptr)));
	if(!GET_ALLOC(HEADER(NEXT_BP(oldptr))) && ((int) oldSize + nextSize - (int) newSize >= 2*DSIZE)){
		char* nextBP = NEXT_BP(oldptr);
		char* next = *(char**)NEXT_FREE_PTR(nextBP);
		char* prev = *(char**)PREV_FREE_PTR(nextBP);
		PUT(HEADER(oldptr), PACK(newSize, 1));
		PUT(FOOTER(oldptr), PACK(newSize, 1));
		nextBP = NEXT_BP(oldptr);
		PUT(HEADER(nextBP), PACK(oldSize + nextSize - newSize, 0));
		PUT(FOOTER(nextBP), PACK(oldSize + nextSize - newSize, 0));
		
		if (next) PUT(PREV_FREE_PTR(next), nextBP);
		if (prev) PUT(NEXT_FREE_PTR(prev), nextBP);
		return oldptr;
	}

	void *newptr;
	size_t copySize;
	
	newptr = mm_malloc(newSize);
	if (newptr == NULL)
	  return NULL;
	copySize = oldSize;
	if (newSize < copySize)
	  copySize = newSize;
	memcpy(newptr, oldptr, copySize);
	mm_free(oldptr);
	return newptr;
}

static void* coalesce(char* bp){
	int prev_alloc = GET_ALLOC(HEADER(PREV_BP(bp)));
	int next_alloc = GET_ALLOC(HEADER(NEXT_BP(bp)));
	size_t size = GET_SIZE(HEADER(bp));
	
	if (prev_alloc && next_alloc){
		//Update free list
		if (firstFree == NULL){
			firstFree = bp;
			PUT(PREV_FREE_PTR(firstFree), NULL);
			PUT(NEXT_FREE_PTR(firstFree), NULL);
		}
		else if (firstFree != bp) {
			PUT(PREV_FREE_PTR(firstFree), bp);
			PUT(NEXT_FREE_PTR(bp), firstFree);
			firstFree = bp;
		} else {
			PUT(NEXT_FREE_PTR(bp), NULL);
			PUT(PREV_FREE_PTR(bp), NULL);
		}
	}
	else if (prev_alloc && !next_alloc){
		//Update free list
		char* nextBP = NEXT_BP(bp);
		if (firstFree != nextBP){
			char* next = *(char**)NEXT_FREE_PTR(nextBP);
			char* prev = *(char**)PREV_FREE_PTR(nextBP);
			if (next) PUT(PREV_FREE_PTR(next), prev);
			if (prev) PUT(NEXT_FREE_PTR(prev), next);
			size += GET_SIZE(HEADER(NEXT_BP(bp)));
			PUT(HEADER(bp), PACK(size, 0));
			PUT(FOOTER(bp), PACK(size, 0));

			PUT(PREV_FREE_PTR(firstFree), bp);
			PUT(NEXT_FREE_PTR(bp), firstFree);
			firstFree = bp;
		}
		else {
			char* next = *(char**)NEXT_FREE_PTR(nextBP);
			if (next) PUT(PREV_FREE_PTR(next), bp);

			size += GET_SIZE(HEADER(NEXT_BP(bp)));
			PUT(HEADER(bp), PACK(size, 0));
			PUT(FOOTER(bp), PACK(size, 0));

			firstFree = bp;
		}
	}
	else if (!prev_alloc && next_alloc){
		char* prevBP = PREV_BP(bp);
		if (firstFree != prevBP){
			//Update free list
			char* next = *(char**)NEXT_FREE_PTR(prevBP);
			char* prev = *(char**)PREV_FREE_PTR(prevBP);
			if (next) PUT(PREV_FREE_PTR(next), prev);
			if (prev) PUT(NEXT_FREE_PTR(prev), next);
			PUT(PREV_FREE_PTR(firstFree), prevBP);
			PUT(NEXT_FREE_PTR(bp), firstFree);
			firstFree = prevBP;

			//Coalesce
			size += GET_SIZE(HEADER(PREV_BP(bp)));
			PUT(FOOTER(bp), PACK(size, 0));
			PUT(HEADER(prevBP), PACK(size, 0));
			bp = prevBP;
		} else {
			char* next = *(char**)NEXT_FREE_PTR(prevBP);

			size += GET_SIZE(HEADER(PREV_BP(bp)));
			PUT(FOOTER(bp), PACK(size, 0));
			PUT(HEADER(prevBP), PACK(size, 0));
			bp = prevBP;

			PUT(NEXT_FREE_PTR(bp), next);
		}
	}
	else {
		char* nextBP = NEXT_BP(bp);
		char* prevBP = PREV_BP(bp);
		if (nextBP == *(char**)NEXT_FREE_PTR(prevBP) || nextBP == *(char**)PREV_FREE_PTR(prevBP)){
			if (firstFree != nextBP && firstFree != prevBP){ //f - a - f
				if (nextBP == *(char**)NEXT_FREE_PTR(prevBP)){ //f > f
					char* next = *(char**)NEXT_FREE_PTR(nextBP);
					char* prev = *(char**)PREV_FREE_PTR(prevBP);
					if (next) PUT(PREV_FREE_PTR(next), prev);
					if (prev) PUT(NEXT_FREE_PTR(prev), next);
				}
				else { // f < f
					char* next = *(char**)NEXT_FREE_PTR(prevBP);
					char* prev = *(char**)PREV_FREE_PTR(nextBP);
					if (next) PUT(PREV_FREE_PTR(next), prev);
					if (prev) PUT(NEXT_FREE_PTR(prev), next);
				}

				PUT(PREV_FREE_PTR(firstFree), prevBP);
				PUT(NEXT_FREE_PTR(nextBP), firstFree);
				PUT(PREV_FREE_PTR(nextBP), NULL);
				firstFree = prevBP;
			} else if (firstFree == nextBP){ //f - a - 1
				PUT(NEXT_FREE_PTR(nextBP), *(char**)NEXT_FREE_PTR(prevBP));
				firstFree = prevBP;
			} else { //1 - a - f
				char* next = *(char**)NEXT_FREE_PTR(nextBP);
				if (next) PUT(PREV_FREE_PTR(next), prevBP);
			}
		}
		else{
			if (firstFree != nextBP && firstFree != prevBP){
				//Update free list
				char* next = *(char**)NEXT_FREE_PTR(nextBP);
				char* prev = *(char**)PREV_FREE_PTR(nextBP);
				if (next) PUT(PREV_FREE_PTR(next), prev);
				if (prev) PUT(NEXT_FREE_PTR(prev), next);

				next = *(char**)NEXT_FREE_PTR(prevBP);
				prev = *(char**)PREV_FREE_PTR(prevBP);
				if (next) PUT(PREV_FREE_PTR(next), prev);
				if (prev) PUT(NEXT_FREE_PTR(prev), next);

				PUT(PREV_FREE_PTR(firstFree), prevBP);
				PUT(NEXT_FREE_PTR(nextBP), firstFree);
				firstFree = prevBP;
			} else if (firstFree == nextBP) {
				char* next = *(char**)NEXT_FREE_PTR(prevBP);
				char* prev = *(char**)PREV_FREE_PTR(prevBP);
				if (next) PUT(PREV_FREE_PTR(next), prev);
				if (prev) PUT(NEXT_FREE_PTR(prev), next);
				char* inter = *(char**) NEXT_FREE_PTR(nextBP);
				PUT(PREV_FREE_PTR(inter), prevBP);
				firstFree = prevBP;
			} else { //prev is firstfree
				char* next = *(char**)NEXT_FREE_PTR(nextBP);
				char* prev = *(char**)PREV_FREE_PTR(nextBP);
				if (next) PUT(PREV_FREE_PTR(next), prev);
				if (prev) PUT(NEXT_FREE_PTR(prev), next);
				PUT(NEXT_FREE_PTR(nextBP), *(char**)NEXT_FREE_PTR(prevBP));
				PUT(PREV_FREE_PTR(*(char**)NEXT_FREE_PTR(nextBP)), prevBP);
			}
		}

		//Coalesce
		size += GET_SIZE(HEADER(PREV_BP(bp))) + GET_SIZE(HEADER(NEXT_BP(bp)));
		PUT(HEADER(PREV_BP(bp)), PACK(size, 0));
		PUT(FOOTER(NEXT_BP(bp)), PACK(size, 0));
		bp = PREV_BP(bp);
	}
	PUT(PREV_FREE_PTR(firstFree), NULL);
	return bp;
}

static void* extend_heap(size_t words){	
	char *bp;
	size_t size;
	/* Allocate an even number of words to maintain alignment */
	size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
	if ((long)(bp = mem_sbrk(size)) == -1)
		return NULL;

	/* Initialize free block header/footer and the epilogue header */
	PUT(HEADER(bp), PACK(size, 0)); /*Free block header*/
	PUT(FOOTER(bp), PACK(size, 0)); /*Free block footer*/
	PUT(HEADER(NEXT_BP(bp)), PACK(0, 1)); /*New epilogue header*/
	/* Coalesce if the previous block was free */
	bp = coalesce(bp);
	return bp;
}

static void* find_fit(size_t size){
	void *bp = NULL;
	int block = 0;
	for (bp = firstFree; bp != NULL; bp = *(char**)NEXT_FREE_PTR(bp)){
		++block;
		if (!GET_ALLOC(HEADER(bp)) && size <= GET_SIZE(HEADER(bp))){
			return bp;
		}
		if (block == 100) break;
	}
	return NULL;
}

static void* place(void* bp, size_t size){
	size_t freeSize = GET_SIZE(HEADER(bp));
	if ((freeSize - size) >= 2*DSIZE){
		char* next = *(char**)NEXT_FREE_PTR(bp);
		char* prev = *(char**)PREV_FREE_PTR(bp);
		PUT(HEADER(bp), PACK(size, 1));
		PUT(FOOTER(bp), PACK(size, 1));
		char* nextBP = NEXT_BP(bp);
		if (bp == firstFree){
			firstFree = nextBP;
		}
		if (next != NULL) PUT(PREV_FREE_PTR(next), nextBP);
		if (prev != NULL) PUT(NEXT_FREE_PTR(prev), nextBP);
		bp = nextBP;
		PUT(HEADER(bp), PACK((freeSize - size), 0));
		PUT(FOOTER(bp), PACK((freeSize - size), 0));
	}
	else {
		if (bp == firstFree){
			firstFree = *(char**)NEXT_FREE_PTR(bp);
		}
		char* next = *(char**)NEXT_FREE_PTR(bp);
		char* prev = *(char**)PREV_FREE_PTR(bp);
		PUT(HEADER(bp), PACK(freeSize, 1));
		PUT(FOOTER(bp), PACK(freeSize, 1));
		if (next != NULL) PUT(PREV_FREE_PTR(next), prev);
		if (prev != NULL) PUT(NEXT_FREE_PTR(prev), next);
	}
}

// static void printHeap(){
// 	if (!debug) return;
// 	int i = 0;
// 	int free = 0;
// 	char* bp;
// 	printf("Current heap:\n");
// 	for (bp = heap_listp; GET_SIZE(HEADER(bp)) > 0 && i < toPrint; bp = NEXT_BP(bp)){
// 		++i;
// 		if (!GET_ALLOC(HEADER(bp)) && i < toPrint){
// 			++free;
// 			printf("Address %d (%x): Block %d with size %d: ", bp - heap_listp, bp, i, GET_SIZE(HEADER(bp)));
// 			unsigned int next = (*(char**)NEXT_FREE_PTR(bp) == NULL) ? NULL : *(char**) NEXT_FREE_PTR(bp) - heap_listp;
// 			unsigned int prev = (*(char**)PREV_FREE_PTR(bp) == NULL) ? NULL : *(char**) PREV_FREE_PTR(bp) - heap_listp;
// 			printf ("FREE, next = %d, prev = %d\n", next, prev);
// 		}
// 		else {
// 			printf("Address %d (%x): Block %d with size %d: ", bp - heap_listp, bp, i, GET_SIZE(HEADER(bp)));
// 			printf ("ALLOC'D\n");
// 		}
// 		// for (int j = 0; j < GET_SIZE(HEADER(bp)); j+= 4){
// 		// 	printf("%d ", *(unsigned int*) (bp - 4 + j));
// 		// }
// 		// printf("\n");
// 	}
// 	printf("Address %d: End of heap == %d\n", bp + GET_SIZE(HEADER(bp)) - heap_listp, (char*) mem_heap_hi() - heap_listp);
// 	printf("Total Free: %d\n", free);
// 	printf("\n");
// 	//char a[1]; scanf("%s",a);
// }

// static void printFreeList(){
// 	if (!debug) return;
// 	printf("Free blocks addresses: \n");
// 	int block = 0;
// 	int free = 0;
// 	for (char* bp = firstFree; bp != NULL; bp = *(char**)NEXT_FREE_PTR(bp)){
// 		++free;
// 		printf("%d > \n", (char*) bp - heap_listp);
// 		++block;
// 		if (block == 20) break;
// 	}
// 	printf("Total Free: %d\n", free);
// 	printf("\n");
// }

// static void addressOf(char* ptr){
// 	if (!debug) return;
// 	if (ptr == NULL) {
// 		printf("NULL\n");
// 	}
// 	else {
// 		printf("%d\n", ptr - heap_listp);
// 	}
// }