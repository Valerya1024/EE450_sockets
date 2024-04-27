#include "serverU.h"
#include "msg.h"
#include "util.h"
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

static struct sockaddr_in server_U_addr;

int main() {
    room_t* db = malloc(sizeof(room_t));
    SET_ADDR(server_U_addr, SERVER_U_PORT);
    printf(BACKEND_MSG_BOOT_UP, NAME_U, SERVER_U_PORT);
    // boot up server
    int sockfd = bootup_backend(db, SERVER_U_PORT, SERVER_M_UDP_PORT, U_FILE_PATH);
    // Bind socket
    if (BIND(sockfd, server_U_addr) < 0) {
        LOG_ERR("bind error");
    }
    printf(BACKEND_MSG_SEND_ROOM_STATUS, NAME_U);
    // handle quey/ reserve req
    handle_request(sockfd, db, NAME_U);
    destroy_room_db(db);
    close(sockfd);
}
