#include "util.h"
#include "msg.h"
#include "connection.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

// boot up backend server
int bootup_backend(room_t* db, int port, int mainport, char* fpath) {
    struct sockaddr_in server_M_udp_addr;
    struct sockaddr_in this_addr;
    int sockfd;
    // Load rooms from file
    char room_str[MAXLEN];
    load_room(fpath, db, room_str);
    //print_room(db);
    
    // Set main addr
    SET_ADDR(server_M_udp_addr, SERVER_M_UDP_PORT);
    // Set this addr
    SET_ADDR(this_addr, port);

    // create socket
    if ((sockfd = CREATE_UDP_SOCKET()) < 0) {
        LOG_ERR("socket creation error");
    }

    //set opt
    if (SETSOCKOPT(sockfd) < 0) {
        LOG_ERR("set socket opt error");
    }

    // //connect
    // if (CONNECT(sockfd, server_M_udp_addr)) {
    //     LOG_ERR("connect error");
    // }

    // send room status
    if (SEND_TO(sockfd, room_str, server_M_udp_addr) < 0) {
        LOG_ERR("send room status error");
    }
    
    //printf("%d", ntohs(server_M_udp_addr.sin_port));

    // status sent, close connection
    close(sockfd);

    // create new socket for handling query / availability request
     if ((sockfd = CREATE_UDP_SOCKET()) < 0) {
        LOG_ERR("socket creation error");
    }

    //set opt
    if (SETSOCKOPT(sockfd) < 0) {
        LOG_ERR("set socket opt error");
    }

    return sockfd;
}

// handle query / availability request
int handle_request(int sockfd, room_t* db, char* server_name) {
    char buffer[MAXLEN];
    char msg[MAXLEN];
    struct sockaddr from;
    socklen_t addr_len = sizeof(struct sockaddr);
    int mode;
    int action;
    char room[MAXLEN];
    int res;
    
    while (TRUE) {
        RECV_FROM(sockfd, buffer, from, &addr_len);
        sscanf(buffer, "%d %d %s", &mode, &action, room);

        if (action == RESERVE) {
            //RECEIVE RESERVE REQ
            printf(BACKEND_MSG_RESERVE_REQ, server_name);
            // reserve room
            res = reserve_room(db, room);
            if (res == ERR_NOT_AVAILABLE) {
                //NOT AVAILABLE
                printf(BACKEND_MSG_RESERVE_FAIL, room);
                sprintf(msg, "%d", ERR_NOT_AVAILABLE);
                SEND_TO(sockfd, msg, from);
            } else if (res >= 0) {
                //RESERVED
                printf(BACKEND_MSG_RESERVE_SUC, room, res);
                sprintf(msg, "%d", res);
                SEND_TO(sockfd, msg, from);
            } else {
                // NO SUCH ROOM
                printf(BACKEND_MSG_RESERVE_NO_ROOM);
                sprintf(msg, "%d", ERR_NOT_FOUND);
                SEND_TO(sockfd, msg, from);
            }
            //SEND RESERVE RESPONSE
            if (res >= 0) {
                printf(BACKEND_MSG_RESERVE_RESP_UPDATED, server_name);
            } else {
                printf(BACKEND_MSG_RESERVE_RESP, server_name);
            }
        } else {
            //RECEIVE AVAILABILITY REQ
            printf(BACKEND_MSG_AVAILABILITY_REQ, server_name);
            res = lookup_room(db, room);
            if (res > 0) {
                // Available
                printf(BACKEND_MSG_AVAILABLE, room);
            } else if (res == ERR_NOT_FOUND) {
                // NO SUCH ROOM
                printf(BACKEND_MSG_AVAILABILITY_NO_ROOM);
            } else {
                // not available
                printf(BACKEND_MSG_NOT_AVAILABLE, room);
            }
            sprintf(msg, "%d", res);
            SEND_TO(sockfd, msg, from);
            //SENT AVAIL RESPONSE
            printf(BACKEND_MSG_AVAILABILITY_RESP, server_name);
        }
    }
}