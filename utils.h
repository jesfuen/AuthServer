#include <stdint.h>
#include <stddef.h>
#include <openssl/evp.h>

enum {
    MAX_PORT = 65535,
    TIMEOUT = 10,
};

typedef struct {
    unsigned char r[20];
    uint64_t T;
    char login[256];
} response;

typedef struct {
    uint64_t high;
    uint64_t low;
} nonce;

typedef struct {
    char login[256];
    unsigned char key[20];
} account;


int to_int(char *str);
nonce create_nonce(int *cont);
void handle_alarm(int sig);
void send_all(int fd, const void *buf, size_t len);
unsigned char hex_to_bytes(char c);
void recv_all(int fd, void *buf, size_t len);
int check_timestamp(uint64_t T);
account *read_accounts(const char *path, int *count);
void hash_sha1(unsigned char *hash, unsigned int *md_len,
               const unsigned char *pad, size_t pad_len,
               const unsigned char *data, size_t data_len);
void compute_hmac(const unsigned char *key, size_t key_len,
                  const unsigned char *data, size_t data_len,
                  unsigned char *out, unsigned int *out_len);
