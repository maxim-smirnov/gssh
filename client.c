#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <termios.h>
#include <pty.h>
//#include <util.h>
#include <errno.h>

#include "common.h"
#include "config.h"

#include "client.h"

int spawn_pty_server(int socket_fd)
{
    struct termios old_term, new_term;
    struct winsize winsize;
    char buffer[BUFSIZ];

    char is_found = 0;

    if (isatty(STDIN_FILENO)) {
        if (ioctl(STDIN_FILENO, TIOCGWINSZ, &winsize) < 0)
            perror("ioctl() error");
        else
            is_found = 1;
    }

    if (!is_found) {
        winsize.ws_col = 80;
        winsize.ws_row = 24;
    }

    log("winsize: %d:%d\n", winsize.ws_col, winsize.ws_row);

    if (write(socket_fd, (char *) &winsize, sizeof(struct winsize)) != sizeof(struct winsize)) {
        debug_print("cannot send winsize\n");
        return EXIT_FAILURE;
    }

    if (isatty(STDOUT_FILENO)) {
        if (tcgetattr(STDOUT_FILENO, &old_term) < 0) {
            perror("tcgetattr() error");
            return EXIT_FAILURE;
        }

        memcpy((void *) &new_term, (void *) &old_term, sizeof(new_term));

        new_term.c_iflag |= IGNPAR;
        new_term.c_iflag &= ~(ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXANY | IXOFF);
        new_term.c_lflag &= ~(ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHONL | IEXTEN);
        new_term.c_oflag &= ~(OPOST);

        new_term.c_cc[VMIN]  = 1;
        new_term.c_cc[VTIME] = 0;

        if (tcsetattr(STDOUT_FILENO, TCSADRAIN, &new_term) < 0) {
            perror("tcsetattr() error");
            return EXIT_FAILURE;
        }
    }

    while (1) {
        fd_set rset;
        int ret;

        FD_ZERO(&rset);
        FD_SET(STDIN_FILENO, &rset);
        FD_SET(socket_fd, &rset);

        ret = select(socket_fd + 1, &rset, NULL, NULL, &(struct timeval) { COMMAND_TIMEOUT, 0 });
        if (ret < 0) {
            if (errno == EINTR)
                continue;
            perror("select() error");
            break;
        }
        if (!ret) {
            printf("\n");
            log("timeout\n");
            break;
        }

        if (FD_ISSET(socket_fd, &rset)) {
            if ((ret = read(socket_fd, buffer, BUFSIZ)) > 0) {
                ret = write(STDOUT_FILENO, buffer, ret);
                if (ret <= 0) {
                    perror("write() error");
                    break;
                }
            } else {
                // fprintf(stderr, "crypto_recv() error\n");
                break;
            }
        }

        if (FD_ISSET(STDIN_FILENO, &rset)) {
            ret = read(STDIN_FILENO, buffer, BUFSIZ);
            if (ret == 0) {
                log("stdin eof\n");
                break;
            }
            if (ret < 0) {
                perror("read() error");
                break;
            }

            if (write(socket_fd, buffer, ret) != ret) {
                fprintf(stderr, "crypto_send() error\n");
            }
        }
    }

    if (isatty(STDOUT_FILENO)) {
        tcsetattr(STDOUT_FILENO, TCSADRAIN, &old_term);
    }

    log("connection has lost\n");

    return EXIT_SUCCESS;
}


int main() {
/*
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
*/
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
    return spawn_pty_server(sockfd);
}
