#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <limits.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

typedef struct
{
    char login[256];
    // Resto de campos key, nonce, timestamp...
} client;

typedef struct
{
    uint64_t high;
    uint64_t low;
} nonce;

int
to_int(char *str)
{
    long port;
    char *endptr;
    

    port = strtol(str,&endptr,10);
    if (port < 0 || port > INT_MAX || *endptr != '\0') {
        errx(EXIT_FAILURE, "error: bad value %s", str);
    }

    return (int)port;
}

void
usage()
{
    errx(EXIT_FAILURE,"usage: ./authserver file [int]");
}

nonce
create_nonce(int *cont) {
    // Parte pseudoaleatoria de 64 bits que se lee de /dev/urandom
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
    *cont++;
    n.low = (uint64_t)*cont;

    return n;

}

int
main(int argc, char *argv[])
{
    struct sockaddr_in sin;
    struct sockaddr sclient;
    socklen_t addrlen;
    int port = 9999;
    int fd;
    int sockfd;

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

        // Crear el nonce concatenando un numero aleatorio de 64 bits y el contador
        // Mandar el nonce
        

        close(fd);
    }

    close(sockfd);

}