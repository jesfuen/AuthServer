#include <stdint.h>
#include <stddef.h>

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

void usage(void);
int to_int(char *str);
nonce create_nonce(int *cont);
void handle_alarm(int sig);
void send_all(int fd, const void *buf, size_t len);
void recv_all(int fd, void *buf, size_t len);
void check_timestamp(uint64_t T);
