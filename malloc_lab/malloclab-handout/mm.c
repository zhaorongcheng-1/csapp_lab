/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
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
    "ZRC",
    /* First member's full name */
    "RongCheng Zhao",
    /* First member's email address */
    "zhaorc@test.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))



/* 
 * zhaorc's macros
 * */

#define WSIZE		4
#define DSIZE		8
#define CHUNKSIZE	(1<<12)		/* Extend heap by this amount(bytes) */

#define MAX(x, y)	((x) > (y)? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)	((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)			(*(unsigned int*)(p))
#define PUT(p, val)		(*(unsigned int*)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)		(GET(p) & ~0x7)
#define GET_ALLOC(p)		(GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)		((char*)(bp) - WSIZE)
#define FTRP(bp)		((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Gived block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)		((char*)(bp) + GET_SIZE(((char*)(bp) - WSIZE)))
#define PREV_BLKP(bp)		((char*)(bp) - GET_SIZE(((char*)(bp) - DSIZE)))



#define PRED_ADDR(bp)		((bp))
#define SUCC_ADDR(bp)		((char*)(bp) + WSIZE)
#define INVALID_BLOCK_ADDR_32		0xFFFFFFFF	



#define LISTS_ARRAY_SIZE		10

#define MINIMUM_BLOCK_SIZE		16



/* 
 * minimum block size is 16 bytes
 * malloc return address is aligned to 8 bytes
 *
 * */


/* 
 * Each free list is associated with a size class
 * 
 * size is (1<<(4 + array_index)):
 *
 * fits           : 0-16, 17-32, 33-64, 65-128, ...,                2049-4096, 4097-
 * list block size: {16}, {32}, {48-64}, {80-128}, {144-256}, ..., {2064-4096},  {4112- }
 * */

static char* list_array[LISTS_ARRAY_SIZE];

static char* heap_listp;







static void* insert_block_to_list(void* bp)
{
    int list_head;
    size_t size = GET_SIZE(HDRP(bp));
    
    char* old_bp;

    
    // 1. find proper list for block
    list_head = 0;
    size_t t_size = size >> 5;

    while (t_size > 0)
    {
        t_size = t_size >> 1;
	list_head += 1;
    }

    if ((size ^ (1 << (list_head+4))) > 0)
    {
        list_head += 1;
    }


    if (list_head >= LISTS_ARRAY_SIZE)
    {
        list_head = LISTS_ARRAY_SIZE - 1;
    }


    // 2. initialize pred/succ pointer
    PUT(PRED_ADDR(bp), list_head);

    old_bp = list_array[list_head];
    PUT(SUCC_ADDR(bp), old_bp);
    


    // 3. insert block to list
    list_array[list_head] = bp;

    if (old_bp != NULL)
    {
        PUT(PRED_ADDR(old_bp), bp);
    }


    return bp;

}


static void *pick_block_from_list(void* bp)
{
    unsigned int list_head;

    void* pred_block_bp;
    void* succ_block_bp;

    pred_block_bp = GET(PRED_ADDR(bp));
    succ_block_bp = GET(SUCC_ADDR(bp));

    list_head = GET(PRED_ADDR(bp));


    if (list_head == INVALID_BLOCK_ADDR_32)
    {
        return bp;
    }

    // 1. change predecessor block's successor
    if (list_head < LISTS_ARRAY_SIZE)
    {
        list_array[list_head] = succ_block_bp;
    }
    else
    {
        PUT(SUCC_ADDR(pred_block_bp), succ_block_bp);
    }

    
    // 2. change successor block's predecessor
    if (succ_block_bp != NULL)
    {
        PUT(PRED_ADDR(succ_block_bp), pred_block_bp);
    }


    return bp;


}




static void *find_fit(size_t asize)
{
    int list_head;
    size_t size = asize;

    char* list;
    char* bp;

    size_t b_size;
    int find;


    // 1. find proper list for block
    list_head = 0;
    size_t t_size = size >> 5;

    while (t_size > 0)
    {
        t_size = t_size >> 1;
	list_head += 1;
    }

    if ((size ^ (1 << (list_head+4))) > 0)
    {
        list_head += 1;
    }

 
    find = 0;
    bp = NULL;
    while (!find 
	    && list_head < LISTS_ARRAY_SIZE 
	    ) 
    {
        bp = list_array[list_head];

	while (bp != NULL && bp != INVALID_BLOCK_ADDR_32)
	{

	    b_size = GET_SIZE(HDRP(bp));
	    if (b_size >= asize)
	    {
		find = 1;
		break;
	    }

	    bp = GET(SUCC_ADDR(bp));

        }

	list_head++;
    }


    if (!find)
        return NULL;


    return bp;

}

static void place(void* bp, size_t asize)
{
    size_t b_size;
    size_t l_size;

    size_t f_size;
    
    char* l_bp;
    // 1. pick block from list
   pick_block_from_list(bp);


    // 2. check if need split block
    b_size = GET_SIZE(HDRP(bp));


    // 3. splitting

    // case A: block need splitting
    if (b_size > asize && ((b_size - asize) >= MINIMUM_BLOCK_SIZE))
    {
        l_size = b_size - asize;


	PUT(HDRP(bp), PACK(asize, 1));
	PUT(FTRP(bp), PACK(asize, 1));


	l_bp = NEXT_BLKP(bp);
	PUT(HDRP(l_bp), PACK(l_size, 0));
	PUT(FTRP(l_bp), PACK(l_size, 0));


	insert_block_to_list(l_bp);


	return;

    }



    // case B: block don't need splitting
    PUT(HDRP(bp), PACK(b_size, 1));
    PUT(FTRP(bp), PACK(b_size, 1));

}


static void *coalesce(void* bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    void* new_bp = bp;

    if (prev_alloc && next_alloc)
    {
        return new_bp;
    }
    else if (prev_alloc && !next_alloc)
    {
        pick_block_from_list(NEXT_BLKP(bp));


	size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
	new_bp = bp;


    }
    else if (!prev_alloc && next_alloc)
    {
        pick_block_from_list(PREV_BLKP(bp));


	size += GET_SIZE(HDRP(PREV_BLKP(bp)));
	PUT(FTRP(bp), PACK(size, 0));
	PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
	new_bp = PREV_BLKP(bp);


    }
    else
    {
        pick_block_from_list(PREV_BLKP(bp));
	pick_block_from_list(NEXT_BLKP(bp));

	size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
		GET_SIZE(FTRP(NEXT_BLKP(bp)));

	PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
	PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
	new_bp = PREV_BLKP(bp);

    }


    return new_bp;


}




static void *extend_heap(size_t words)
{
    char* bp;
    size_t size;

    size = (words % 2) ? (words+1) * WSIZE: words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;


    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    // TODO: initialize free pred/succ block pointr
    PUT(PRED_ADDR(bp), INVALID_BLOCK_ADDR_32);
    PUT(SUCC_ADDR(bp), 0);


    return coalesce(bp);

}





/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    char* bp;

    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void*)-1)
        return -1;    

    PUT(heap_listp, 0);
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));

    heap_listp += (2*WSIZE);

    // TODO: initialize free block list
    for (int i = 0; i < LISTS_ARRAY_SIZE; i++)
    {
        list_array[i] = 0;
    }


    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if ((bp = extend_heap(CHUNKSIZE/WSIZE)) == NULL)
        return -1;

 
    // TODO: insert free block into block list
    insert_block_to_list(bp);


    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char* bp;


    if (size == 0)
    {
        return NULL;
    }

    if (size <= DSIZE)
    {
        asize = 2*DSIZE;
    }
    else
    {
        asize = DSIZE * ((size + DSIZE + (DSIZE-1)) / DSIZE);
    }


    if ((bp = find_fit(asize)) != NULL)
    {
        place(bp, asize);
	return bp;
    }


    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
    {
        return NULL;
    }
    place(bp, asize);
    return bp;



}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    void* bp = ptr;

    if (bp == NULL)
        return;


    size_t size = GET_SIZE(HDRP(bp));
    void* new_bp;
    
    // 1. change block state to free
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));

    // 2. try coalesce blocks
    new_bp = coalesce(bp);

    // 3. insert free block to list
    insert_block_to_list(new_bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














