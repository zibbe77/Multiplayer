/*sock_init.c*/
//-----------------------------

#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
// tells visual c compiler to use Win sock libary ws2_32.lib
//  if you use MinGW pragma is ignored and you need to compile whit -lws2_32 option
#pragma comment(lib, "ws2_32.lib")

#else
// mac cool dont need setup. Uses berkeley sockets.
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#endif

// socket in windows returns a unsingde int and in onix system it returns a int
#if !defined(_WIN32)
#define SOCKET int
#endif

// socket returns INVALID_SOCKET if it fails on windos and negativ number on Onix. But SOCKET is an unsignid int on windows
#if defined(_WIN32)
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#else
#define ISVALIDSOCKET(s) ((s) >= 0)
#endif

// windows need a special funtion to close sockets
#if defined(_WIN32)
#define CLOSESOCKET(s) closesocket(s)
#else
#define CLOSESOCKET(s) close(s)
#endif

// On windows you retrive errors with the WSAGetLastError fuktion and on Onix errno
#if defined(_WIN32)
#define GETSOCKETERRNO() (WSAGetLastError())
#else
#define GETSOCKETERRNO() (errno)
#endif

// normal includes
//-----------------------------

#include <stdio.h>
#include <time.h>
#include <string.h>

int main()
{
// setup dependencies for win
#if defined(_WIN32)
    WSADATA d;
    // MAKEWORD requsts winsock 2.2
    // WSAstartup initalize winsock
    if (WSAstartup(MAKEWORD(2, 2), &d))
    {
        fprintf(stderr, "Failed to initialize.\n");
        return 1;
    }
#endif
    printf("Ready to use socket \n");
    // program
    //--------------------------------

    printf("Configuring local address...\n");
    struct addrinfo hints;
    // zero out it
    memset(&hints, 0, sizeof(hints));
    // sets it to ipv4 . If it was AF_INET6 it would use ipv6
    hints.ai_family = AF_INET;
    // Specifiy we will use TCP . If we whant UDP SOCK_DGRAM
    hints.ai_socktype = SOCK_STREAM;
    // We are telling getaddrinfo that we what to binde to the wildcard adress and asking it to
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bind_address;

    // first parimater is hostname / adress
    // seckend is port number or servis idk what that is
    // third hints in the form of a addrinfo or null
    // lastly res is the output of the requsted information
    getaddrinfo(0, "5250", &hints, &bind_address);

    printf("Creating socket...\n");

    // SOCKET is the makro we made for the difrent verstions
    // on onix this is a int and on windos its a Winsock type
    SOCKET socket_listen;

    // socket creats a socket for ur program
    socket_listen = socket(
        bind_address->ai_family,
        bind_address->ai_socktype,
        bind_address->ai_protocol);

    // this is a makro
    //  on windows this is a windsock type and on onix this is a int
    if (!ISVALIDSOCKET(socket_listen))
    {
        // prints ur makro for error
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Binding socket to local address...\n");

    // bindes a local ip and port to a socket
    // bind returns a 0 on succss and a non zero on fail
    if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen))
    {
        fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    // we are done with it
    freeaddrinfo(bind_address);

    printf("Listening...\n");

    // First argument is the socket that we want to lisen to and the second is how many queued connections we allow.
    // returns a 0 no success and a -1 on fail
    if (listen(socket_listen, 10) < 0)
    {
        fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Waiting for connection...\n");

    // https://stackoverflow.com/questions/16010622/reasoning-behind-c-sockets-sockaddr-and-sockaddr-storage
    // its for storing but in a protocal independet way
    struct sockaddr_storage client_address;

    // By declaring specific types for these fields, it decouples them from a particular representation like unsigned int.
    // So a guess its for storing the length
    socklen_t client_len = sizeof(client_address);

    // is used to creat a new socket for the first incoming connection from the listen()
    // socket - specifes a socket that was created with socket() and was bind() and has issued a call to listen()
    // address - null for no returns or a pointer to a sockaddr structure
    // address_len - Takes the length of the adress and returns the length of the stored address
    SOCKET socket_client = accept(
        socket_listen,
        (struct sockaddr *)&client_address,
        &client_len);

    if (!ISVALIDSOCKET(socket_client))
    {
        fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    /*time_server.c continued*/
    printf("Client is connected... ");

    // this logs the clients info in the console
    //  IP and hostname since this is for a web server

    // we need to pas in the length becuse this function works on ipv4 and ipv6
    char address_buffer[100];
    getnameinfo(
        (struct sockaddr *)&client_address,
        client_len, address_buffer,
        sizeof(address_buffer),
        0,
        0,
        NI_NUMERICHOST);

    printf("%s\n", address_buffer);

    printf("Reading request...\n");

    char request[1024];

    // First parameter is the socket
    // Second is the buffert where the msg should get stored
    // Third is the lenght of the buffert
    // Lastly is flags we have non so we pas 0
    int bytes_received = recv(socket_client, request, 1024, 0);
    printf("Received %d bytes.\n", bytes_received);

    // we are telling c to print bytes_received chars of request becuse we cant know if reqest is null termineted
    printf("%.*s", bytes_received, request);

    printf("Sending response...\n");
    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "Content-Type: text/plain\r\n\r\n"
        "Local time is: ";

    // first paremeter is what socket to use
    // second is the buffer for the response msg
    // third is the lenght of the response msg
    // btw strlen givs the lenght of the string given to it. string.h

    int bytes_sent = send(socket_client, response, strlen(response), 0);

    printf("Sent %d of %d bytes.\n", bytes_sent, (int)strlen(response));

    // gets time and saves it
    time_t timer;
    time(&timer);
    char *time_msg = ctime(&timer);

    // sends it but agin ???
    // i guess it dosent mather to send all in same send()
    bytes_sent = send(socket_client, time_msg, strlen(time_msg), 0);
    printf("Sent %d of %d bytes.\n", bytes_sent, (int)strlen(time_msg));

    // closes socket
    // makro becuse its diffrent on windows and onix
    CLOSESOCKET(socket_client);

    // here we can accept more request but this is a exemple so we dont

    printf("Closing listening socket...\n");
    CLOSESOCKET(socket_listen);

//--------------------------------
// cleanup dependencies for win
#if defined(_WIN32)
    WSACleanup();
#endif

    printf("Finished.\n");

    return 0;
}
