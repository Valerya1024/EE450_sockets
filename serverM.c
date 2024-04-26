#include "util.h"
#include "connection.h"
#include "msg.h"
#include "serverM.h"
#include "serverS.h"
#include "serverD.h"
#include "serverU.h"
#include "extra.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>

static struct sockaddr_in server_S_addr;
static struct sockaddr_in server_D_addr;
static struct sockaddr_in server_U_addr;
static struct sockaddr_in server_M_tcp_addr;
static struct sockaddr_in server_M_udp_addr;

static member_t* db;
static room_t* room_db;

#ifdef EXTRA
#define M_FILE_PATH "./member_extra.txt"
#else
#define M_FILE_PATH "./member.txt"
#endif

int main() {
    //bootup
    db = malloc(sizeof(member_t)); //member db linked list
    room_db = malloc(sizeof(room_t));
    int sockfd_tcp, sockfd_udp; // socket fd

    bootup_M();
    printf(MAIN_MSG_BOOT_UP);
    sockfd_udp = bootup_M_udp();
    sockfd_tcp = bootup_M_tcp(sockfd_udp);
    //printf("%u %u %d\n", ntohs(server_M_tcp_addr.sin_port), SERVER_M_TCP_PORT, sockfd);

    destroy_member_db(db);
    destroy_room_db(room_db);
    close(sockfd_tcp);
    close(sockfd_udp);
}

void bootup_M() {
    // Load members from file
    load_member(M_FILE_PATH, db);
    //print_member(db);

    // Set dest addr
    SET_ADDR(server_S_addr, SERVER_S_PORT);
    SET_ADDR(server_U_addr, SERVER_U_PORT);
    SET_ADDR(server_D_addr, SERVER_D_PORT);
    // Set this addr
    SET_ADDR(server_M_tcp_addr, SERVER_M_TCP_PORT);
    SET_ADDR(server_M_udp_addr, SERVER_M_UDP_PORT);
}

int bootup_M_tcp(int sockfd_udp) {
    int sockfd, new_fd;

    /**************************TCP*********************************/
    struct sigaction sa;
    struct sockaddr_in from_addr;
    socklen_t sin_size = sizeof(struct sockaddr);

    // create socket
    if ((sockfd = CREATE_TCP_SOCKET()) < 0) {
        LOG_ERR("socket creation error");
    }
    // set opt
    if (SETSOCKOPT(sockfd) < 0) {
        LOG_ERR("set socket opt error");
    }
    // Bind socket
    if (BIND(sockfd, server_M_tcp_addr) < 0) {
        LOG_ERR("bind error");
    }
    // Lsiten
    if (LISTEN(sockfd) < 0) {
        LOG_ERR("listen error");
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    // accept loop
    while(TRUE) { // main accept() loop
        new_fd = ACCEPT(sockfd, from_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        //printf("server: got connection from %d\n", from_addr.sin_port);

        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener
            // if (send(new_fd, "Hello, world!", 13, 0) == -1) {
            //     perror("send");
            // }
            char unencrypt_username[MAXLEN];
            int res = -1;
            while (res < 0) {
                res = handle_login(new_fd, unencrypt_username);
            }
            while (TRUE) {
                handle_action(new_fd, sockfd_udp, unencrypt_username);
            }
            close(new_fd); //done, exit
            exit(0);
        }
        close(new_fd); // parent doesn't need this
    }

    return sockfd;
}

int handle_login(int sockfd, char* unencrypt_username) {
    char buffer[MAXLEN];
    char username[MAXLEN];
    char pwd[MAXLEN];
    memset(buffer, '\0', MAXLEN);
    memset(username, '\0', MAXLEN);
    memset(pwd, '\0', MAXLEN);
    int res;
    if (RECV(sockfd, buffer) < 0) {
        LOG_ERR("receive error");
    }

    if (strlen(buffer) == 0) { //client exited, close connection and exit
        close(sockfd);
        exit(0);
    }
    
    sscanf(buffer, "%s %s", username, pwd);
    strcpy(unencrypt_username, username);
    unencrypt(unencrypt_username);
    //GUEST MODE
    if (strlen(pwd) == 0) {
        printf(MAIN_MSG_GUEST_REQ, unencrypt_username, SERVER_M_TCP_PORT, unencrypt_username);
        res = LOGIN_GUEST;
    } else { // MEMBER
        printf(MAIN_MSG_LOGIN_REQ, unencrypt_username, SERVER_M_TCP_PORT);
        res = lookup_member(db, username, pwd);
    }
    sprintf(buffer, "%d", res);
    if (SEND(sockfd, buffer) < 0) {
        LOG_ERR("send error");
    }
    if (res == LOGIN_GUEST) {
        printf(MAIN_MSG_GUEST_RESP);
    } else {
        printf(MAIN_MSG_LOGIN_RESP);
    }
    return res;
}

void handle_action(int sockfd, int udp_sockfd, char* username) {
    char buffer[MAXLEN];
    char room[MAXLEN];
    char server[MAXLEN];
    int mode = 0;
    int action = 0;
    int res = 0;
    memset(buffer, '\0', MAXLEN);
    memset(room, '\0', MAXLEN);
    memset(server, '\0', MAXLEN);
    struct sockaddr_in target_addr;
    struct sockaddr from;
    socklen_t addr_len = sizeof(struct sockaddr);

    if (RECV(sockfd, buffer) < 0) {
        LOG_ERR("receive error");
    }

    if (strlen(buffer) == 0) { //client exited, close connection and exit
        close(sockfd);
        exit(0);
    }
    
    sscanf(buffer, "%d %d %s", &mode, &action, room);
    strcpy(server, room);
    server[1] = '\0';

    if (action == QUERY) {
        printf(MAIN_MSG_AVAILABILITY_REQ, room, username, SERVER_M_TCP_PORT);
    } else {
        printf(MAIN_MSG_RESERVE_REQ, room, username, SERVER_M_TCP_PORT);
    }

    if (mode == GUEST_MODE && action == RESERVE) {
        // NOT ACCESSIBLE
        printf(MAIN_MSG_RESERVE_GUEST, username);
        res = ERR_NOT_ACCESSIBLE;
        memset(buffer, '\0', MAXLEN);
        sprintf(buffer, "%d", res);
        if (SEND(sockfd, buffer) < 0) {
            LOG_ERR("send error");
        }
        printf(MAIN_MSG_RESERVE_ERROR_RESP);
    }

    //printf("receive req: %s %s\n", buffer, server);
    if (strcmp(server, "S") == 0) {
        target_addr = server_S_addr;
    } else if (strcmp(server, "D") == 0) {
        target_addr = server_D_addr;
    } else if (strcmp(server, "U") == 0) {
        target_addr = server_U_addr;
    } else {
        sprintf(buffer, "%d", ERR_NOT_FOUND);
        if (SEND(sockfd, buffer) < 0) {
            LOG_ERR("send error");
        }
        printf(MAIN_MSG_SEND_RESERVE_RESP);
        
        return;
    }

    if (action == QUERY) {
        //QUERY
        printf(MAIN_MSG_AVAILABILITY_FORWARD_REQ, server);
        //send to
        //printf("udp socket fd: %d, target port%d\n", udp_sockfd, target_addr.sin_port);
        if (SEND_TO(udp_sockfd, buffer, target_addr) < 0) {
            LOG_ERR("send error");
        }
        //recv from
        memset(buffer, '\0', MAXLEN);
        if (RECV_FROM(udp_sockfd, buffer, from, &addr_len) < 0) {
            LOG_ERR("receive error");
        }

        sscanf(buffer, "%d", &res);

        printf(MAIN_MSG_RECEIVE_AVAILABILITY_RESP, server, SERVER_M_UDP_PORT);
        sprintf(buffer, "%d", res);
        if (SEND(sockfd, buffer) < 0) {
            LOG_ERR("send error");
        }
        printf(MAIN_MSG_SEND_AVAILABILITY_RESP);
    } else {
        //RESERVE
        printf(MAIN_MSG_RESERVE_FORWARD_REQ, server);
        //send to
        if (SEND_TO(udp_sockfd, buffer, target_addr) < 0) {
            LOG_ERR("send error");
        }
        //recv from
        memset(buffer, '\0', MAXLEN);
        if (RECV_FROM(udp_sockfd, buffer, from, &addr_len) < 0) {
            LOG_ERR("receive error");
        }

        sscanf(buffer, "%d", &res);

        if (res >= 0) {
            printf(MAIN_MSG_RECEIVE_RESERVE_RESP_UPDATED, server, SERVER_M_UDP_PORT);
            reserve_room(room_db, room);
            printf(MAIN_MSG_ROOM_UPDATED, room);
        } else {
            printf(MAIN_MSG_RECEIVE_RESERVE_RESP, server, SERVER_M_UDP_PORT);
        }
        sprintf(buffer, "%d", res);
        if (SEND(sockfd, buffer) < 0) {
            LOG_ERR("send error");
        }
        printf(MAIN_MSG_SEND_RESERVE_RESP);
    }
    
}

void sigchld_handler(int s) {
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

int bootup_M_udp() {

    /***************************UDP*******************************/
    
    int sockfd;
    char buffer[MAXLEN];
    int received_bytes;
    struct sockaddr from;
    socklen_t addr_len = sizeof(struct sockaddr);

    //printf("%d", ntohs(server_M_udp_addr.sin_port));

    // create socket
    if ((sockfd = CREATE_UDP_SOCKET()) < 0) {
        LOG_ERR("socket creation error");
    }
    //printf("udp socket fd: %d\n", sockfd);
    // set opt
    if (SETSOCKOPT(sockfd) < 0) {
        LOG_ERR("set socket opt error");
    }
    // Bind socket
    if (BIND(sockfd, server_M_udp_addr) < 0) {
        LOG_ERR("bind error");
    }

    for (int i = 0; i<3;i++){
        // recv from
        if ((received_bytes = RECV_FROM(sockfd, buffer, from, &addr_len)) < 0) {
            LOG_ERR("receive error");
        }

        buffer[received_bytes] = '\0';
        //printf("%s", buffer);
        load_room_str(room_db, buffer);
        buffer[1] = '\0';
        printf(MAIN_MSG_RECEIVE_ROOM_STATUS, buffer, SERVER_M_UDP_PORT);

    }

    //print_room(room_db);

    return sockfd;
}
