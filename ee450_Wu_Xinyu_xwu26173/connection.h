#define SERVER_M_UDP_PORT 44907

#define LOCALHOST "127.0.0.1"
#define SET_ADDR(addr, port) do { \
    addr.sin_family = AF_INET; \
    addr.sin_port = htons(port); \
    addr.sin_addr.s_addr = inet_addr(LOCALHOST); \
    memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));\
} while (0)

#define CREATE_TCP_SOCKET() socket(AF_INET, SOCK_STREAM, 0)
#define CREATE_UDP_SOCKET() socket(AF_INET, SOCK_DGRAM, 0)

#define SETSOCKOPT(sockfd) setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int))

#define BIND(sockfd, addr) bind(sockfd, (struct sockaddr *) &addr, sizeof(addr))

#define LISTEN(sockfd) listen(sockfd, 5)

#define CONNECT(sockfd, addr) connect(sockfd, (struct sockaddr *) &addr, sizeof(addr))

#define ACCEPT(sockfd, addr, addr_len_ptr) accept(sockfd, (struct sockaddr *) &addr, addr_len_ptr)

#define SEND(sockfd, msg) send(sockfd, msg, strlen(msg), 0)

#define RECV(sockfd, buf) recv(sockfd, buf, MAXLEN, 0)

#define SEND_TO(sockfd, msg, addr) sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr*) &addr, sizeof(addr))

#define RECV_FROM(sockfd, buf, addr, addr_len_pt) recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*) &addr, addr_len_pt)

int bootup_backend(room_t* db, int port, int mainport, char* fpath);

int handle_request(int sockfd, room_t* db, char* server_name);
