#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <limits.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct client
{
    char login[256];
    // Resto de campos key, nonce, timestamp...
};

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

int
main(int argc, char *argv[])
{
    struct sockaddr_in sin;
    struct sockaddr sclient;
    socklen_t addrlen;
    int port = 9999;
    int fd;
    int sockfd;
    int cont;

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