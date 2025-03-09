#include "cachelab.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define TAG_BYTES           8
#define VALID_BYTE          1
#define LRU_TIMES_BYTES     4 


#define LOAD          1
#define STORE         2
#define MODIFY        3

#define ADDR_BITS      64

unsigned int s;
unsigned int e;
unsigned int b;

char file_path[64];

char str[64];

int visual;

void cache_op_sim (uint64_t*** cache, char op, uint64_t address, int size,
		   int* hit, int* miss, int* evi);

uint64_t*** allocCache();

int main(int argc, char* argv[])
{
    char c;
    char op;
    uint64_t addr = 0;
    int size = 0;

    int hit = 0;
    int miss = 0;
    int evi = 0;

    int h = 0;
    int m = 0;
    int ev = 0;

    // parse args, get s, e, b, file_path
    if (strncmp(argv[1], "-h", 2) == 0)
    {
        printf ("help info:\n");
        return 0;
    }

    if (argc < 9)
    {
        printf ("error args\n");
	return 0;
    }

    int args_shift = 0;

    visual = 0;

    if (strncmp(argv[1], "-v", 2) == 0)
    {
        visual = 1;
	args_shift = 1;
    }


    sscanf(argv[args_shift + 2], "%d", &s);
    sscanf(argv[args_shift + 4], "%d", &e);
    sscanf(argv[args_shift + 6], "%d", &b);

    strcpy(file_path, argv[args_shift + 8]);
 
    //printf ("s:%d e:%d b:%d path:%s\n\n", s, e, b, file_path);


    // allocate cache
    uint64_t*** cache = allocCache();
    if (!cache)
    {
        printf ("alloc cache failed\n");
	return 0;
    }

#if 0

    int s_n = 1 << s;

    uint64_t** set = NULL;
    uint64_t* line = NULL;

    for (int i = 0; i < s_n; i++)
    {
	set = cache[i];

	printf ("set addr: 0x%lx\n", set);

        for (int j = 0; j < e; j++)
	{
	    line = set[j];
	    printf ("  line addr: 0x%lx\n", line);
	    printf ("   %ld %ld %ld\n", line[0], line[1], line[2]);
	
	}

	printf("\n");
    }



#endif

    // parse cache access
    // simulater cache 
    FILE* fp = fopen(file_path, "r");
    if (!fp)
    {
	printf ("open file '%s' failed\n", file_path);
        return 0;
    }

    while (fgets(str, 64, fp) != NULL)
    {
	h = 0;
	m = 0;
	ev = 0;

        sscanf(str, "%c", &c);
	if (c == ' ')
	{
	    sscanf(str, " %c %lx,%d", &op, &addr, &size);
	    
	    //printf("%c, ox%lx, %d\n", op, addr, size);


	    cache_op_sim (cache, op, addr, size, &h, &m, &ev);

#if 1
	    if (visual != 0)
	    {
	        printf("%c %lx,%d ", op, addr, size);

	        if (h)
	            printf("hit ");
	        if (h == 2)
		    printf("hit ");
	        if (m)
		    printf("miss ");
	        if (ev)
		    printf("eviction ");

	        printf("\n");

	        //printf ("%d hit, %d miss, %d evi\n\n", h, m, ev);
	    }
#endif

	    hit += h;
	    miss += m;
	    evi += ev;


	}
    }


#if 1
    printSummary(hit, miss, evi);

#endif

    return 0;
}

void parseArgs()
{

}

void freeSet(uint64_t** set)
{
    if (!set)
        return;

    for (unsigned i = 0; i < e; i++)
    {
        if (set[i])
	    free(set[i]);
    }

    free(set);
}



// allocate cache per SET
// return 64-bits address of per SET
// block offest if not used, because if set&line match, word is in block
uint64_t** allocSet()
{
    uint64_t** set = calloc(e, sizeof(uint64_t));
    if (!set)
        return NULL;

    for (unsigned i = 0; i < e; i++)
    {
	uint64_t* line = calloc(1 + 1 + 1, sizeof(uint64_t));


	if (!line)
	    goto FAILED;
#if 0
        line[0] = 1;
	line[1] = 2;
	line[2] = 3;
#endif
	set[i] = line;
    }

    return set;


FAILED:

    freeSet(set);

    return NULL;
}



// allocate cache
// return 64-bits CACHE_LINES address array
uint64_t*** allocCache()
{
    unsigned set_num = 1 << s;

    uint64_t*** sets = calloc(set_num, sizeof(uint64_t));
    if (!sets)
        return NULL;

    for (unsigned i = 0; i < set_num; i++)
    {
        sets[i] = allocSet();
	if (!sets[i])
            goto FAILED;
    }

    return sets;


FAILED:
    
    for (unsigned i = 0; i < set_num; i++)
    {
        if (sets[i])
	    freeSet(sets[i]);
    }

    free(sets);
    
    return NULL;
}

// do cache block replace,
// return whether have an eviction
int cache_replace (uint64_t** set, uint64_t tag)
{
    unsigned evicted_line = 0;

    uint64_t lru = 1;

    uint64_t* line = NULL;

    for (unsigned i = 0; i < e; i++)
    {
	line = set[i];

        if (!line[0])
        {
	    line[0] = 1;
            line[1] = tag;
	    line[2] = 0;

	    return 0;
        }
        
	// prefer to evict later cache line
	if (line[2] >= lru)
	{
	    lru = line[2];
	    evicted_line = i;
	}
    }

    // do eviction
    line = set[evicted_line];
    line[0] = 1;
    line[1] = tag;
    line[2] = 0;

    return 1;

}

void cache_op_sim (uint64_t*** cache, char op, uint64_t address, int size, 
		int* hit, int* miss, int* evi)
{
 
    uint64_t base = -1;
    uint64_t tag_mask = base << (s + b);
    uint64_t set_mask = (~ tag_mask) >> b;

    int set_index = (address >> b) & set_mask;    
    uint64_t tag = (address >> (b + s));

    uint64_t** set = cache[set_index];

    int match = 0;
    int eviction = 0;

    for (unsigned i = 0; i < e; i++)
    {


        uint64_t* line = set[i];
	line[2]++;

#if 0
	if (set_index == 3)
        {
            printf ("\n addr: 0x%x i:%d line: 0x%x  cur_tag: 0x%x\n", address, i, line, tag);
	    printf ("line[0] = %d, line[1] = 0x%x, line[2] = %d\n\n", line[0], line[1], line[2]);
            
        }

#endif

        if (match == 0 && line[0] && line[1] == tag)
	{
	    // match
	    // not break loop when match, we need up date LRU count
            match = 1;

	    line[2] = 0;
	}

    }



    if (match)
    {
        *hit = 1;
	if (op == 'M')
	    *hit = 2;
    }
    else
    {
	*miss = 1;
        // do replace
	eviction = cache_replace(set, tag);
	*evi = eviction;

	if (op == 'M')
            *hit = 1;
	
    }

    return;
}



























