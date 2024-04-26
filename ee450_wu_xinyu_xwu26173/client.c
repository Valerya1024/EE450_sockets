#include "util.h"
#include "connection.h"
#include "msg.h"
#include "serverM.h"
#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

static struct sockaddr_in server_M_tcp_addr;
static struct sockaddr_in my_addr;
int main() {
    size_t size = MAXLEN;
    int sockfd;
    int mode;
    char* username = malloc(size);
    char* pwd = malloc(size);
    //char* room;

    //bootup
    printf(CLIENT_MSG_BOOT_UP);
    sockfd = bootup_C();

    mode = login(sockfd, username, pwd);
    while (TRUE) {
        send_action(sockfd, mode, username);
    }
    free(username);
    free(pwd);
    close(sockfd);
}

void send_action(int sockfd, int mode, char* username) {
    printf(CLIENT_MSG_ENTER_ROOM);
    
    char buffer[MAXLEN];
    char room[MAXLEN];
    scanf("%s", room);
    int action;
    printf(CLIENT_MSG_ENTER_ACTION);
    scanf("%s", buffer);
    if (strcmp(buffer, "Reservation") == 0) {
        action = RESERVE;
    } else {
        action = QUERY;
    }
    memset(buffer, '\0', MAXLEN);
    sprintf(buffer, "%d %d %s", mode, action, room);
    //printf("send req: %s\n", buffer);
    if (SEND(sockfd, buffer) < 0) {
        LOG_ERR("send error");
    }
    if (action == QUERY) {
        printf(CLIENT_MSG_AVAILABILITY_REQ, username);
    } else {
        printf(CLIENT_MSG_RESERVE_REQ, username);
    }

    memset(buffer, '\0', MAXLEN);
    if (RECV(sockfd, buffer) < 0) {
        LOG_ERR("recv error");
    }
    int res;
    //printf("receive buffer: %s", buffer);
    sscanf(buffer, "%d", &res);
    if (res == ERR_NOT_ACCESSIBLE) { // guest reserve
        printf(GUEST_MSG_RESERVE_RESP);
    } else if (res == ERR_NOT_FOUND) { // no such room
        if (action == QUERY) {
            printf(CLIENT_MSG_AVAILABILITY_NO_ROOM, ntohs(my_addr.sin_port));
        } else {
            printf(MEMBER_MSG_RESERVE_NO_ROOM, ntohs(my_addr.sin_port));
        }
    } else if (res == ERR_NOT_AVAILABLE) { // count is 0
        if (action == QUERY) {
            printf(CLIENT_MSG_NOT_AVAILABLE_RESP, ntohs(my_addr.sin_port));
        } else {
            printf(MEMBER_MSG_RESERVE_FAIL, ntohs(my_addr.sin_port));
        }
    } else { //success
        if (action == QUERY) {
            printf(CLIENT_MSG_AVAILABLE_RESP, ntohs(my_addr.sin_port));
        } else {
            printf(MEMBER_MSG_RESERVE_SUC, ntohs(my_addr.sin_port), room);
        }
    }
}

int bootup_C() {
    int sockfd;

    // Set dest addr
    SET_ADDR(server_M_tcp_addr, SERVER_M_TCP_PORT);

    // create socket
    if ((sockfd = CREATE_TCP_SOCKET()) < 0) {
        LOG_ERR("socket creation error");
    }
    // set opt
    if (SETSOCKOPT(sockfd) < 0) {
        LOG_ERR("set socket opt error");
    }
    return sockfd;
}

int login(int sockfd, char* username,char* pwd) {
    int mode = 0;
    size_t size = MAXLEN;
    int res = 0;

    //connect
    if (CONNECT(sockfd, server_M_tcp_addr) < 0) {
        LOG_ERR("connect error");
    }

    // get my addr
    socklen_t addr_len = sizeof(struct sockaddr);
    if (getsockname(sockfd, (struct sockaddr*)&my_addr, (socklen_t *)&addr_len) < 0) {
        LOG_ERR("get my addr error");
    }
    //printf("port number %d\n", ntohs(my_addr.sin_port));

    while (res <= 0) {
        //input name and pwd
        getline(&username, &size, stdin);
        username[strlen(username)-1] = '\0';
        char usrtname_encrypted[MAXLEN];
        strcpy(usrtname_encrypted, username);
        encrypt(usrtname_encrypted);
        printf(CLIENT_MSG_ENTER_PASSWORD);
        getline(&pwd, &size, stdin);
        pwd[strlen(pwd)-1] = '\0';
        encrypt(pwd);

        if (strlen(pwd) == 0) {
            //GUEST MODE
            mode = GUEST_MODE;
        } else {
            //MEMBER MODE
            mode = MEMBER_MODE;
        }
        
        char msg[MAXLEN];
        memset(msg, '\0', MAXLEN);
        strcpy(msg, usrtname_encrypted);
        strcat(msg, " ");
        strcat(msg, pwd);

        //send login req
        if (SEND(sockfd, msg) < 0) {
            LOG_ERR("send error");
        }

        if (mode == GUEST_MODE) {
            printf(GUEST_MSG_GUEST_REQ, username, my_addr.sin_port);
        } else {
            printf(MEMBER_MSG_LOGIN_REQ, username);
        }

        //receive response
        char buffer[MAXLEN];
        memset(buffer, '\0', MAXLEN);
        if (RECV(sockfd, buffer) < 0) {
            LOG_ERR("recv error");
        }
        sscanf(buffer, "%d", &res);

        if (res == LOGIN_SUC) {
            printf(MEMBER_MSG_LOGIN_SUC, username);
            mode = MEMBER_MODE;
        } else if (res == LOGIN_GUEST) {
            printf(GUEST_MSG_RECEIVE_RESP, username);
            mode = GUEST_MODE;
        } else if (res == LOGIN_FAIL_USERNAME) {
            printf(MEMBER_MSG_LOGIN_FAIL_USERNAME);
        } else {
            printf(MEMBER_MSG_LOGIN_FAIL_PWD);
        }
    }
    
    return mode;
}
