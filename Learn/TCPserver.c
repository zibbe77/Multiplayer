#include "Network.h"
#include <ctype.h>

int main(int argc, char *argv[])
{
// setup dependencies for win
#if defined(_WIN32)
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d))
    {
        fprintf(stderr, "Failed to initialize.\n");
        return 1;
    }
#endif

    // program
    //--------------------------------

    printf("Configuring local address...\n");

    // setings
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // binde address
    struct addrinfo *bind_address;
    getaddrinfo(0, "5400", &hints, &bind_address);
    printf("Creating socket...\n");

    // socket to lisen too
    SOCKET socket_listen;
    socket_listen = socket(
        bind_address->ai_family,
        bind_address->ai_socktype,
        bind_address->ai_protocol);

    // error if socket fail
    if (!ISVALIDSOCKET(socket_listen))
    {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Binding socket to local address...\n");

    // Traditionally, this operation is called “assigning a name to a socket”.
    if (bind(
            socket_listen,
            bind_address->ai_addr,
            bind_address->ai_addrlen))
    {
        fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    // cleaing
    freeaddrinfo(bind_address);

    // listen to the socket
    printf("Listening...\n");
    if (listen(socket_listen, 10) < 0)
    {
        fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    // stores all active sockets
    fd_set master;
    FD_ZERO(&master);
    FD_SET(socket_listen, &master);
    SOCKET max_socket = socket_listen;

    while (1)
    {
        // copys master. We lose it if we dont copy
        fd_set reads;
        reads = master;

        if (select(max_socket + 1, &reads, 0, 0, 0) < 0)
        {
            fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
            return 1;
        }

        for (SOCKET i = 1; i <= max_socket; ++i)
        {
            // checks if the socket is ready to be lisen to
            if (FD_ISSET(i, &reads))
            {

                if (i == socket_listen)
                {
                    struct sockaddr_storage client_address;
                    socklen_t client_len = sizeof(client_address);

                    // accepts a connetsion
                    SOCKET socket_client = accept(
                        socket_listen,
                        (struct sockaddr *)&client_address,
                        &client_len);
                    // on fail crash
                    if (!ISVALIDSOCKET(socket_client))
                    {
                        fprintf(stderr, "accept() failed. (%d)\n",
                                GETSOCKETERRNO());
                        return 1;
                    }

                    // Saves the new connetsion in master
                    FD_SET(socket_client, &master);

                    // maintains max sockets
                    if (socket_client > max_socket)
                    {
                        max_socket = socket_client;
                    }

                    // prints the address of the clinet
                    char address_buffer[100];
                    getnameinfo((struct sockaddr *)&client_address,
                                client_len,
                                address_buffer, sizeof(address_buffer), 0, 0,
                                NI_NUMERICHOST);
                    printf("New connection from %s\n", address_buffer);
                }
                else
                {
                    // if the socket connetsion is established

                    char read[1024];
                    int bytes_received = recv(i, read, 1024, 0);

                    // closes the connectsion
                    if (bytes_received < 1)
                    {
                        // clears the file descriptor
                        FD_CLR(i, &master);
                        CLOSESOCKET(i);
                        continue;
                    }

                    // Content
                    //----------------
                    for (int j = 0; j < bytes_received; ++j)
                    {
                        read[j] = toupper(read[j]);
                    }
                    send(i, read, bytes_received, 0);
                    //-----------------
                }
            }
        }
    }

    // closes socket
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