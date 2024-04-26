#include "serverS.h"
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

static struct sockaddr_in server_S_addr;

int main() {
    room_t* db = malloc(sizeof(room_t));
    SET_ADDR(server_S_addr, SERVER_S_PORT);
    printf(BACKEND_MSG_BOOT_UP, NAME_S, SERVER_S_PORT);
    int sockfd = bootup_backend(db, SERVER_S_PORT, SERVER_M_UDP_PORT, S_FILE_PATH);
    // Bind socket
    if (BIND(sockfd, server_S_addr) < 0) {
        LOG_ERR("bind error");
    }
    printf(BACKEND_MSG_SEND_ROOM_STATUS, NAME_S);
    handle_request(sockfd, db, NAME_S);
    destroy_room_db(db);
    close(sockfd);
}
