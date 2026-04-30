#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/evp.h>

static void
usage(void)
{
    errx(EXIT_FAILURE, "usage: ./authserver file [port]");
}

int
main(int argc, char *argv[])
{
    struct sockaddr_in sin;
    struct sockaddr_in sclient;
    socklen_t addrlen;
    nonce nonce;
    response response;
    int port = 9999;
    account *accounts;
    int naccounts;
    int fd;
    int sockfd;
    int cont = 0;
    char client_ip[INET_ADDRSTRLEN];
    unsigned char resp_buf[20 + 8 + 256];
    int ok;
    account *acc;
    unsigned char data[sizeof(nonce) + sizeof(response.T)];
    unsigned char hmac[EVP_MAX_MD_SIZE];
    unsigned int hmac_len;

    if (argc < 2) {
        usage();
    }

    accounts = read_accounts(argv[1], &naccounts);

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
        fd = accept(sockfd, (struct sockaddr *)&sclient, &addrlen);
        if(fd < 0){
            err(1, "accept failed");
        }

        inet_ntop(AF_INET, &sclient.sin_addr, client_ip, sizeof(client_ip));

        nonce = create_nonce(&cont);
        send_all(fd, &nonce, sizeof(nonce));
        alarm(TIMEOUT);
        recv_all(fd, resp_buf, sizeof(resp_buf));
        memcpy(response.r, resp_buf, 20);
        memcpy(&response.T, resp_buf + 20, sizeof(response.T));
        memcpy(response.login, resp_buf + 28, 256);

        ok = 1;

        if (check_timestamp(response.T) < 0) {
            ok = 0;
        }

        acc = NULL;
        if (ok) {
            for (int i = 0; i < naccounts; i++) {
                if (strncmp(accounts[i].login, response.login, sizeof(response.login)) == 0) {
                    acc = &accounts[i];
                    break;
                }
            }
            if (acc == NULL)
                ok = 0;
        }

        if (ok) {
            memcpy(data, &nonce, sizeof(nonce));
            memcpy(data + sizeof(nonce), &response.T, sizeof(response.T));

            compute_hmac(acc->key, sizeof(acc->key), data, sizeof(data), hmac, &hmac_len);

            if (hmac_len != sizeof(response.r) || memcmp(hmac, response.r, sizeof(response.r)) != 0)
                ok = 0;
        }

        if (ok)
            printf("SUCCESS, %s from %s\n", response.login, client_ip);
        else
            printf("FAILURE, %s from %s\n", response.login, client_ip);

        send_all(fd, ok ? "SUCCESS" : "FAILURE", 8);
        close(fd);
    }

    close(sockfd);

}