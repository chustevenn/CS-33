/*
 * 905094800_mm.c - Malloc package implemented through an explicit doubly linked free list.
 * 
 * In this approach, a block is allocated by updating its 
 * header and removing it from the linked free list. 
 * Each block contains a word-long header and footer 
 * containing boundary tags with information on the block's
 * size and allocation status.
 *
 * This approach also takes advantage of the payload storage
 * in free blocks to store two pointers: one to the next
 * free block, and one to the previous free block. Any newly 
 * freed blocks have their pointers adjusted such that they
 * are inserted into the free list as the first item.
 *
 * Any newly created free blocks are immediately coalesced 
 * using the information from the boundary tags. Realloc is
 * implemented using mm_malloc and mm_free, taking into account
 * several different situations.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "905094800",
    /* First member's full name */
    "Steven Chu",
    /* First member's email address */
    "schu92620@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* CONSTANTS AND MACROS */
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)

#define MAX(x, y) ((x) > (y) ? (x) : (y))

// Combine block size and allocated bit into single word
#define PACK(size, alloc) ((size) | (alloc))

// Read and write words at a given address p
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *) (p) = (val))

// Read block size and allocated bit from given address p
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

// Determine address of header and footer of a block, given block pointer bp
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

// Determine address of next and previous blocks, given block pointer bp
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

// Determine address of next and previous free blocks
#define GET_NEXT_FBLK(bp) (*(char **)(bp + WSIZE))
#define GET_PREV_FBLK(bp) (*(char **)(bp))

// Set address of next and previous free blocks
#define SET_NEXT_FBLK(bp, np) (GET_NEXT_FBLK(bp) = np)
#define SET_PREV_FBLK(bp, np) (GET_PREV_FBLK(bp) = np)

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* GLOBAL VARIABLES */
static char *list_begin;
static char *heap_listp;

/* HELPER FUNCTIONS */
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static int mm_check(void);
static void place(void *bp, size_t size);
static void *find_fit(size_t asize);
static void return_to_list(void *bp);
static void remove_from_list(void *bp);

/* 
 * mm_init - initialize the malloc package. Initializes prologue and epilogue blocks, 
 * as well as an initial double word aligned minimum size free block. Initializes beginning of free list.
 */
int mm_init(void)
{	
    if((heap_listp = mem_sbrk(8*WSIZE)) == (void *)-1)
      return -1;
    PUT(heap_listp, 0);				  //Padding for alignment
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));  //Prologue header
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));  //Prologue footer
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));	  //Epilogue header
    list_begin = heap_listp + (2*WSIZE);
    // Insert initial free block into initialized heap
    if(extend_heap(WSIZE) == NULL)
	return -1;
    return 0;
}

/* 
 * mm_malloc - Search free list for free block of sufficient size. Extend heap
 *     if no such block is present.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;		// Adjusted block size
    size_t extendsize;		// Amount to extend heap if large enough block is not found
    char *bp;
    // Ignore invalid requests
    if(size == 0)
	return NULL;
    // Adjust block size to include overhead and alignment requirements
    if(size <= DSIZE)
	asize = 2*DSIZE;
    else
	asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);
    // Perform search for an appropriate free block. If found, place requested block.
    if((bp = find_fit(asize)) != NULL)
    {
	place(bp, asize);
	return bp;
    }
    // If no fit is found, extend the heap and place the block at end block.
    extendsize = MAX(asize, CHUNKSIZE);
    if((bp = extend_heap(extendsize/WSIZE)) == NULL)
	return NULL;
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Free block at designated address. Resets allocated bits in boundary tags.
 * Performs immediate coalescing.
 */
void mm_free(void *ptr)
{
    // Ignore invalid requests
    if(ptr == NULL)
	return;
    // Reset boundary tags
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
}

/*
 * mm_realloc - Reallocates a given allocated block with a new size. Several cases.
 *  If given size is 0, free the block. If given size is less than original, leave block unchanged.
 *  If given size is greater, coalesce next block if possible, or reallocate a new block.
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *newptr;
    size_t copySize;
    // If size is zero, free block
    if(size == 0)
    {
	mm_free(ptr);
	return NULL;
    }
    copySize = size + DSIZE;
    size_t osize = GET_SIZE(HDRP(ptr));
    // If size is less than original block, no change needed.
    if (copySize <= osize)
      return ptr;
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
    size_t combine_size = osize + GET_SIZE(HDRP(NEXT_BLKP(ptr)));
    // If size is larger than original, coalesce the next block if it is free and large enough
    // to accommodate the new size.
    if(!next_alloc && (combine_size >= copySize))
    {
	remove_from_list(NEXT_BLKP(ptr));
	PUT(HDRP(ptr), PACK(combine_size, 1));
        PUT(FTRP(ptr), PACK(combine_size, 1));
        return ptr;
    }
    // If coalescing fails, allocate a new block of sufficient size.
    newptr = mm_malloc(copySize);
    place(newptr, copySize);
    memcpy(newptr, ptr, copySize);
    mm_free(ptr);
    return newptr;
}

/*
 * extend_heap - In the event there is insufficient memory in heap, allocates a new block
 * of size words. Adjusts epilogue header.
 */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;
    // Adjust block size according to double word alignment
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if(size < (2*DSIZE))
	size = (2*DSIZE);
    if((long)(bp = mem_sbrk(size))  == -1)
	return NULL;
    // Initialize the new free block's header and footer, adjust epilogue header
    PUT(HDRP(bp), PACK(size, 0));		// header
    PUT(FTRP(bp), PACK(size, 0));		// footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));	// epilogue header
    // Coalesce if previous block was free
    return coalesce(bp);
}

/*
 * coalesce - Utilizes boundary tag coalescing to recombine a given free block with
 * any adjacent free blocks.
 */
static void *coalesce(void *bp)
{
    // Extract allocation status from next and previous blocks
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp))) || PREV_BLKP(bp) == bp;
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    // Case 1: Previous and next blocks both allocated. No coalescing done.
    if(prev_alloc && next_alloc)
    {	
	return_to_list(bp);
	return bp;
    }
    // Case 2: Previous block is allocated, while next block is free. Coalesce two blocks.
    else if(prev_alloc && !next_alloc)
    {
	size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
	remove_from_list(NEXT_BLKP(bp));
	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
	return_to_list(bp);
	return bp;
    }
    // Case 3: Previous block is free, while next block is allocated. Coalesce two blocks.
    else if(!prev_alloc && next_alloc)
    {
	size += GET_SIZE(HDRP(PREV_BLKP(bp)));
	bp = PREV_BLKP(bp);
	remove_from_list(bp);
	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
	return_to_list(bp);
	return bp;
    }
    // Case 4: Both previous and next blocks are free. Coalesce three blocks.
    else
    {
	size += (GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp))));
	remove_from_list(PREV_BLKP(bp));
	remove_from_list(NEXT_BLKP(bp));
	bp = PREV_BLKP(bp);
	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
	return_to_list(bp);
	return bp;
    }
}

/*
 * place - Place a requested block at the beginning of a chosen free block. Split the free
 * block only if the size of the remainder exceeds the minimum block size.
 */
static void place(void *bp, size_t size)
{
    // Remainder is a greater size than minimum block size
    size_t bsize = GET_SIZE(HDRP(bp));
    size_t remainder = bsize - size;
    // Split free block, perform immediate coalescing.
    if(remainder >= (2*DSIZE))
    {
	PUT(HDRP(bp), PACK(size, 1));
	PUT(FTRP(bp), PACK(size, 1));
	remove_from_list(bp);
	bp = NEXT_BLKP(bp);
	PUT(HDRP(bp), PACK(remainder, 0));
	PUT(FTRP(bp), PACK(remainder, 0));
	coalesce(bp);
    }
    // Remainder is not large enough
    else
    {
	PUT(HDRP(bp), PACK(bsize, 1));
	PUT(FTRP(bp), PACK(bsize, 1));
	remove_from_list(bp);	
    }
}

/*
 * find_fit - Given a requested size, perform a linear search of the explicit list.
 * When a block of sufficient size is found, return the address of that block.
 * If no appropriate block found, return NULL. 
 */
static void *find_fit(size_t asize)
{ 
    void *bp; 
    // Linear search through the free list to find a block large enough.
    for(bp = list_begin; GET_ALLOC(HDRP(bp)) == 0; bp = GET_NEXT_FBLK(bp))
    {
	if((GET_SIZE(HDRP(bp)) >= asize))
	{
	    return bp;
	}
    }
    // Appropriate block not found, return NULL.
    return NULL;
}

/*
 * mm_check - Heap consistency checker: Performs a check on the entire heap. Will return error messages
 * if any errors are found in the prologue or epilogue blocks, as well as boundary/alignment issues with
 * regular blocks.
 * Also performs a cursory check on the explicit free list. Checks if the free list actually contains all
 * free blocks, as well as if the pointers all point to valid free blocks.
 * Return -1 if error found, 0 otherwise.
 */
static int mm_check(void)
{
    void *ptr = heap_listp;
    size_t free_block_count = 0;
    size_t free_list_count = 0;
    // Check validity of prologue block.
    if(GET_SIZE(HDRP(ptr)) != DSIZE || !GET_ALLOC(HDRP(ptr))){
	printf("Error: Invalid prologue header\n");
	return -1;
    }
    if((long)ptr % DSIZE != 0){
	printf("Error: Prologue block is not properly aligned\n");
	return -1;
    }
    if(GET(HDRP(ptr)) != GET(FTRP(ptr))){
	printf("Error: Prologue header and footer mismatched\n");
	return -1;
    }
    // Loop through all the storage blocks in the heap and check validity of each block.
    for(; GET_SIZE(HDRP(ptr)) > 0; ptr = NEXT_BLKP(ptr))
    {
	if(GET(HDRP(ptr)) != GET(FTRP(ptr))){
	    printf("Error: Block %p header and footer mismatched\n", ptr);
	    return -1;
	}
	if((long)ptr % DSIZE != 0){
            printf("Error: Block %p is not properly aligned\n", ptr);
	    return -1;
	}
	if(!GET_ALLOC(HDRP(ptr)))
	    free_block_count++;
    }
    // Check validity of epilogue block.
    if(GET_SIZE(HDRP(ptr)) != 0 || !GET_ALLOC(HDRP(ptr))){
	printf("Error: Invalid epilogue header\n");
	return -1;
    }
    
    // Checking validity of explicit free list
    void *free_ptr;
    for(free_ptr = GET_NEXT_FBLK(list_begin); GET_NEXT_FBLK(free_ptr) != NULL; 
	free_ptr = GET_NEXT_FBLK(free_ptr))
    {
	if(GET_NEXT_FBLK(free_ptr) != NULL)
	{
	    if(GET_ALLOC(GET_PREV_FBLK(free_ptr)) || GET_ALLOC(GET_NEXT_FBLK(free_ptr))){
	        printf("Error: Invalid pointers. Allocated block found in free list\n");
		return -1;
	    }
	}
	free_list_count++;
    }
    if(free_block_count != free_list_count){
	printf("Error: Not all free blocks found in free list\n");
	return -1;
    }
    return 0;
}


/*
 * return_to_list - LIFO policy for returning free blocks to the free list
 */
static void return_to_list(void *bp)
{
    SET_NEXT_FBLK(bp, list_begin);
    SET_PREV_FBLK(list_begin, bp);
    SET_PREV_FBLK(bp, NULL);
    list_begin = bp;
}

/*
 * remove_from_list - Remove designated block from free list, rearrange pointers accordingly.
 */
static void remove_from_list(void *bp)
{
    // If block being removed is the first block of the free list.
    if(GET_PREV_FBLK(bp) == NULL)
    {
	list_begin = GET_NEXT_FBLK(bp);
    }
    // Any other block.
    else
    {
	SET_NEXT_FBLK(GET_PREV_FBLK(bp), GET_NEXT_FBLK(bp));
    }
    SET_PREV_FBLK(GET_NEXT_FBLK(bp), GET_PREV_FBLK(bp));
}


