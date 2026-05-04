#ifndef MYCACHE_H
#define MYCACHE_H



#define CACHE_SIZE 1049000


struct item_s{
    char key[MAXLINE];   // hostname:port/path
    
    char* body;

    size_t body_size;

    int visit_cnt;       // LRU -- array_index && visit_cnt
			 // array_index smaller && visit_cnt smaller  => Older && less used 

    struct item_s* next;
};



struct cache_s{
    int current_size;
    int max_size;

    struct item_s* first_item;

    struct item_s* last_item;

    int item_cnt;


};




void construct_key(char* key, char* hostname, char* port, char* path);

struct item_s* alloc_item(char* key, char* body_buf, size_t body_size);

void free_item(struct item_s* item);

void init_cache(struct cache_s* cache);

void add_item(struct cache_s* cache, struct item_s* new_item);

void delete_item(struct cache_s* cache, struct item_s* del_item);

struct item_s* find_item(struct cache_s* cache, char* key);

void lru_evict(struct cache_s* cache, struct item_s* new_item);




#endif
