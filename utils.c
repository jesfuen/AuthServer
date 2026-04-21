#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/socket.h>

void
usage(void)
{
    errx(EXIT_FAILURE, "usage: ./authserver file [int]");
}

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

void
check_timestamp(uint64_t T)
{
    time_t now = time(NULL);
    if (now == (time_t)-1) {
        err(EXIT_FAILURE, "time failed");
    }
    uint64_t t_server = (uint64_t)now;
    if (T > t_server || (t_server - T) > 300) {
        errx(EXIT_FAILURE, "error: timestamp expired");
    }
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
