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

using namespace std;


//encrypt
string encrypt(const string& info) {
    string encryptresult = "";
    for (char c : info) {
    if (isalpha(c)) {
            char base = isupper(c)?'A':'a';
            encryptresult += ((c-base+3)%26)+base;
    }
        else if (isdigit(c)) {
            encryptresult += ((c-'0'+3)%10)+'0';
        }else{
            encryptresult += c; // Special characters remain unchanged
}
    }
    return encryptresult;
}

//authentication
string authentication(int sockfd,int port) {
    string encryptresult;
     string input;
    while(1){
    // Get username
    string username;
    //edge case: username must be all alphabetic character
     do {
        cout << "Please enter the username: ";
        getline(cin, username);
        // Check if username not only contains alphabetic letters
        bool notletter = false;
        for (char c : username) {
            if (!isalpha(c)) {
                notletter = true;
                break;
            }
        }
        if (notletter) {
            cout << "Username must contain only alphabetic characters. Please try again." << endl;
            username = "";
        }
    } while (username == "");


    // Get password
      string password;
   
  do {
     cout << "Please enter the password (Press “Enter” to skip for guest):";
    getline(cin, password);
 //edge case: check if password contains spaces
 if (password.find_first_not_of(' ') == string::npos) {
            break; //case for Press “Enter” to skip. 'find_first_not_of' search from chatgpt
        }
        if (password.find(' ') != string::npos) {
            cout << "Password cannot contain spaces. Please try again." << endl;
            password = "";
        }
    } while (password == "");
    //for guest: check if password is empty (user pressed Enter)
    bool isGuest = password.empty();

    // Encrypt
    string encryptusername = encrypt(username);
    if (!isGuest) {
    string encryptpassword = encrypt(password);
    encryptresult=encryptusername+" "+encryptpassword;
    cout << username << " sent an authentication request to the main server."<< endl;
 // Send the encrypted login info to the server
        send(sockfd, encryptresult.c_str(), encryptresult.size(), 0);     
//receiving the result of the authentication request from Main Server. Beej P49
        char buffer[100];
        int nbytes = recv(sockfd, buffer, sizeof(buffer), 0);  
        if (nbytes == -1) {
        perror("recv");
        continue;
        }
buffer[nbytes] = '\0';
 if (strcmp(buffer, "Authenticated") == 0){
    cout<< "Welcome member "<<username <<"!"<< endl;
    string input = username+" "+password;
    return  input;
 }else if (strcmp(buffer, "Usernotfound") == 0){
 cout<<"Failed login: Username does not exist."<<endl;
}
else if(strcmp(buffer, "Incorrectpassword") == 0){
    cout<<"Failed login: Password does not match."<<endl;
}
}
 else{
// Send the encrypted login info to the server
    send(sockfd, encryptusername.c_str(), encryptusername.size(), 0);   
    cout << username << " sent a guest request to the main server using TCP over port "<<port<<"."<< endl;

    //receiving the result of the authentication request from Main Server. Beej P49
        char buffer[100];
        int nbytes = recv(sockfd, buffer, sizeof(buffer), 0);  
         if (nbytes == -1) {
            perror("recvguest");
            continue;
        }
        buffer[nbytes] = '\0';
        string guestconfirm(buffer);
        if(guestconfirm == "Guest"){
 cout << "Welcome guest "<<username<<"!"<<endl;
       
    return  username;
       }
     }
    }
return  input;
}
int main() {
    cout << "Client is up and running." << endl;

    // Establish TCP connection,bind and listen. from beej
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("client socket");
         return 1;
    }

    // Bind the socket to an automatically assigned local address
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    client_addr.sin_port = 0; 
    if (::bind(sockfd, (struct sockaddr*)&client_addr, sizeof(client_addr)) == -1) {
        perror("bind");
        close(sockfd);
        return 1;
    }

//from project description
/*Retrieve the locally-bound name of the specified socket and store it in the sockaddr structure*/
    struct sockaddr_in local_addr;
    socklen_t local_addr_len = sizeof(local_addr);
    int getsock_check=getsockname(sockfd, (struct sockaddr*)&local_addr, (socklen_t *)&local_addr_len);
//Error checking
    if(getsock_check== -1) {
        perror("getsockname");
        return 1;
    }
    int local_port = ntohs(local_addr.sin_port);   


    // Connect to the server.from Beej
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    server_addr.sin_port = htons(45764); 
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error connecting to server");
        close(sockfd);
        return 1; 
    }

//authentication
//save the user's name in username and the user type in isguest. if isguest is false,the user is a member.
bool isguest;
string userinput=authentication(sockfd,local_port);
string username, password;
size_t pos = userinput.find(" ");
if (pos != string::npos) {
    username = userinput.substr(0, pos);
   // password = userinput.substr(pos + 1);
    isguest=false;
}else{
 isguest=true;
 username = userinput;
}

//room code request.  
    while(1){
        string roomcode;
        cout << "Please enter the room code:";
        getline(cin, roomcode);
        string action;
        do{
        cout << "Would you like to search for the availability or make a reservation? (Enter “Availability” to search for the availability or Enter “Reservation” to make a reservation ):";
        getline(cin, action);
       if (action != "Availability" && action != "Reservation") {
            cout << "Invalid input. Please enter 'Availability' or 'Reservation' strictly." << endl;
            action="";
        }
         } while (action=="");

    if(action=="Availability"){
        cout <<username<<" sent an availability request to the main server."<<endl;
        std::string request= "a,"+roomcode+","+"member";
        send(sockfd, request.c_str(), request.size(),0);

//recv and parse available response
        char requestBuffer[100];
        int requestreturn = recv(sockfd,requestBuffer,sizeof(requestBuffer),0);
         requestBuffer[requestreturn] = '\0';
        string requestResult(requestBuffer);
        if(requestreturn == -1){
            perror("Availabilityroomrecv");
            break; 
        }
        cout << "The client received the response from the main server using TCP over port "<<local_port<<"."<<endl;
   //     cout << "The client received the response "<<requestResult<<"."<<endl;
   //parse response and print result
        if (requestResult=="Available") { 
        cout <<"The requested room is available.\n"<< endl;
        cout <<"-----Start a new request-----"<< endl;
        } 
        else if (requestResult == "Notavailable") {
            cout << "The requested room is not available.\n" << endl;
            cout <<"-----Start a new request-----"<< endl;
            } 
        else if (requestResult == "Notabletofind") {
            cout << "Not able to find the room layout.\n" << endl;
            cout <<"-----Start a new request-----"<< endl;
            }  
            else if(requestResult == "eb") {
                 cout<<"Invalid roomcode! Please input a roomcode with S/D/U prefix."<<endl;
            }       
        }
    else if(action=="Reservation"){
      
        if(!isguest) {
          //   cout <<"isguest"<<isguest<<endl;

         std::string request= "r,"+roomcode+","+"member";
          send(sockfd, request.c_str(), request.size(),0);
  cout <<username<<" sent a reservation request to the main server."<<endl;
          //recv
       char requestBuffer[1024];
        int requestreturn = recv(sockfd,requestBuffer,sizeof(requestBuffer),0);
        if(requestreturn == -1){
            perror("Reservationroomrecv");
            continue; 
        }
         requestBuffer[requestreturn] = '\0';
        string requestResult(requestBuffer);
        cout << "The client received the response from the main server using TCP over port "<<local_port<<"."<<endl;
        if(requestResult=="t"){
            cout << "Congratulation! The reservation for Room "<<roomcode<<" has been made.\n"<<endl;
            cout <<"-----Start a new request-----"<< endl;
        }
        else if(requestResult=="f"){
        cout <<"Sorry! The requested room is not available.\n"<<endl;
        cout <<"-----Start a new request-----"<< endl;
        }
        else if(requestResult=="n"){
             cout <<"Oops! Not able to find the room.\n"<<endl;
             cout <<"-----Start a new request-----"<< endl;
        }
         else if(requestResult == "eb") {
                cout<<"Invalid roomcode! Please input a roomcode with S/D/U prefix."<<endl;
            }      
    }
    else{
//guest want reservation, sent request to main
  string request= "r,"+roomcode+","+"guest";
          send(sockfd, request.c_str(), request.size(),0);
            cout <<username<<" sent a reservation request to the main server."<<endl;
 //recv reject response from the main server and print
        char guestReservationbuf[1024];
        int guestReservation = recv(sockfd,guestReservationbuf, sizeof(guestReservationbuf), 0);  
        if(guestReservation == -1){
            perror("guestReservation");
             continue;
        }
        guestReservationbuf[guestReservation] = '\0';
        string errorresponse(guestReservationbuf);
        if(errorresponse=="gnr"){
cout <<"Permission denied: Guest cannot make a reservation."<<endl;
        }
    }
    }
}
 close(sockfd);
 return 0;
}
