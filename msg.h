#pragma once

#define BACKEND_MSG_BOOT_UP "The Server %s is up and running using UDP on port %d.\n"
#define BACKEND_MSG_SEND_ROOM_STATUS "The Server %s has sent the room status to the main server.\n"
#define BACKEND_MSG_AVAILABILITY_REQ "The Server %s received an availability request from the main server.\n"
#define BACKEND_MSG_AVAILABLE "Room %s is available.\n"
#define BACKEND_MSG_NOT_AVAILABLE "Room %s is not available.\n"
#define BACKEND_MSG_AVAILABILITY_NO_ROOM "Not able to find the room layout.\n"
#define BACKEND_MSG_AVAILABILITY_RESP "The Server %s finished sending the response to the main server.\n"
#define BACKEND_MSG_RESERVE_REQ "The Server %s received a reservation request from the main server.\n"
#define BACKEND_MSG_RESERVE_SUC "Successful reservation. The count of Room %s is now %d.\n"
#define BACKEND_MSG_RESERVE_FAIL "Cannot make a reservation. Room %s is not available.\n"
#define BACKEND_MSG_RESERVE_NO_ROOM "Cannot make a reservation. Not able to find the room layout.\n"
#define BACKEND_MSG_RESERVE_RESP_UPDATED "The Server %s finished sending the response and the updated room status to the main server.\n"
#define BACKEND_MSG_RESERVE_RESP "The Server %s finished sending the response to the main server.\n"

#define MAIN_MSG_BOOT_UP "The main server is up and running.\n"
#define MAIN_MSG_RECEIVE_ROOM_STATUS "The main server has received the room status from Server %s using UDP over port %d.\n"
#define MAIN_MSG_LOGIN_REQ "The main server received the authentication for %s using TCP over port %d\n"
#define MAIN_MSG_LOGIN_RESP "The main server sent the authentication result to the client.\n"
#define MAIN_MSG_GUEST_REQ "The main server received the guest request for %s using TCP over port %d.The main server accepts %s as a guest.\n"
#define MAIN_MSG_GUEST_RESP "The main server sent the guest response to the client.\n"
#define MAIN_MSG_AVAILABILITY_REQ "The main server has received the availability request on Room %s from %s using TCP over port %d.\n"
#define MAIN_MSG_AVAILABILITY_FORWARD_REQ "The main server sent a request to Server %s.\n"
#define MAIN_MSG_RECEIVE_AVAILABILITY_RESP "The main server received the response from Server %s using UDP over port %d.\n"
#define MAIN_MSG_SEND_AVAILABILITY_RESP "The main server sent the availability information to the client.\n"
#define MAIN_MSG_RESERVE_REQ "The main server has received the reservation request on Room %s from %s using TCP over port %d.\n"
#define MAIN_MSG_RESERVE_GUEST "%s cannot make a reservation.\n"
#define MAIN_MSG_RESERVE_ERROR_RESP "The main server sent the error message to the client.\n"
#define MAIN_MSG_RESERVE_FORWARD_REQ "The main server sent a request to Server %s.\n"
#define MAIN_MSG_RECEIVE_RESERVE_RESP_UPDATED "The main server received the response and the updated room status from Server %s using UDP over port %d.\n"
#define MAIN_MSG_RECEIVE_RESERVE_RESP "The main server received the response from Server %s using UDP over port %d.\n"
#define MAIN_MSG_ROOM_UPDATED "The room status of Room %s has been updated.\n"
#define MAIN_MSG_SEND_RESERVE_RESP "The main server sent the reservation result to the client.\n"

#define CLIENT_MSG_BOOT_UP "Client is up and running.\n"
#define CLIENT_MSG_ENTER_USERNAME "Please enter the username: "
#define CLIENT_MSG_ENTER_PASSWORD "Please enter the password: "

#define CLIENT_MSG_ENTER_ROOM "Please enter the room code: "
#define CLIENT_MSG_ENTER_ACTION "Would you like to search for the availability or make a reservation? (Enter “Availability” to search for the availability or Enter “Reservation” to make a reservation ): "
#define CLIENT_MSG_AVAILABILITY_REQ "%s sent an availability request to the main server.\n"
#define CLIENT_MSG_AVAILABLE_RESP "The client received the response from the main server using TCP over port %d. The requested room is available.\n\n-----Start a new request-----\n"
#define CLIENT_MSG_NOT_AVAILABLE_RESP "The client received the response from the main server using TCP over port %d. The requested room is not available.\n\n-----Start a new request-----\n"
#define CLIENT_MSG_AVAILABILITY_NO_ROOM "The client received the response from the main server using TCP over port %d. Not able to find the room layout.\n\n-----Start a new request-----\n"
#define CLIENT_MSG_RESERVE_REQ "%s sent a reservation request to the main server.\n"

#define MEMBER_MSG_LOGIN_REQ "%s sent an authentication request to the main server.\n"
#define MEMBER_MSG_LOGIN_SUC "Welcome member %s!\n"
#define MEMBER_MSG_LOGIN_FAIL_USERNAME "Failed login: Username does not exist.\n"
#define MEMBER_MSG_LOGIN_FAIL_PWD "Failed login: Password does not match.\n"
#define MEMBER_MSG_RESERVE_SUC "The client received the response from the main server using TCP over port %d. Congratulation! The reservation for Room %s has been made. \n\n-----Start a new request-----\n"
#define MEMBER_MSG_RESERVE_FAIL "The client received the response from the main server using TCP over port %d. Sorry! The requested room is not available.\n\n-----Start a new request-----\n"
#define MEMBER_MSG_RESERVE_NO_ROOM "The client received the response from the main server using TCP over port %d. Oops! Not able to find the room.\n\n-----Start a new request-----\n"

#define GUEST_MSG_GUEST_REQ "%s sent a guest request to the main server using TCP over port %d.\n"
#define GUEST_MSG_RECEIVE_RESP "Welcome guest %s!\n"
#define GUEST_MSG_RESERVE_RESP "Permission denied: Guest cannot make a reservation.\n"

//edge case handling
#define CLIENT_INVALID_USERNAME_INPUT "Length of username should be > 0 and <= %d\n"
#define CLIENT_INVALID_PWD_INPUT "Length of password should be <= %d\n"
#define CLIENT_INVALID_ROOM_INPUT "Length of room code should be <= %d\n"
#define CLIENT_INVALID_ACTION_INPUT "Enter “Availability” to search for the availability or Enter “Reservation” to make a reservation\n"