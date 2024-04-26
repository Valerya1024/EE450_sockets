#pragma once

#define SERVER_M_TCP_PORT 45907
#define SERVER_M_UDP_PORT 44907
#define SERVER_S_PORT 41907

void bootup_M();

int bootup_M_tcp(int udp_sockfd);

int bootup_M_udp();

void sigchld_handler(int s);

int handle_login(int sockfd, char* unencrypt_username);

void handle_action(int sockfd, int udp_sockfd, char* username);
