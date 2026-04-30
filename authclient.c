#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/evp.h>

static void
usage(void)
{
    errx(EXIT_FAILURE, "usage: ./authclient login key server_addr port");
}

int
main(int argc, char *argv[])
{
    struct sockaddr_in sin;
    int sockfd;
    int port;
    char *serverip;
    char *login;
    char *hexkey;
    unsigned char key[20];
    nonce nonce;
    uint64_t t;
    unsigned char data[sizeof(nonce) + sizeof(t)];
    unsigned char hmac[EVP_MAX_MD_SIZE];
    unsigned int hmac_len;
    unsigned char send_buf[20 + 8 + 256];
    char result[8];

    if (argc != 5) {
        usage();
    }

    login    = argv[1];
    hexkey   = argv[2];
    serverip = argv[3];
    port     = to_int(argv[4]);

    for (int i = 0; i < 20; i++)
        key[i] = (hex_to_bytes(hexkey[2*i]) << 4) | hex_to_bytes(hexkey[2*i+1]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        err(1, "socket failed");

    sin.sin_family      = AF_INET;
    sin.sin_addr.s_addr = inet_addr(serverip);
    sin.sin_port        = htons(port);

    if (connect(sockfd, (struct sockaddr *)&sin, sizeof(sin)) == -1)
        err(1, "connect failed");

    recv_all(sockfd, &nonce, sizeof(nonce));

    t = (uint64_t)time(NULL);
    if (t == (uint64_t)-1)
        err(EXIT_FAILURE, "time failed");

    memcpy(data, &nonce, sizeof(nonce));
    memcpy(data + sizeof(nonce), &t, sizeof(t));

    compute_hmac(key, sizeof(key), data, sizeof(data), hmac, &hmac_len);

    memcpy(send_buf, hmac, 20);
    memcpy(send_buf + 20, &t, sizeof(t));
    memset(send_buf + 28, 0, 256);
    strncpy((char *)(send_buf + 28), login, 255);

    send_all(sockfd, send_buf, sizeof(send_buf));

    recv_all(sockfd, result, sizeof(result));
    printf("%.*s\n", (int)sizeof(result), result);

    close(sockfd);
    return 0;
}
