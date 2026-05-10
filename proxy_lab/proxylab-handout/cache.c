/*
 * for lab cache use
 * */
#include "csapp.h"

#include "cache.h"


void construct_key(char* key, char* hostname, char* port, char* path)
{
    sprintf(key, "%s:%s/%s", hostname, port, path);

}	

struct item_s* alloc_item(char* key, char* body_buf, size_t body_size)
{

    int key_len = strlen(key);
    
    struct item_s* item = Malloc(sizeof(struct item_s));

    memset(item, 0, sizeof(struct item_s));

    item->body = Malloc(sizeof(char) * body_size);
    memset(item->body, 0, sizeof(char) * body_size);

    memcpy(item->body, body_buf, body_size);


    memcpy(item->key, key, key_len);

    item->body_size = body_size;

    item->visit_cnt = 1;

    item->next = NULL;

    return item;

}


void free_item(struct item_s* item)
{

    Free(item->body);

    Free(item);

}



void init_cache(struct cache_s* cache)
{

    cache->current_size = 0;
    cache->max_size = CACHE_SIZE;

    cache->first_item = NULL;
    cache->last_item = NULL;

    cache->item_cnt = 0;

}


// add new item to rear of cache list
void add_item(struct cache_s* cache, struct item_s* new_item)
{
    

    if (cache->first_item == NULL)
    {
        cache->first_item = new_item;
	cache->last_item = new_item;
 
    }
    else
    {
        cache->last_item->next = new_item;
	cache->last_item = new_item;
    
    }


    cache->current_size += new_item->body_size;

    cache->item_cnt += 1;


}


void delete_item(struct cache_s* cache, struct item_s* del_item)
{
    
    struct item_s* item_pre = NULL;

    struct item_s* temp = cache->first_item;


    while(temp != NULL && temp != del_item)
    {
        item_pre = temp;
	temp = temp->next;
    
    }

    if (temp == NULL || temp != del_item)
    {
        printf("delete item error\n");
	return;
    
    }


    if (item_pre != NULL)
    {
        item_pre->next = del_item->next;
    }


    if (cache->first_item == del_item)
    {
        cache->first_item = del_item->next;
    }

    if (cache->last_item == del_item)
    {
        cache->last_item = item_pre;
    
    }

    // now del_item is deleted from item list
    //
    // then change cache size & cnt

    cache->current_size -= del_item->body_size;

    cache->item_cnt -= 1;



    free_item(del_item);

}



struct item_s* find_item(struct cache_s* cache, char* key)
{
    if (cache == NULL || cache->item_cnt == 0)
        return NULL;


    int key_len = strlen(key);

    struct item_s* temp = cache->first_item;

    while(temp != NULL)
    {
        if(strlen(temp->key) == key_len && strcmp(temp->key, key) == 0)
	{
	    break;
	}

	temp = temp->next;
    }

    if (temp != NULL)
    {
        // find item, add visit_cnt
        temp->visit_cnt += 1;
    }


    return temp;

}


void lru_evict(struct cache_s* cache, struct item_s* new_item)
{

    if (cache->current_size + new_item->body_size <= cache->max_size)
    {
        add_item(cache, new_item);

	return;
    }


    struct item_s* victim = cache->first_item;
    struct item_s* temp = victim->next;


    while(victim != NULL && temp != NULL)
    {
        if(victim->visit_cnt > temp->visit_cnt)
	{
	    victim = temp;
	}

	temp = temp->next;

    }

    size_t new_cache_size = cache->current_size - victim->body_size + new_item->body_size;


    if (new_cache_size > cache->max_size)
    {
        return;
    }


    delete_item(cache, victim);

    add_item(cache, new_item);

    return;

}


















