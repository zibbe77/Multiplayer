#include "Network.h"

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

    // if we pass less arguents than 3 we close the program
    if (argc < 3)
    {
        fprintf(stderr, "usage: tcp_client hostname port\n");
        return 1;
    }

    printf("Configuring remote address...\n");

    struct addrinfo hints;
    // this clears hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;

    // First paremter host name / ip
    // Second paremter service port or protical
    // Third addrinfo of hits in this case we input SOCK_STREAM to indecate that we want to use TCP
    // Lastly a pointer to the
    struct addrinfo *peer_address;
    if (getaddrinfo(argv[1], argv[2], &hints, &peer_address))
    {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Remote address is: ");

    char address_buffer[100];
    char service_buffer[100];

    // this does the revers of getaddrinfo it converts addrinfo back to strings in a protocl independedt way
    // debuging stage
    getnameinfo(
        peer_address->ai_addr,
        peer_address->ai_addrlen,
        address_buffer, sizeof(address_buffer),
        service_buffer, sizeof(service_buffer),
        NI_NUMERICHOST);

    printf("%s %s\n", address_buffer, service_buffer);

    printf("Creating socket...\n");
    // SOCKET is used becuse on windows its an unsinde int
    SOCKET socket_peer;

    // first is the set the protical ipv4 or ipv6
    // seckond is the typ udp or tcp
    // third is the protical used for the socket 0 takes one that will work
    socket_peer = socket(
        peer_address->ai_family,
        peer_address->ai_socktype,
        peer_address->ai_protocol);

    if (!ISVALIDSOCKET(socket_peer))
    {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Connecting...\n");

    // first is the socket to connect to
    // seckond is the remote address -- the socket adress
    // third is the remote address lenght
    if (connect(socket_peer,
                peer_address->ai_addr,
                peer_address->ai_addrlen))
    {
        fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    freeaddrinfo(peer_address);

    printf("Connected.\n");
    printf("To send data, enter text followed by enter.\n");

    while (1)
    {
        // a file descriptor
        fd_set reads;
        // zeros it
        FD_ZERO(&reads);

        // set to mointer socket
        FD_SET(socket_peer, &reads);

#if !defined(_WIN32)
        // 0 is the file discritor for stdin this is the same as FD_SET(fileno(stdin), &reads);
        // this moniters for terminal inputs
        FD_SET(0, &reads);
#endif

        // sets a time out on 100000 ms
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        // select inducateds witch file descriptor thats ready for reading
        // first is the socket idk why + 1
        // the seckond, third and fourth is file discritor that it can track
        // lastly its a timeout

        if (select(socket_peer + 1, &reads, 0, 0, &timeout) < 0)
        {
            fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
            return 1;
        }

        // this makro returns true if the first parameter is a file descriptor that is set
        if (FD_ISSET(socket_peer, &reads))
        {
            char read[4096];
            // reads the msg form the socket
            int bytes_received = recv(socket_peer, read, 4096, 0);
            if (bytes_received < 1)
            {
                printf("Connection closed by peer.\n");
                break;
            }
            printf("Received (%d bytes): %.*s",
                   bytes_received, bytes_received, read);
        }

#if defined(_WIN32)
        if (_kbhit())
        {
#else
        if (FD_ISSET(0, &reads))
        {
#endif
            char read[4096];
            if (!fgets(read, 4096, stdin))
            {
                break;
            }
            printf("Sending: %s", read);

            int bytes_sent = send(socket_peer, read, strlen(read), 0);
            printf("Sent %d bytes.\n", bytes_sent);
        }
    }
    printf("Closing socket...\n");
    CLOSESOCKET(socket_peer);
//--------------------------------
// cleanup dependencies for win
#if defined(_WIN32)
    WSACleanup();
#endif

    printf("Finished.\n");

    return 0;
}