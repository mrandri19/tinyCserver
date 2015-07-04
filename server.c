#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <yajl/yajl_tree.h> 

const char *port = "8080";
const char *msg = "daw";
const int backlog = 10;
const char *config_path = "./config.json";

int readConfig()
{
    int return_status;
    size_t rd;
    yajl_val node;
    char errbuf[1024];
    char configData[65536];
    FILE *fd;
    fd = fopen(config_path, "r");

    rd = fread((void *) configData, 1, sizeof(configData)-1, fd);
    //TODO error checking
    
    node = yajl_tree_parse((const char *) configData, errbuf, sizeof(errbuf));
    //TODO error checking

    const char *path[] = {"message"};
    yajl_val v = yajl_tree_get(node, path, yajl_t_string);
    //TODO error checking

    fprintf(stderr, "%s\n", YAJL_GET_STRING(v));

    yajl_tree_free(node);
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
    socklen_t addr_size;
    
    s = setup(&sockfd);
    //TODO error checking
    s = readConfig();

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
