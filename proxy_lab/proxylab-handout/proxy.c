#include <stdio.h>

#include "csapp.h"

#include <string.h>
#include <signal.h>

#include "cache.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400



#define DEFAULT_PORT 80


/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

static const char *http_version = "HTTP/1.0";

static const char* support_method = "GET";


int readcnt;    /* Initially = 0 */
sem_t mutex;    /* Initially = 1 */
sem_t w;        /* initially = 1 */


struct cache_s cache;



// parse URL: http://hostname:port/path
int parse_uri(char* uri, char* server_name, char* port, char* path);



void readrequest_line(rio_t *rp, char* method, char* uri, char* version);

void readrequest_hdrs(rio_t *rp);




// make proxy connect to server
int connect_to_server(char* hostname, char* port);



// build request line send to server
void build_request_line(char* forward_request, char* method, char* path, char* version);

// build request hdrs send to server
void build_request_hdrs(char* forward_request, char* server_name);



// forward server response to client
size_t forward_server_response_to_client(int client_connfd, int proxy_connfd, char* response_body);


// forward cache object to client
void forward_cache_object_to_client(int client_connfd, struct item_s* item);


// handle a connect from, process client HTTP request
void do_it(int client_connfd);



// handle connected tcp thread
void* handle_thread(void* vargp);


int main(int argc, char **argv)
{
    int listenfd;
    int* connfd_ptr;

    char clientname[MAXLINE], port[MAXLINE];

    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    pthread_t tid;


    if (argc != 2) {
        fprintf (stderr, "usage %s <port>\n", argv[0]);
	exit(1);
    }

    signal(SIGPIPE, SIG_IGN);


    // init cache
    init_cache(&cache);


    readcnt = 0;

    // init semaphore
    Sem_init(&mutex, 0, 1);
    Sem_init(&w, 0, 1);



    listenfd = Open_listenfd(argv[1]);

    while(1) {
        clientlen = sizeof(clientaddr);
	connfd_ptr = Malloc(sizeof(int));

	*connfd_ptr = Accept(listenfd, (SA *)&clientaddr, &clientlen);

	Getnameinfo((SA *)&clientaddr, clientlen, clientname, MAXLINE, port, MAXLINE, 0);
	printf ("Proxy: Accept connection from (%s %s)\n", clientname, port);

	Pthread_create(&tid, NULL, handle_thread, connfd_ptr);
    
    }


    return 0;
}



void* handle_thread(void* vargp)
{
    int connfd = *((int*) vargp);
    Pthread_detach(Pthread_self());
    Free(vargp);

    do_it(connfd);

    Close(connfd);

    return NULL;

}













void do_it(int client_connfd)
{
    char buf[MAXLINE];

    char forward_request[MAXBUF];

    rio_t client_rio;

    char method[MAXLINE], uri[MAXLINE], version[MAXLINE];

    char server_name[MAXLINE], port[MAXLINE], path[MAXLINE];

    int proxy_connfd;


    char item_key[MAXLINE];
    struct item_s* item = NULL;

    char response_body[MAX_OBJECT_SIZE];
    size_t response_size = 0;


    Rio_readinitb(&client_rio, client_connfd);

    // read client HTTP request

    // 1. read request line
    //    parse request line URI: server_name, port, path
    readrequest_line(&client_rio, method, uri, version);

    parse_uri(uri, server_name, port, path);

#if 0
    printf ("Request line: %s %s %s\n", method, uri, version);

    printf ("Server: %s Port: %s Path: %s\n", server_name, port, path);
#endif

    if (strcmp(method, support_method) != 0)
    {
        fprintf(stderr, "not support HTTP method %s\n", method);
	
	return;
    }

    if (strcmp(version, http_version) != 0)
    {
        memset(version, 0, sizeof(version));
	strcpy(version, http_version);
    }


    // 2. read request headers

    readrequest_hdrs(&client_rio);



    // 3. try to find response in cache, if find, then reply and return

    construct_key(item_key, server_name, port, path);
    printf("KEY: {%s}\n", item_key);


    P(&mutex);
    readcnt++;
    if (readcnt == 1)
        P(&w);
    V(&mutex);


    item = find_item(&cache, item_key);


    P(&mutex);
    readcnt--;
    if (readcnt == 0)
        V(&w);
    V(&mutex);


    if (item != NULL)
    {
        printf("forward cache object\n");
        forward_cache_object_to_client(client_connfd, item);

	return;
    }



    // 4. connect to Web server

#if 0
    printf("Try connect server %s port %s\n", server_name, port);
#endif

    proxy_connfd = connect_to_server(server_name, port);
    if (proxy_connfd < 0)
    {
        return;
    }

#if 0
    printf("connect server %s port %s\n", server_name, port);
#endif
    

    // 5. proxy build HTTP request to server

    memset(forward_request, 0, sizeof(forward_request));
    
    build_request_line(forward_request, method, path, version);

    build_request_hdrs(forward_request, server_name);

#if 0
    printf("==== Sending to server ====\n");
    printf("%s", forward_request);
    printf("==== Sending to server ====\n");
#endif

    // 6. forward request to server

    Rio_writen(proxy_connfd, forward_request, strlen(forward_request));

    // 7. forward server response to client and try cache response

    printf("forward server response\n");
    response_size = forward_server_response_to_client(client_connfd, proxy_connfd, response_body);

    if (response_size <= MAX_OBJECT_SIZE)
    {
	P(&w);
	
	// sleep thread to test concurrency and cache
	usleep((rand() % 100) * 1000);
	

	// find item again
	//printf ("find item AGAIN\n");
	item = find_item(&cache, item_key);
	if (item != NULL)
	{
	    V(&w);
	    return;
	}

	usleep((rand() % 100) * 1000);


        // cache
	printf ("alloc item\n");
	item = alloc_item(item_key, response_body, response_size);
        
	
	printf ("lru evict\n");
	lru_evict(&cache, item);


	V(&w);
    
    }


    return;
}





void readrequest_line(rio_t *rp, char* method, char* uri, char* version)
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);

#if 0
    printf("Request header line: %s\n", buf);
#endif

    sscanf(buf, "%s %s %s", method, uri, version);


    // method == GET
    // version == HTTP/1.0


    return;

}





int parse_uri(char* uri, char* server_name, char* port, char* path)
{
    char* host_start;
    char* host_end;
    char* path_start;
    char* port_start;


    char* hostname = server_name;


    int default_port = DEFAULT_PORT;

    if (strncasecmp(uri, "http://", 7) == 0) {
        host_start = uri + 7;
    } else {
        strcpy(path, uri);
	strcpy(hostname, "");
	sprintf(port, "%d", default_port);

	return 0;
    }


    path_start = strchr(host_start, '/');

    if (path_start == NULL) {
        path_start = host_start + strlen(host_start);
	strcpy(path, "/");
    } else {
        strcpy(path, path_start);
    }



    host_end = host_start;
    while (*host_end != '\0' && *host_end != ':' && *host_end != '/') {
        host_end ++;
    }


    if (*host_end == ':') {
        port_start = host_end + 1;

	char* port_end = port_start;
	while (*port_end != '\0' && *port_end != '/') {
	    port_end++;
	}

	int port_len = port_end - port_start;
	strncpy(port, port_start, port_len);
	port[port_len] = '\0';

	int host_len = host_end - host_start;
	strncpy(hostname, host_start, host_len);
	hostname[host_len] = '\0';

    } else {
        sprintf(port, "%d", default_port);

	int host_len = host_end - host_start;
	strncpy(hostname, host_start, host_len);
	hostname[host_len] = '\0';
    
    }

    return 0;


}


void readrequest_hdrs(rio_t *rp)
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);

    while (strcmp(buf, "\r\n")) {
        Rio_readlineb(rp, buf, MAXLINE);

#if 0
	printf ("%s", buf);
#endif

    }

    return;

}



int connect_to_server(char* hostname, char* port)
{
    int proxy_connfd = Open_clientfd(hostname, port);

    return proxy_connfd;


}




void build_request_line(char* forward_request, char* method, char* path, char* version)
{
    // version is HTTP/1.0


    sprintf (forward_request, "%s %s %s\r\n", method, path, version);


    return;

}



void build_request_hdrs(char* forward_request, char* server_name)
{

    sprintf (forward_request, "%sHost: %s\r\n", forward_request, server_name);

    sprintf (forward_request, "%sUser-Agent: %s", forward_request, user_agent_hdr);

    sprintf (forward_request, "%sConnection: close\r\n", forward_request);

    sprintf (forward_request, "%sProxy-Connection: close\r\n\r\n", forward_request);


}




size_t forward_server_response_to_client(int client_connfd, int proxy_connfd, char* response_body)
{
    size_t n;

    size_t response_size = 0;

    char* body_p = response_body;

    char buf[MAXLINE];

    rio_t server_rio;

    Rio_readinitb(&server_rio, proxy_connfd);


//    printf ("==== Server response ====\n");

    while((n = Rio_readlineb(&server_rio, buf, MAXLINE)) > 0) {

	response_size += n;

	if (response_size <= MAX_OBJECT_SIZE)
        {
	    memcpy(body_p, buf, n);
	    body_p += n;
	}

//	printf("%s", buf);

	Rio_writen(client_connfd, buf, n);
    }

//    printf ("==== Server response ====\n");


    return response_size;

}

void forward_cache_object_to_client(int client_connfd, struct item_s* item)
{
    size_t n = item->body_size;

    char* buf = item->body;


    Rio_writen(client_connfd, buf, n);

}






