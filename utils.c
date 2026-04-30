#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/socket.h>
#include <openssl/evp.h>

int
to_int(char *str)
{
    long port;
    char *endptr;

    port = strtol(str, &endptr, 10);
    if (port < 1 || port > MAX_PORT || *endptr != '\0') {
        errx(EXIT_FAILURE, "error: bad value %s", str);
    }

    return (int)port;
}

nonce
create_nonce(int *cont)
{
    nonce n;
    int urandom_fd;

    urandom_fd = open("/dev/urandom", O_RDONLY);
    if (urandom_fd < 0) {
        err(EXIT_FAILURE, "error: open /dev/urandom failed");
    }

    if (read(urandom_fd, &n.high, sizeof(n.high)) != sizeof(n.high)) {
        err(EXIT_FAILURE, "error: read /dev/urandom failed");
    }

    close(urandom_fd);
    (*cont)++;
    n.low = (uint64_t)*cont;

    return n;
}

void
handle_alarm(int sig)
{
    (void)sig;
}

void
send_all(int fd, const void *buf, size_t len)
{
    size_t total = 0;
    ssize_t bytes;

    while (total < len) {
        bytes = send(fd, (const char *)buf + total, len - total, 0);
        if (bytes < 0)
            err(EXIT_FAILURE, "error: send failed");
        total += (size_t)bytes;
    }
}

int
check_timestamp(uint64_t T)
{
    time_t now = time(NULL);
    if (now == (time_t)-1) {
        err(EXIT_FAILURE, "time failed");
    }
    uint64_t t_server = (uint64_t)now;
    if (T > t_server || (t_server - T) > 300)
        return -1;
    return 0;
}

unsigned char
hex_to_bytes(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return c - 'A' + 10;
}

account *
read_accounts(const char *path, int *count)
{
    FILE *f;
    account *accounts = NULL;
    account *tmp;
    char line[512], *login, *hexkey, *saveptr;
    int n = 0;

    f = fopen(path, "r");
    if (f == NULL)
        err(EXIT_FAILURE, "error: cannot open %s", path);

    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\n")] = '\0';

        login = strtok_r(line, ":", &saveptr);
        hexkey = strtok_r(NULL, ":", &saveptr);
        if (login == NULL || hexkey == NULL)
            continue;

        tmp = realloc(accounts, (n + 1) * sizeof(account));
        if (tmp == NULL)
            err(EXIT_FAILURE, "error: realloc failed");
        accounts = tmp;

        strncpy(accounts[n].login, login, 255);
        accounts[n].login[255] = '\0';

        for (int i = 0; i < 20; i++)
            accounts[n].key[i] = (hex_to_bytes(hexkey[2*i]) << 4) | hex_to_bytes(hexkey[2*i+1]);
        n++;
    }

    fclose(f);
    *count = n;
    return accounts;
}

void
recv_all(int fd, void *buf, size_t len)
{
    size_t total = 0;
    ssize_t bytes;

    while (total < len) {
        bytes = recv(fd, (char *)buf + total, len - total, 0);
        if (bytes < 0) {
            if (errno == EINTR) {
                alarm(0);
                errx(EXIT_FAILURE, "error: timeout!");
            }
            err(EXIT_FAILURE, "error: recv failed");
        }
        if (bytes == 0) {
            alarm(0);
            errx(EXIT_FAILURE, "error: connection closed prematurely");
        }
        total += (size_t)bytes;
    }
    alarm(0);
}

void
hash_sha1(unsigned char *hash, unsigned int *md_len,
          const unsigned char *pad, size_t pad_len,
          const unsigned char *data, size_t data_len)
{
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx)
        errx(EXIT_FAILURE, "EVP_MD_CTX_new failed");
    if (!EVP_DigestInit(mdctx, EVP_sha1()))
        errx(EXIT_FAILURE, "EVP_DigestInit failed");
    if (pad && !EVP_DigestUpdate(mdctx, pad, pad_len))
        errx(EXIT_FAILURE, "EVP_DigestUpdate failed");
    if (data && !EVP_DigestUpdate(mdctx, data, data_len))
        errx(EXIT_FAILURE, "EVP_DigestUpdate failed");
    if (!EVP_DigestFinal_ex(mdctx, hash, md_len))
        errx(EXIT_FAILURE, "EVP_DigestFinal_ex failed");
    EVP_MD_CTX_free(mdctx);
}

void
compute_hmac(const unsigned char *key, size_t key_len,
             const unsigned char *data, size_t data_len,
             unsigned char *out, unsigned int *out_len)
{
    enum { PADSIZE = 64 };
    unsigned char k_ipad[PADSIZE], k_opad[PADSIZE];
    unsigned char inner[EVP_MAX_MD_SIZE];
    unsigned int inner_len;
    int i;

    memset(k_ipad, 0, PADSIZE);
    memset(k_opad, 0, PADSIZE);

    if (key_len <= PADSIZE) {
        memcpy(k_ipad, key, key_len);
        memcpy(k_opad, key, key_len);
    } else {
        hash_sha1(k_ipad, &inner_len, NULL, 0, key, key_len);
        memcpy(k_opad, k_ipad, inner_len);
    }

    for (i = 0; i < PADSIZE; i++) {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5c;
    }

    hash_sha1(inner, &inner_len, k_ipad, PADSIZE, data, data_len);
    hash_sha1(out, out_len, k_opad, PADSIZE, inner, inner_len);
}

