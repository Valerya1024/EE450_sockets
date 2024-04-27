#include "util.h"
#include "extra.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// look up room by room code
int lookup_room(room_t* db, char* target) {
    room_t * node = db->next;
    while (node != NULL) {
        if (strcmp(target, node->room_code) == 0) {
            if (node->count == 0) {
                return ERR_NOT_AVAILABLE;
            }
            return node->count;
        }
        node = node->next;
    }
    return ERR_NOT_FOUND;
}

// add room to linked list
void insert_room(room_t* db, char* room_code, int count) {
    room_t * new_room = malloc(sizeof(room_t));
    strcpy(new_room->room_code, room_code);
    new_room->count = count;
    new_room->next = db->next;
    db->next = new_room;
}

//load room from file to a linked list
void load_room(char* file_path, room_t* db, char* room_str) {
    FILE *fp;
    char row[MAXLEN];
    char room[MAXLEN];
    int num;
    strcpy(room_str, "");

    fp = fopen(file_path,"r");

    while (feof(fp) != TRUE)
    {
        fgets(row, MAXLEN, fp);
        //printf("Row: %s", row);
        strcat(room_str, row);
        if (strlen(row) == 0) {break;}

        sscanf(row, "%[^,], %d", room, &num);

        insert_room(db, room, num);

        strcpy(row, "");
    }

    fclose(fp);
}

// load room from string to a linked list
void load_room_str(room_t* db, char* room_str) {
    int offset = 0;
    char room[MAXLEN];
    int num;
    for ( int pos; sscanf(room_str + offset, "%[^,], %d\n%n", room, &num, &pos) == 2; offset += pos) {
        insert_room(db, room, num);
    }
}

// traverse and print
void print_room(room_t* db) {
    db = db->next;
    while (db != NULL) {
        printf("%s %d\n", db->room_code, db->count);
        db = db->next;
    }
}

// look up and try reserve room
int reserve_room(room_t* db, char* room_code) {
    room_t * node = db->next;
    while (node != NULL) {
        if (strcmp(room_code, node->room_code) == 0) {
            if (node->count == 0) {
                return ERR_NOT_AVAILABLE;
            }
            node->count -= 1;
            return node->count;
        }
        node = node->next;
    }
    return ERR_NOT_FOUND;
}

// free db
void destroy_room_db(room_t* db) {
    while (db != NULL) {
        room_t * node = db;
        db = db->next;
        free(node);
    }
}

// find member by name and password
int lookup_member(member_t* db, char* target_name, char* target_pwd) {
    member_t * node = db->next;
    while (node != NULL) {
        if (strcmp(target_name, node->username) == 0) {
            if (strcmp(target_pwd, node->pwd) == 0) {
                return LOGIN_SUC;
            }
            return LOGIN_FAIL_PWD;
        }
        node = node->next;
    }
    return LOGIN_FAIL_USERNAME;
}

// add member to linked list
void insert_member(member_t* db, char* username, char* pwd) {
    member_t * new_member = malloc(sizeof(member_t));
    strcpy(new_member->username, username);
    strcpy(new_member->pwd, pwd);
    new_member->next = db->next;
    db->next = new_member;
}

// free db
void destroy_member_db(member_t* db) {
    while (db != NULL) {
        member_t * node = db;
        db = db->next;
        free(node);
    }
}

// load member from file to linked list
void load_member(char* file_path, member_t* db) {
    FILE *fp;
    char row[MAXLEN];
    char username[MAXLEN];
    char pwd[MAXLEN];

    fp = fopen(file_path,"r");

    while (feof(fp) != TRUE)
    {
        // read row
        fgets(row, MAXLEN, fp);
        //printf("Row: %s", row);
        if (strlen(row) == 0) {break;}

        // parse row
        sscanf(row, "%[^,], %s", username, pwd);
        insert_member(db, username, pwd);

        strcpy(row, "");
    }

    fclose(fp);
}

// traverse & print member db
void print_member(member_t* db) {
    db = db->next;
    while (db != NULL) {
        printf("%s %s\n", db->username, db->pwd);
        db = db->next;
    }
}

// test
void test_room(char* room) {
    room_t* db_room = malloc(sizeof(room_t));
    char room_str[MAXLEN];
    load_room("./single.txt", db_room, room_str);
    print_room(db_room);
    printf("%d", lookup_room(db_room, room));
    printf("%d", reserve_room(db_room, room));
    printf("%d", lookup_room(db_room, room));
    destroy_room_db(db_room);
    db_room = malloc(sizeof(room_t));
    load_room_str(db_room, room_str);
    print_room(db_room);
    destroy_room_db(db_room);
}

// test
void test_member(char* username, char* pwd) {
    member_t* db = malloc(sizeof(member_t));
    encrypt(username);
    encrypt(pwd);
    load_member("./member_extra.txt", db);
    print_member(db);
    printf("%d", lookup_member(db, username, pwd));
    destroy_member_db(db);
}

// shift characters
char shift_from(char in, char c, int n, int shift) {
    int dist = in-c;
    if (dist >= 0 && dist < n) {
        dist += shift;
        if (dist >= n) dist -= n;
    }
    return c + dist;
}
char shift_back(char in, char c, int n, int shift) {
    int dist = in-c;
    if (dist >= 0 && dist < n) {
        dist -= shift;
        if (dist < 0) dist += n;
    }
    return c + dist;
}

#ifdef EXTRA
// extra credit encryption algorithm
void encrypt(char* str) {
    char encrypt[MAXLEN];
    memset(encrypt, ' ', MAXLEN);
    int pos = 0;
    int len = strlen(str);
    int h = (int) floor(sqrt(len));
    int w = (int) ceil(sqrt(len));
    if (w*h < len) h++;

    for (int i = 0; i < h; i++) {
        for (int j=0; j < w; j++) {
            if (str[j*h + i] == '\0') {
                encrypt[pos++] = '`';
            } else {
                encrypt[pos] = str[j*h + i];
                encrypt[pos] = shift_from(encrypt[pos], '0', 10, i+j+1);
                encrypt[pos] = shift_from(encrypt[pos], 'A', 26, i+j+1);
                encrypt[pos] = shift_from(encrypt[pos], 'a', 26, i+j+1);
                pos++;
            }
        }
    }
    
    encrypt[pos] = '\0';
    strcpy(str, encrypt);
}

void unencrypt(char* str) {
    char unencrypt[MAXLEN];
    memset(unencrypt, ' ', MAXLEN);
    int pos = 0;
    int len = strlen(str);
    int h = (int) floor(sqrt(len));
    int w = (int) ceil(sqrt(len));
    for (int i = 0; i < w; i++) {
        for (int j=0; j < h; j++) {
            if (str[i + j*w] == '`') {
                unencrypt[pos++] = '\0';
            } else {
                unencrypt[pos] = str[i + j*w];
                unencrypt[pos] = shift_back(unencrypt[pos], '0', 10, i+j+1);
                unencrypt[pos] = shift_back(unencrypt[pos], 'A', 26, i+j+1);
                unencrypt[pos] = shift_back(unencrypt[pos], 'a', 26, i+j+1);
                pos++;
            }
        }
    }
    unencrypt[pos] = '\0';
    strcpy(str, unencrypt);
}
#else
// basic encryption algorithm
void encrypt(char* str) {
    int pos = 0;
    while (pos < strlen(str)) {
        str[pos] = shift_from(str[pos], 'a', 26, ENCRYPT_SHIFT);
        str[pos] = shift_from(str[pos], 'A', 26, ENCRYPT_SHIFT);
        str[pos] = shift_from(str[pos], '0', 10, ENCRYPT_SHIFT);
        pos++;
    }
}

void unencrypt(char* str) {
    int pos = 0;
    while (pos < strlen(str)) {
        str[pos] = shift_back(str[pos], 'a', 26, ENCRYPT_SHIFT);
        str[pos] = shift_back(str[pos], 'A', 26, ENCRYPT_SHIFT);
        str[pos] = shift_back(str[pos], '0', 10, ENCRYPT_SHIFT);
        pos++;
    }
}
#endif


//int main() {
// //     // test_room("S301");
// //     char username[10];
// //     memset(username, '\0', 10);
// //     strcpy(username, "James");
// //     char pwd[10];
// //     memset(pwd, '\0', 10);
// //     strcpy(pwd, "SODids392");
// //     test_member(username, pwd);
//     char test[MAXLEN];
//     memset(test, '\0', MAXLEN);
//     strcpy(test, "Welcome-To-EE450!");
//     encrypt(test);
//     printf("%s %lu\n", test, strlen(test));
//     unencrypt(test);
//     printf("%s %lu\n", test, strlen(test));
// }