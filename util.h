#pragma once

#define ERR_NOT_FOUND -1
#define ERR_NOT_AVAILABLE -2
#define ERR_PWD -3
#define ERR_NOT_ACCESSIBLE -4
#define MAXLEN 1024
#define INPUT_MAXLEN 50
#define TRUE 1
#define FALSE 0

#define LOGIN_SUC 10
#define LOGIN_GUEST 11
#define LOGIN_FAIL_USERNAME -10
#define LOGIN_FAIL_PWD -11

#define MEMBER_MODE 1
#define GUEST_MODE 2
#define QUERY 3
#define RESERVE 4

#define ENCRYPT_SHIFT 3
#define NUM_BACKEND 3

#define LOG_ERR(msg) do {\
    perror(msg);\
    exit(1);\
} while(0)

typedef struct __room_t {
    char room_code[10];
    int count;
    struct __room_t* next;
} room_t;

typedef struct __member_t {
    char username[20];
    char pwd[20];
    struct __member_t* next;
} member_t;

int lookup_room(room_t* db, char* target);

void insert_room(room_t* db, char* room_code, int count);

void load_room(char* file_path, room_t* db, char* room_str);

void load_room_str(room_t* db, char* room_str);

void print_room(room_t* db);

int reserve_room(room_t* db, char* room_code);

void destroy_room_db(room_t* db);

int lookup_member(member_t* db, char* target_name, char* target_pwd);

void insert_member(member_t* db, char* username, char* pwd);

void destroy_member_db(member_t* db);

void load_member(char* file_path, member_t* db);

void print_member(member_t* db);

void encrypt(char* str);

void unencrypt(char* str);