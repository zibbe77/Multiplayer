// network include
// -----------------------------
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

// this is needed on windos to indicate if a terminal input is wating
#if defined(_WIN32)
#include <conio.h>
#endif

// Needs to be check for windows - includes
// -----------------------------
#include <pthread.h>

#include "raylib.h"
#include "raymath.h"

// normal includes
// -----------------------------

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>