#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>

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

    signal(SIGALRM,handle_alarm);

    for(;;){
        addrlen = sizeof(sclient);
        fd = accept(sockfd, &sclient, &addrlen);
        if(fd < 0){
            err(1, "accept failed");
        }

        nonce = create_nonce(&cont);
        send_all(fd, &nonce, sizeof(nonce));

        alarm(TIMEOUT);
        recv_all(fd, &response, sizeof(response));

        check_timestamp(response.T);

        // Calcular el HMAC-SHA1 con el nonce, T y key mapeada con el login
        

        // Mapear el login -> Ir leyendo hasta encontrar el usuario o leer y almacenar varios struct cliente con login y key

        // Usar hmac-sha1 guardada en utils.c
         
        close(fd);
    }

    close(sockfd);

}