#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <yajl/yajl_tree.h> 
#include <errno.h>

struct config_data{
    char *message;
};
const char *port = "8080";
const char *msg = "daw\n";
const int backlog = 10;
const char *config_path = "./config.json";



/* 
 * Gets the config file, which must be in valid JSON, from the path in *config_path.
 * Gets the file size and allocates a buffer
 * Reads the config in the buffer
 * Parses the JSON and returns a struct with the config
 *
 */
int readConfig(struct config_data *cfg_data)
{
    int return_status, size = 0;
    size_t rd;
    yajl_val node;
    char errbuf[1024];
    char *configBuffer;
    FILE *fd;
    struct stat st;

    fd = fopen(config_path, "r");
    if (!fd) {
        perror("Failed to open the file");
    }

    int filedes = fileno(fd);

    fstat(filedes, &st);
    
    size = st.st_size;

    configBuffer = malloc(size*sizeof(char));
    if (!configBuffer) {
        perror("Failed to allocate configBuffer Buffer");
    }

    rd = fread(configBuffer, sizeof(char), st.st_size-1, fd);
    if (!rd) {
        perror("Failed to read");
    }
    
    node = yajl_tree_parse((const char *) configBuffer, errbuf, sizeof(errbuf));
    if (!node){
        perror("Failed to parse");
    }

    const char *path[] = {"message", (char *)0};
    yajl_val v = yajl_tree_get(node, path, yajl_t_string);
    if (!v) {
        perror("Failed to get the message");
    }

    cfg_data->message = strdup(YAJL_GET_STRING(v));

    yajl_tree_free(node);
    free(configBuffer);
    return return_status;
}

int setup(int *sockfd)
{
    int return_status, s;
    struct sockaddr_storage;
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    //Gets the addrinfo struct on localhost:8080
    s = getaddrinfo(NULL, port, &hints, &res);
    if (s != 0) {
        fprintf(stderr, "Error in addrinfo: %s", gai_strerror(s));
        exit(0);
    } 
    //Creates a socket
    *sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    //Binds the socket to the port
    s = bind(*sockfd, res->ai_addr, res->ai_addrlen);
    //TODO error checking

    freeaddrinfo(res);

    //Starts listening on the socket
    s = listen(*sockfd, backlog);
    //TODO error checking

    return return_status;
}

int main()
{
    int s, sockfd, new_fd;
    int i = 0;
    struct sockaddr their_addr;
    struct config_data *cfg_data;
    socklen_t addr_size;
    
    memset(cfg_data, 0, sizeof(*cfg_data));

    s = setup(&sockfd);
    //TODO error checking

    s = readConfig(cfg_data);
    fprintf(stderr, "Inside main: %s\n", cfg_data->message);

    while (1) {
        //Accepts the connection, returning a new socket and changing the sockaddr
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
        fprintf(stderr, "Accepted a connection, #%d\n",i);
        i++;
        //Send a response
        int len = strlen(msg);
        s = send(new_fd, msg, len, 0);
        
        //Check if the response has been sent completely
        if (s<len){
            fprintf(stderr, "Error in send, send less bytes than the msg\n");
        }
        
        close(new_fd);
    }

    close(sockfd);
    return 0;
}
