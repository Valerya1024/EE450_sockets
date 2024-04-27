a. Muyang YIN
b. 4875681764
c. I complete all the steps in the assignment. 
d. What your code files are and what each one of them does. (Please do not repeat the project description, just name your code files and briefly mention what they do):
My project consists of a total of 5 cpp files: serverM, client, and three backend servers S, D, U.

<1>For client:client.cpp
This client mainly used for connecting with users and main servers，obtaining user’s requirements, ask main server and responding users accordingly. My client has the following functions:
1.string encrypt(const string& info): This function is used during the authentication process.It encrypts the input string according to the project's encryption requirements. In the project, we encrypt the username and password (if any) entered by the user.
2.string authentication(int sockfd,int port): This function requires an integer TCP socket and the port dynamically allocated for this client as parameters. It also prompts the user for input.  The socket is used for communication with the main server, and the port is required for printing on-screen messages.This function encompasses the entire user identity verification process, including reading and checking user input, encrypting input, requesting authentication from the main server, receiving and printing the main server's authentication result.
	-2.1.reading and checking user input: According to the requirements in piazza@152, check if the user input meets the restrictions (if the username is not all lowercase English letters, or if the password (if not skipped) contains spaces). If the input does not meet the restrictions, I have set up error prompts to be displayed on the screen and ask the user to re-enter.
	-2.2. input encrypt, send and receive authentication: Use the encrypt() function for encryption. if it's not a guest, combine the encrypted username and password into a single string separated by spaces. Communicate with the main server via TCP, use send() to send this string, and recv() to receive the result from the main server. 
3.int main():Executes the entire process.
	-3.1.Boot up and create tcp socket: using socket(),bind(). After bind a random port, use getsockname() to achieve the main port。After That use Connect() to establish connection with main server.
	-3.2.Complete tcp socket and then use authentication() identity authentication process.	
    -3.3.After authentication, user will send the request about the room as a member or a guest. Use while loop to keep terminal until forcibly exited with Ctrl+C. First, prompt the user to input the room code. Then, prompt the user to input the desired action. Validate the action strictly, ensuring it matches either "Availability" or "Reservation"; otherwise, display an error and prompt the user to input again.
	-3.4.After getting action，roomcode, send this msg with user type to main server。if  action is Availability，Receive the request, analyze, and print the results. For "Reservation" action, discuss based on user identity:
    For a member: send the request, receive the response, analyze, and print the results.
    For a guest: after sending the request, the main server will respond with a denial of the request.
In particular, if the prefix of the room code does not match any of S, D, or U, after the client sends a request to the main server, the main server will not forward the request to the backend. Instead, it will directly return an "invalid room code" response to the client. This is a error message set by me, and the client will display a msg on the screen and prompt the user to input the correct room code next.




<2>For main server:serverM.cpp
serverM.cpp need tcp connection with client.cpp，and need a udp for backend server. He also needs to support multiple clients communicating simultaneously. My serverM.cpp mainly has the following functions:
1.map<string, string> readSaveFile(const string& filename)：read member file to a map
2.void Availabilityudprequest(int sockudp, const string& request_msg, const struct sockaddr_in& tcp_server_addr, socklen_t server_addr_len, int client_fd, string id): 
    First, establish UDP communication with the backend server, send a request, and after receiving a reply from the backend server, forward the reply content to the client via TCP.
3.void Reservationudprequest(string roomcode,int sockudp, const string& request_msg, const struct sockaddr_in& tcp_server_addr, socklen_t server_addr_len, int client_fd, string id,map<string,int>roommap): 
    Use sendto() send request to backend server and use recvfrom to receive the response from backend server. After receiving the backend server's reply, parse the reply. If the reply indicates a successful reservation, decrement the count of the corresponding room number in the room status map maintained in serverM by 1. This step is independent of the actions taken towards sending to the client and the backend. We won't use this map during communication; thus, we will still send requests to the backend every time we receive a client request instead of querying this map. The change in this map is solely for meeting the requirement stated in the project description, where the main server needs to maintain the status of rooms, which changes according to the actual number of rooms. Finally, we will use TCP to send the result to the client.

int main():
1use readSaveFile to read member.txt
2.create udp and set. finish the address setting for 3 backend server. Use a while loop to receive the msg about room status from 3 backend server after booting up .
3.create tcp and set. Use while(1) loop for tcp。Here I use fork(), citing form beej, after accept. This fork() can help realize the multiple client connection. 
4.In while(1)loop, recv the authentication request sent from client, If the authentication result is a failure, send a request to the client to input again., until successful authentication or user sent a guest request to main serve er. Then use another while(1) loop to deal with client’s Availability/reservation request，if client’s request is Availability，according to roomcode prefix determine which backend should ew send to，receiving backend response and sent back to client. If room code prefix is not belong to S,D,U. then we need to send a ‘invalid code’ to  client instead of backend server. In the case of reservation, for members and Availability, the process is similar, but it involves calling Reservationudprequest(). If it's a guest, directly send an explanation "not a valid backend prefix" to the client and do not send a request to the backend.



<3>For backend: serverS.cpp, serverD.cpp, serverU.cpp
For 3 backend server, the cpp file is same in code part except for the parameters I've defined. The differences in parameter parts for each backend are as follows:
PORT: The UDP port for each backend server.
id: The identifier for each backend (S, D, U).
filename: The filename for each backend to read from.

Backend is primarily used for UDP interaction with the main server, obtaining the main server's requirements, and responding accordingly. My Backend consists of the following functions:
1.map<string, int> readSaveFile(const string& filename): read in file and save in a map.
2.string handleAvailabilityRequest(string roomcode,map<string,int> &roommap)：Processes Availability Request sent by main server. Searching for roomcode in the map. This function will get a string msg which is ready to send.
3.void handleReservationRequest(string roomcode,map<string,int> &uRoomMap, sockaddr_in bdserver, socklen_t bdserverize, int sockfd):Processes Reservation Request sent by main server. This function i use int socket as parameters, complete the whole process including Reservation send and recv, processing and sending response, including -1 at the corresponding room number in map.
4.int newudp(): create a new udp。this part comes from beej
5.Int main()：Executes the entire process.  First  create a new udp，set and sent msg to the address for mianserver
Send the initial room status. After receiving requests from the main server, parse them. Based on the action, select either handleReservationRequest or handleAvailabilityRequest for processing. Generate the text to be sent and send the response back to the main server.


e. The format of all the messages exchanged, e.g., username and password are concatenated and delimited by a comma, etc.
1.a space is used as a delimiter between the username and password to avoid special characters like commas in the password. 
2.When the backend sends the initial room dynamics to the main server, semicolons are used. 
3.Other msg are separated by commas.
4.msg table:
backend:
    <1>1st sent room status to main after booting up: use ‘,’between room code and room number, use ‘;’at the end of each line.
    <2>Send msg for availability to main:
        For room code not exist in map: ’Notabletofind’ 
        For room code exists in map and the number is greater than 0: ‘Available'
        For room code exists in map but the number is 0: “Notavailable”;
    <3>  Send msg for reservation to main:
        For room code not exist in map:’n’
        For room code exists in map and the number is greater than 0:’t’
        For room code exists in map but the number is 0:’f’
Main:
udp:
    Sent msg to backend for availability :”a,”+roomcode;
    Sent msg to backend for reservation:”r,”+roomcode;
Tcp:
    Sent msg to client for authentication response:
        User not exist in member list: ‘Usernotfound’
        User exist in member list but the password is not the same as the password in       list:’Incorrectpassword’
        User exist in the list and the password is right:’Authenticated’
        User guest response:’Guest’
    Sent msg to client for availability:
        For room code not exist in map: ’Notabletofind’ 
        For room code exists in map and the number is greater than 0: ‘Available'
        For room code exists in map but the number is 0: “Notavailable”;
        For invalid room code: ‘eb’
    Sent msg to client for reservation:
        For room code not exist in map:’n’
        For room code exists in map and the number is greater than 0:’t’
        For room code exists in map but the number is 0:’f’
        For guest sent err msg:’gnr’
        For invalid room code: ‘eb’
Client:
    <1>Sent msg to main server for authentication(member): encryptusername+" "+encryptpassword 
    <2>Sent msg to main server for authentication(guest):encryptusername
    <3>Sent msg to main server for availability room request: "a,"+roomcode+","+"member";
    <4>Sent msg to main server for reservation room request for member: "r,"+roomcode+","+"member";
    <5>Sent msg to main server for reservation room request for guest: "r,"+roomcode+","+"guest";
f. Any idiosyncrasy of your project:
In extremely occasional situation(which I encounter only once or twice during testing), bind failures may occur when I boot up sequenced CPP files, which may be related to things like zombie processes. The error will no longer appear after ctrl+C quitting and restarting all the cpp files.

g. Reused Code: 
1.I learn"find_first_not_of()" from chatgpt search result and use it in client.cpp authentication() part for 'enter skip' case.Also I use it in serverM.cpp readSaveFile() for handlding the comma with space between username and password.
2.The code referenced from beej have been annotated in the code.(my code about socket create, bind,set address, listen, accept, connect, send and receive part, parse response using char[] are from beej)