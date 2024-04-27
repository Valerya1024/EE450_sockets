a. Name: Xinyu Wu

b. usc ID: 2850514907

c. I implemented TCP server / client and UDP server / client, load data from file to a linked list, user login, query and reserve room.
    Extra credit:
            I implemented an encryption / decryption algorithm for extra credit.
            The encryption algorithm is implemented with both reordering and shifting.
        Reorder: assume the string is in a w x h matrix, read the string column-wise.
            the matrix size is w = ceil(sqrt(str_lenghth)), h = ceil(str_lenghth/h)
            for example, "Welcome-To-EE450!" can be fit into a 5x4 matrix:
                                      0 1 2 3 
                                    0 W e l c
                                    1 o m e -
                                    2 T o - E
                                    3 E 4 5 0
                                    4 ! \0\0\0
            The reordered string is a 20-char string "WoTE!emo4\0le-5\0c-E0\0". '\0' in the string is replaced by '`'.
        shifting: similar to the original scheme in this project, shift the character [i, j] by i+j+1.
            for example, character 0 is located at [3,3], and will be shifted by 7 to 7.
            The result string is "XqWI!gps9`oi-1`g-K7`".
        Run:
            Run "make extra", then run ./client ./server... as in normal mode.
                When using "make extra", serverM reads encrypted member message from "member_extra.txt".
                The unencrypted version of "member_extra.txt" is:
                                    james, SODids392
                                    patricia, 3827#o29
                                    michael, Pass12398

            To switch back to normal mode, run "make" again.


d. Files and what they do
    client.c client.h: code for client, handles user input, send request to main server, receive response from main server over TCP
    serverM.c serverM.h: code for main server, loads member information from file, handle user login / quey / reserve request, forward requests to backend servers over UDP 
    connection.c connection.h: defines functions for tcp / udp communication.
    serverS/D/U.c serverS/D/U.h: code for backend servers. Load room data from file, send room status to main server on boot up. handle requests forwarded by main server.
    util.c util.h: define status code, file input functions, linked list functions, encryption functions.
    msg.h: message format strings.
    extra.h: when using extra credit encryption algorithm, #define EXTRA is added into this file. It only has one line.

e. Format of messages
    send room status (backend -> serverM): room code and count in csv format.
    login (client -> serverM): Username and password are concatenated and delimited by a space.
    send action (client -> serverM -> backend): mode code (menber or guest), action code (query or reserve), room code are concatenated and delimited by a space.
    send response (backend -> serverM -> backend): a single status code or room count.

f. Idiosyncrasy
    Does not handle concurrency.

g. Reference:
    Beej Tutorial Sockets, 6.1 A Simple Stream Server, page 34
    Beej Tutorial Sockets, 5.3 bind(), page 25
    Geeks for Geeks, reorder encryption algorithm https://www.geeksforgeeks.org/encryption-and-decryption-of-string-according-to-given-technique/
