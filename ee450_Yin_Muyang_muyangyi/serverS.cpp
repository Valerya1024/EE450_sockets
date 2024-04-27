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

const int PORT = 41764;
const int MAIN_SERVER_PORT = 44764;
const string id = "S";
const string filename = "single.txt";
struct sockaddr_in bdserver;


//read the room information from the txt file and save it into a map<string,int> data structure
map<string, int> readSaveFile(const string& filename) {
    map<string, int> roomMap;
    ifstream file(filename);
    string line;
    if (file.is_open()) {
        while (getline(file, line)) {
            size_t pos = line.find(',');
            if (pos != string::npos) {
                string key = line.substr(0, pos);
                int value = stoi(line.substr(pos + 1));
                roomMap[key] = value;
            }
        }
        file.close();
    } else {
        cerr << "Unable to open file: " << filename << endl;
    }
    return roomMap;
}


//handle Availability Request
string handleAvailabilityRequest(string roomcode,map<string,int> &roommap) {
    string response;
    int count = roommap[roomcode];
if (count > 0) {
        cout<< "Room " <<roomcode<< " is available." <<endl;
         response = "Available";

    } else if(count == 0) {
        cout<< "Room " <<roomcode<< " is not available." <<endl;
         response = "Notavailable";
    }
    return response;
}

//handle Reservation Request
void handleReservationRequest(string roomcode,map<string,int> &uRoomMap, sockaddr_in bdserver, socklen_t bdserverize, int sockfd) {
string response;
 int count = uRoomMap[roomcode];
if (count > 0) {
    uRoomMap[roomcode]=count-1;
         response = "t";
        sendto(sockfd, response.c_str(), response.size(), 0, (struct sockaddr*)&bdserver, bdserverize);
        cout<<"Successful reservation. The count of Room "<<roomcode<<" is now "<<to_string(uRoomMap[roomcode])<<"."<<endl;
        cout<<"The Server "<<id<<" finished sending the response and the updated room status to the main server."<<endl;
        uRoomMap[roomcode]=count-1;
    } else if(count == 0) {
        response = "f";
        cout<<"Cannot make a reservation. Room "<< roomcode<<" is not available."<<endl;
        sendto(sockfd, response.c_str(), response.size(), 0, (struct sockaddr*)&bdserver, bdserverize);
        cout<<"The Server "<<id<<" finished sending the response to the main server."<<endl;
    }

}


//copy from Beej guide p25 'THIS IS THE OLD WAY' part
int newudp(){
    int udpSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSock == -1){
        perror("create UDP socket");
        exit(1);
    }
    bdserver.sin_family = AF_INET;
    bdserver.sin_addr.s_addr = inet_addr("127.0.0.1");
    bdserver.sin_port = htons(PORT);
    memset(bdserver.sin_zero, '\0', sizeof bdserver.sin_zero);
    if (::bind(udpSock, (struct sockaddr *)&bdserver, sizeof(bdserver))==-1){
        perror("bind UDP socket");
        exit(1);
    }
    return udpSock;
}
//main function
int main() {
    cout<<"The Server "<<id<<" is up and running using UDP on port "<<PORT<<"."<<endl;
    map<string, int> roommap = readSaveFile(filename);
    //set udp socket
 int sockfd = newudp();
     // prepare main server addr and msg. addr part from beej P25
    struct sockaddr_in mainServerAddr;
    mainServerAddr.sin_family = AF_INET;
    mainServerAddr.sin_port = htons(MAIN_SERVER_PORT);
    mainServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    stringstream ss;
    for (auto& room : roommap) {
        ss << room.first << "," << room.second << ";";
    }
    string message = ss.str();
    int n;
    
// send the 1st room status to main
    if ((n = sendto(sockfd, message.c_str(), message.length(), 0,
             (struct sockaddr *)&mainServerAddr, sizeof(mainServerAddr))) == -1) {
        perror("sendtomain");
        exit(1);
    }
    cout<<"The Server "<<id<<" has sent the room status to the main server."<<endl;
    close(sockfd);


// recv either "Availability" or "Reservation" request. 
//recvfrom learn from beej definition and p39, p115 example. sendto learn from beej definition and p74 example
    sockfd = newudp();
    while (1) {
        //recvfrom learn from beej p39, p115
        char requestBuffer[1024];
        socklen_t bdserverize = sizeof(bdserver);
        //recv request and roomcode from main server
        int bytesReceived = recvfrom(sockfd, requestBuffer, sizeof(requestBuffer), 0, (struct sockaddr*)&bdserver, &bdserverize);
        if (bytesReceived == -1) {
            cout << "Error: Failed to receive data." << endl;
            break;
        }
        //parse
        requestBuffer[bytesReceived] = '\0';
        string request(requestBuffer);
        size_t pos = request.find(",");
        string requestType, roomcode;
        requestType = request.substr(0, pos);
        roomcode = request.substr(pos + 1);
        //handle request and send result
        if (requestType == "a") {
            cout<<"The Server "<<id<<" received an availability request from the main server."<<endl; 
            if(roommap.count(roomcode)==0){
            cout<<"Not able to find the room layout."<<endl;
            // send result to main server
            string response = "Notabletofind";
            sendto(sockfd, response.c_str(), response.size(), 0, (struct sockaddr*)&bdserver, bdserverize);
            cout<<"The Server "<<id<<" finished sending the response to the main server."<<endl;
            }else{
                string response = handleAvailabilityRequest(roomcode,roommap);
                sendto(sockfd, response.c_str(), response.size(), 0, (struct sockaddr*)&bdserver, bdserverize);
                cout << "The Server "<<id<<" finished sending the response to the main server." << endl;
            }
            
        } else if (requestType == "r") {
            cout<<"The Server "<<id<<" received a reservation request from the main server."<<endl;
            if(roommap.count(roomcode)==0){
            cout<<"Cannot make a reservation. Not able to find the room layout."<<endl;
            //send result to main
            string response = "n";
            sendto(sockfd, response.c_str(), response.size(), 0, (struct sockaddr*)&bdserver, bdserverize);
            cout<<"The Server "<<id<<" finished sending the response to the main server. "<<endl;
            }else{
            handleReservationRequest(roomcode,roommap,bdserver,bdserverize,sockfd);
            }
        }
    }
 close(sockfd);
return 0;
}