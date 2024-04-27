#include <stdio.h>
#include <stdlib.h>   
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

using namespace std;

#define MAXDATASIZE 1024

//mainserver port
const int UDP_SERVER_PORT=44764;
const int TCP_SERVER_PORT=45764;
//backend server port
const int UDP_S_PORT=41764;
const int UDP_D_PORT=42764;
const int UDP_U_PORT=43764;
//file name
const char* MEMBER_FILE = "member.txt";

//beej p34
// void sigchld_handler(int s) {
//     // waitpid() might overwrite errno, so we save and restore it:
// int saved_errno = errno;
// while(waitpid(-1, NULL, WNOHANG) > 0);
// errno = saved_errno; }

//read the member information from the txt file and save it into a map<string,int> data structure
//find_first_not_of() from chatgpt search result
map<string, string> readSaveFile() {
    map<string, string> member;
    ifstream file(MEMBER_FILE);
    string line;

    if (file.is_open()) {
        while (getline(file, line)) {
            size_t pos = line.find(',');
            if (pos != string::npos) {
                // Find position of non-space character after the comma
                size_t skipspacepos = line.find_first_not_of(" ", pos + 1);
                if (skipspacepos != string::npos) {
                    string key = line.substr(0, pos);
                    string value = line.substr(skipspacepos);
                    member[key] = value;
                }
            }
        }
        file.close();
    } else {
        cerr << "Unable to open file: " << MEMBER_FILE << endl;
    }
    return member;
}

//send availiability request to backend server through udp and recv, then send to client with tcp send
void Availabilityudprequest(int sockudp, const string& request_msg, const struct sockaddr_in& tcp_server_addr, socklen_t server_addr_len, int client_fd, string id) {
 char buffer[1024]; 
    ssize_t room;
    char roomstatusbuf[1024]; 

// send to backend 
    if (sendto(sockudp, request_msg.c_str(), request_msg.size(), 0, (struct sockaddr*)&tcp_server_addr, sizeof(tcp_server_addr)) == -1) {
        perror("sendto");  
        return;
    }
  //   cout << "request:"<<request_msg<<endl;
    cout << "The main server sent a request to Server"<<id<<"." << endl;
    //recv from backend
    room = recvfrom(sockudp, roomstatusbuf, sizeof(roomstatusbuf), 0,(struct sockaddr *)&tcp_server_addr, &server_addr_len );
        if (room == -1) {
            perror("recvfrom"); 
           return;
        }
        roomstatusbuf[room] = '\0'; 
        string roomstatus(roomstatusbuf);
         cout<<"The main server received the response from Server "<<id<<" using UDP over port 45764."<<endl;
          
 //sent to client
        int bytes_sent = send(client_fd, roomstatus.c_str(), roomstatus.size(), 0);
        if (bytes_sent == -1) {
            perror("fail to send response to client"); 
        }
          cout<<"The main server sent the availability information to the client."<<endl;
}

void Reservationudprequest(string roomcode,int sockudp, const string& request_msg, const struct sockaddr_in& tcp_server_addr, socklen_t server_addr_len, int client_fd, string id,map<string,int>roommap){
char buffer[1024]; 
    ssize_t room;
    char roomstatusbuf[1024]; 
// send to backend 
    if (sendto(sockudp, request_msg.c_str(), request_msg.size(), 0, (struct sockaddr*)&tcp_server_addr, sizeof(tcp_server_addr)) == -1) {
        perror("sendto");
        close(client_fd);
        return;
    }
    cout << "The main server sent a request to Server "<<id<<"." << endl;
 //recv from backend
    room = recvfrom(sockudp, roomstatusbuf, sizeof(roomstatusbuf), 0,(struct sockaddr *)&tcp_server_addr, &server_addr_len );
        if (room == -1) {
            perror("recvfrom");
            close(client_fd);
           return;
        }
        roomstatusbuf[room] = '\0'; 
        string roomstatus(roomstatusbuf);

int count = roommap[roomcode];
if(roomstatus=="n"||roomstatus=="f"){
cout<<"The main server received the response from Server "<<id<<" using UDP over port 45764."<<endl;
}else if(roomstatus=="t"){
    roommap[roomcode]=count-1;
cout<<"The main server received the response and the updated room status from Server "<<id<<" using UDP over port 45764."<<endl;
cout<<"The room status of Room "<<roomcode<<" has been updated."<<endl;
 }
 //const char* response = roomstatus.c_str();
        int bytes_sent = send(client_fd, roomstatus.c_str(), roomstatus.size(), 0);
        if (bytes_sent == -1) {
            perror("send");
        }
 cout<<"The main server sent the reservation result to the client."<<endl;
}

int main(){
    cout<<"The main server is up and running."<<endl;
    //map users stored the member info from txt
    map<string,string> users=readSaveFile();
    // Maps to store data received from backend servers
    map<string, int> sroom;
    map<string, int> droom;
    map<string, int> uroom;
    //recieve the data of room txts from the backend server
    //Create UDP socket
int sockudp = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockudp == -1) {
        perror("udpsocket");
        cout << "udpsocket" << endl;
        return 1;
    }
    // Bind UDP socket
    sockaddr_in udp_server_addr;
    memset(&udp_server_addr, 0, sizeof(udp_server_addr));
    udp_server_addr.sin_family = AF_INET;
    udp_server_addr.sin_port = htons(UDP_SERVER_PORT); 
    udp_server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (::bind(sockudp, (struct sockaddr *)&udp_server_addr, sizeof(udp_server_addr)) == -1) {
        perror("bind UDP socket");
        return 1;
    }
    //copy from Beej P86"example of packing a struct by hand, IPv4" 
    // addr for backend S
     struct sockaddr_in addrS;
    addrS.sin_family = AF_INET;
    addrS.sin_port = htons(UDP_S_PORT);
    inet_pton(AF_INET, "127.0.0.1", &addrS.sin_addr);
     // addr for backend D
     struct sockaddr_in addrD;
    addrD.sin_family = AF_INET;
    addrD.sin_port = htons(UDP_D_PORT);
    inet_pton(AF_INET, "127.0.0.1", &addrD.sin_addr);
     // addr for backend U
     struct sockaddr_in addrU;
    addrU.sin_family = AF_INET;
    addrU.sin_port = htons(UDP_U_PORT);
    inet_pton(AF_INET, "127.0.0.1", &addrU.sin_addr);
    
//recv 1st room status from backend and save in map
    int receivedCount = 0;
    while (receivedCount < 3) {
        // recv from backend
        char buffer[1024];
        struct sockaddr_in their_addr;
        socklen_t addr_len = sizeof(their_addr);
        int numbytes = recvfrom(sockudp, buffer, sizeof(buffer), 0,
                                (struct sockaddr *)&their_addr, &addr_len);
        if (numbytes == -1) {
            perror("recvfrom");
            continue;
        }
        // parse
        buffer[numbytes] = '\0'; 
        string data(buffer);
 //cout << data << endl;
        istringstream iss(data);
        string keyValue;
        while (getline(iss, keyValue, ';')) {
            size_t pos = keyValue.find(',');
            if (pos != string::npos) {
                string key = keyValue.substr(0, pos);
                int value = stoi(keyValue.substr(pos + 1));
                switch (receivedCount) {
                    case 0:
                        sroom[key] = value;
                        break;
                    case 1:
                        droom[key] = value;
                        break;
                    case 2:
                        uroom[key] = value;
                        break;
                }
            }
        }
        receivedCount++;
        switch (receivedCount) {
            case 1:
                cout << "The main server has received the room status from Server S using UDP over port 44764." << endl;
                break;
            case 2:
                cout << "The main server has received the room status from Server D using UDP over port 44764." << endl;
                break;
            case 3:
                cout << "The main server has received the room status from Server U using UDP over port 44764." << endl;
                break;
        }
    }


// Create TCP socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("tcpsocket");
cout << "tcpsocket" << endl;
        return 1;
    }
    // Bind TCP socket
    sockaddr_in tcp_server_addr;
    memset(&tcp_server_addr, 0, sizeof(tcp_server_addr));
    tcp_server_addr.sin_family = AF_INET;
    tcp_server_addr.sin_port = htons(TCP_SERVER_PORT);
    tcp_server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (::bind(sockfd, (struct sockaddr *)&tcp_server_addr, sizeof(tcp_server_addr)) == -1) {
        perror("bind TCP socket");
        return 1;
    }
    
    //tcp listen
    if (listen(sockfd, 5) == -1) {
            perror("listen");
           return 1;
        }
         string usernm;
    while (1) {
        //tcp Accept connection
        sockaddr_in client_addr;
        int client_len = sizeof(struct sockaddr_in);
        int client_fd = accept(sockfd, (struct sockaddr *)&client_addr, (socklen_t*)&client_len);
        if (client_fd == -1) {
            perror("accept");
             cout << "accept" << endl;
            continue;
        }
        
        //learn from Beej P36
        if (!fork()) { // Child process
            close(sockfd); 

        //Receive authentication request from client
       
        while(1){
        char buffer[1024];
        
        int bytes_received = recv(client_fd, buffer, 1023, 0);
        if (bytes_received == -1) {
            perror("recv");
            cout << "The main server not received" <<endl;
            close(client_fd);
            continue;
        }
        buffer[bytes_received] = '\0';
        string request(buffer);
        //parse authentication request
        size_t pos = request.find(" ");
        string username, password;
        string userType;
        if (pos != string::npos) {
        //msg contains username and password, member request
        username = request.substr(0, pos);
        password = request.substr(pos + 1);
        usernm=username;
        userType="member";
        // cout << "username"<<username<<endl;
        // cout << "password"<<password<<endl;
        cout << "The main server received the authentication for " << username << " using TCP over port 45764." << endl;
        //Authenticate username and password
        string authentication_result;
         if (users.find(username)== users.end()) {
      authentication_result= "Usernotfound";
 //Send authentication result to client
        const char* response = authentication_result.c_str();
  int bytes_sent = send(client_fd, authentication_result.c_str(), authentication_result.length(), 0);
        if (bytes_sent == -1) {
            perror("send");
             continue;
        }
         cout << "The main server sent the authentication result to the client." << endl;

    }else if (users[username].compare(password)!=0) {
            authentication_result= "Incorrectpassword";
             //Send authentication result to client
       int bytes_sent = send(client_fd, authentication_result.c_str(), authentication_result.length(), 0);
        if (bytes_sent == -1) {
            perror("send");
             continue;
        }
         cout << "The main server sent the authentication result to the client." << endl;}
 else{
    authentication_result= "Authenticated";
        //Send authentication result to client
        int bytes_sent = send(client_fd, authentication_result.c_str(), authentication_result.length(), 0);
        if (bytes_sent == -1) {
            perror("send");
             continue;
        }
         cout << "The main server sent the authentication result to the client." << endl;
            break;
         }
} else {
    //msg contains username only, guest request
    username = request;
    usernm=username;
    userType="guest";
    cout << "The main server received the guest request for " << username << " using TCP over port 45764." << endl;
    cout << "The main server accepts " << username << " as a guest." << endl;
    const char* response = "Guest";
    int bytes_sent = send(client_fd, response, strlen(response), 0);
    cout<<"The main server sent the guest response to the client."<<endl;
    break;
}
    }

while(1){
//recv the Availability/reservation request
char querybuf[1024];
        int queryreceived = recv(client_fd, querybuf, sizeof(querybuf), 0);
        if (queryreceived == -1) {
            perror("recv");
            close(client_fd);
            continue;
        }
        querybuf[queryreceived] = '\0';
        string query(querybuf);
        
   // Parse Availability/reservation request
    size_t cut1 = query.find(",");
    size_t cut2 = query.find(",", cut1 + 1);
    string requestType, roomcode,userrequest;
    //msg contains requestType,roomcode,userrequest
    requestType = query.substr(0, cut1);
    roomcode = query.substr(cut1 + 1, cut2 - cut1 - 1);
    userrequest = query.substr(cut2 + 1);
    int room;
    char roomstatusbuf[1024];
    if(requestType=="a"){
    cout<<"The main server has received the availability request on Room "<<roomcode<<" from "<<usernm<<" using TCP over port 45764."<<endl;
   string request= requestType+","+roomcode;
    if (roomcode[0] == 'S') {
        // send to backend S
        string id="S";
   Availabilityudprequest(sockudp,request,addrS,sizeof(addrS),client_fd,id);
} else if (roomcode[0] == 'D') {
    // send to backend D
     string id="D";
   Availabilityudprequest(sockudp,request,addrD,sizeof(addrD),client_fd,id);

} else if (roomcode[0] == 'U') {
 string id="U";
   Availabilityudprequest(sockudp,request,addrU,sizeof(addrU),client_fd,id);
} else{
     //not for a valid backend,send error msg
     string errbackend = "eb";
        int bytes_sent = send(client_fd, errbackend.c_str(), errbackend.size(), 0);
        if (bytes_sent == -1) {
            perror("send");
        }
 cout<<"Not S/D/U backend. The main server sent the error msg to the client."<<endl;
}
    } else if(requestType=="r"){
    cout<<"The main server has received the reservation request on Room "<<roomcode<<" from "<<usernm<<" using TCP over port "<<TCP_SERVER_PORT<<"."<<endl;
  if(userrequest=="guest"){
    cout<<usernm<<" cannot make a reservation."<<endl;
//send error to client
 const char* errorresponse = "gnr";
        int errorsent = send(client_fd, errorresponse, strlen(errorresponse), 0);
    cout<<"The main server sent the error message to the client."<<endl;
  }
  else if(userrequest=="member"){
    //send result to backend Server
    string request= requestType+","+roomcode;
if (roomcode[0] == 'S') {
        // send to backend S
        string id="S";
   Reservationudprequest(roomcode,sockudp,request,addrS,sizeof(addrS),client_fd,id,sroom);
} else if (roomcode[0] == 'D') {
    // send to backend D
     string id="D";
   Reservationudprequest(roomcode,sockudp,request,addrD,sizeof(addrD),client_fd,id,droom);
 // send to backend =U
} else if (roomcode[0] == 'U') {
 string id="U";
   Reservationudprequest(roomcode,sockudp,request,addrU,sizeof(addrU),client_fd,id,uroom);
}
else{
     //not a valid backend prefix,send error msg
     string errbackend = "eb";
        int bytes_sent = send(client_fd, errbackend.c_str(), errbackend.size(), 0);
        if (bytes_sent == -1) {
            perror("send");
        }
 cout<<"Not S/D/U backend. The main server sent the error msg to the client."<<endl;
}
  }
    } 
    }
    close(client_fd);
exit(0); }
close(client_fd);}
    close(sockfd);
    return 0;
}

