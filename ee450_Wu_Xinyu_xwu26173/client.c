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
    int sockfd;
    int mode;
    char* username = malloc(MAXLEN);
    char* pwd = malloc(MAXLEN);

    //bootup msg
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

// send availablity / query request
void send_action(int sockfd, int mode, char* username) {
    // user input room code
    char buffer[MAXLEN];
    char room[MAXLEN];
    while (TRUE) {
        printf(CLIENT_MSG_ENTER_ROOM);
        memset(buffer, '\0', MAXLEN);
        scanf("%s", room);
        // invalid input handling
        if (strlen(room) > INPUT_MAXLEN) {
            printf(CLIENT_INVALID_ROOM_INPUT, INPUT_MAXLEN);
        } else {
            break;
        }
    }

    // user input action ()
    int action;
    while (TRUE) {
        printf(CLIENT_MSG_ENTER_ACTION);
        memset(buffer, '\0', MAXLEN);
        scanf("%s", buffer);
        if (strcmp(buffer, "Reservation") == 0) {
            action = RESERVE;
            break;
        } else if (strcmp(buffer, "Availability") == 0) {
            action = QUERY;
            break;
        } else {
            // invalid input handling
            printf(CLIENT_INVALID_ACTION_INPUT);
        }
    }
    
    // Prepare message to send
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

    // Receive response
    memset(buffer, '\0', MAXLEN);
    if (RECV(sockfd, buffer) < 0) {
        LOG_ERR("recv error");
    }
    // extract status code from response
    int res;
    //printf("receive buffer: %s", buffer);
    sscanf(buffer, "%d", &res);
    if (res == ERR_NOT_ACCESSIBLE) { 
        // guest reserve
        printf(GUEST_MSG_RESERVE_RESP);
    } else if (res == ERR_NOT_FOUND) { 
        // no such room
        if (action == QUERY) {
            printf(CLIENT_MSG_AVAILABILITY_NO_ROOM, ntohs(my_addr.sin_port));
        } else {
            printf(MEMBER_MSG_RESERVE_NO_ROOM, ntohs(my_addr.sin_port));
        }
    } else if (res == ERR_NOT_AVAILABLE) { 
        // count is 0, not available
        if (action == QUERY) {
            printf(CLIENT_MSG_NOT_AVAILABLE_RESP, ntohs(my_addr.sin_port));
        } else {
            printf(MEMBER_MSG_RESERVE_FAIL, ntohs(my_addr.sin_port));
        }
    } else { 
        if (action == QUERY) {
            // Available
            printf(CLIENT_MSG_AVAILABLE_RESP, ntohs(my_addr.sin_port));
        } else {
            // Reserve success
            printf(MEMBER_MSG_RESERVE_SUC, ntohs(my_addr.sin_port), room);
        }
    }
}

// boot up client, create socket
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

// send login request
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
        //input username
        while (TRUE) {
            memset(username, '\0', MAXLEN);
            printf(CLIENT_MSG_ENTER_USERNAME);
            getline(&username, &size, stdin);
            username[strlen(username)-1] = '\0';
            
            // invalid input handling
            if (strlen(username) == 0 || strlen(username) > INPUT_MAXLEN) {
                printf(CLIENT_INVALID_USERNAME_INPUT, INPUT_MAXLEN);
            } else {
                // contain space
                int space = FALSE;
                for (int i = 0; i < strlen(username); i++) {
                    if (username[i] == ' ') space = TRUE;
                }
                if (space) {
                    printf(CLIENT_INVALID_INPUT_SPACE);
                } else {
                    break;
                }
            }
        }
        
        // get encrypted username
        char username_encrypted[MAXLEN];
        strcpy(username_encrypted, username);
        encrypt(username_encrypted);

        //input pwd
        while (TRUE) {
            printf(CLIENT_MSG_ENTER_PASSWORD);
            getline(&pwd, &size, stdin);
            pwd[strlen(pwd)-1] = '\0';
            // handle invalid input
            if (strlen(pwd) > INPUT_MAXLEN) {
                printf(CLIENT_INVALID_PWD_INPUT, INPUT_MAXLEN);
            } else {
                int space = FALSE;
                for (int i = 0; i < strlen(pwd); i++) {
                    if (pwd[i] == ' ') space = TRUE;
                }
                if (space) {
                    printf(CLIENT_INVALID_INPUT_SPACE);
                } else {
                    break;
                }
            }
        }
        //get encrypted pwd
        encrypt(pwd);

        if (strlen(pwd) == 0) {
            //GUEST MODE
            mode = GUEST_MODE;
        } else {
            //MEMBER MODE
            mode = MEMBER_MODE;
        }
        
        // prepare login request message
        char msg[MAXLEN];
        memset(msg, '\0', MAXLEN);
        strcpy(msg, username_encrypted);
        strcat(msg, " ");
        strcat(msg, pwd);

        //send login req
        if (SEND(sockfd, msg) < 0) {
            LOG_ERR("send error");
        }

        if (mode == GUEST_MODE) {
            printf(GUEST_MSG_GUEST_REQ, username, ntohs(my_addr.sin_port));
        } else {
            printf(MEMBER_MSG_LOGIN_REQ, username);
        }

        //receive login response
        char buffer[MAXLEN];
        memset(buffer, '\0', MAXLEN);
        if (RECV(sockfd, buffer) < 0) {
            LOG_ERR("recv error");
        }
        sscanf(buffer, "%d", &res);

        if (res == LOGIN_SUC) {
            // login success
            printf(MEMBER_MSG_LOGIN_SUC, username);
            mode = MEMBER_MODE;
        } else if (res == LOGIN_GUEST) {
            // as a guest
            printf(GUEST_MSG_RECEIVE_RESP, username);
            mode = GUEST_MODE;
        } else if (res == LOGIN_FAIL_USERNAME) {
            // login fail, no such user
            printf(MEMBER_MSG_LOGIN_FAIL_USERNAME);
        } else {
            // login passwor not match
            printf(MEMBER_MSG_LOGIN_FAIL_PWD);
        }
    }
    
    return mode;
}
