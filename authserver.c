#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <limits.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

enum {
    MAX_PORT = 65535
};

typedef struct
{
    unsigned char r[20];
    uint64_t T;
    char login[256];
    
} response;

typedef struct
{
    uint64_t high;
    uint64_t low;
} nonce;

void
usage()
{
    errx(EXIT_FAILURE,"usage: ./authserver file [int]");
}

int
to_int(char *str)
{
    long port;
    char *endptr;
    

    port = strtol(str,&endptr,10);
    if (port < 1 || port > MAX_PORT || *endptr != '\0') {
        errx(EXIT_FAILURE, "error: bad value %s", str);
    }
    
    return (int)port;
}

nonce
create_nonce(int *cont) {
    nonce n;
    int urandom_fd;

    urandom_fd = open("/dev/urandom",O_RDONLY);
    if (urandom_fd < 0) {
        err(EXIT_FAILURE,"open /dev/urandom failed");
    }

    if (read(urandom_fd,&n.high,sizeof(n.high)) != sizeof(n.high)) {
        err(EXIT_FAILURE,"read /dev/urandom failed");
    }

    close(urandom_fd);
    (*cont)++;
    n.low = (uint64_t)*cont;

    return n;

}

int
main(int argc, char *argv[])
{
    struct sockaddr_in sin;
    struct sockaddr sclient;
    socklen_t addrlen;
    nonce nonce;
    response response;
    int port = 9999;
    int fd;
    int sockfd;
    int cont = 0;
    int total = 0;
    int bytes;

    if (argc < 2) {
        usage();
    }

    if (argc == 3) {
        port = to_int(argv[2]);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        err(1, "socket failed");
    }
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(port);
    if(bind(sockfd, (struct sockaddr *)&sin, sizeof(sin)) < 0){
        err(1, "bind failed");
    }
    if(listen(sockfd, 100) < 0){ //como mucho 100 clientes en la cola
        err(1, "listen failed");
    }
    for(;;){
        addrlen = sizeof(sclient);
        fd = accept(sockfd, &sclient, &addrlen);
        if(fd < 0){
            err(1, "accept failed");
        }

        nonce = create_nonce(&cont);
        
        if (send(fd,&nonce,sizeof(nonce),0) != sizeof(nonce)) {
            err(EXIT_FAILURE,"send response failed");
        }

        while (total <= sizeof(response)) {
            bytes = recv(fd,&response,sizeof(response),0);

            if (bytes < 0) {
                err(EXIT_FAILURE,"recv response failed");
            }

            if (bytes == 0) {
                errx(EXIT_FAILURE,"connection closed prematurely");
            }

            total += bytes;
        }

        // Comprobar el timeout

        
        close(fd);
    }

    close(sockfd);

}