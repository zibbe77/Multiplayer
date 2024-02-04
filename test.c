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

    //-------------------------------
    time_t timer;
    time(&timer);
    printf("Local time is: %s", ctime(&timer));

//--------------------------------
// cleanup dependencies for win
#if defined(_WIN32)
    WSACleanup();
#endif
    return 0;
}
