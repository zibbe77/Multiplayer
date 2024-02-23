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

    printf("Configuring local address...\n");

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    // We use SOCK_DGRAM for UDP
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bind_address;
    getaddrinfo(0, "5400", &hints, &bind_address);

    printf("Creating socket...\n");
    SOCKET socket_listen;
    socket_listen = socket(
        bind_address->ai_family,
        bind_address->ai_socktype,
        bind_address->ai_protocol);

    if (!ISVALIDSOCKET(socket_listen))
    {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Binding socket to local address...\n");
    if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen))
    {
        fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    freeaddrinfo(bind_address);

    // starts to lisen to connectsions
    struct sockaddr_storage client_address;
    socklen_t client_len = sizeof(client_address);
    char read[1024];

    // we resive the address of were i came from here

    // socket - specifes the socket
    // buffet - Where we want to save the msg
    // length - the length of the bufferts in bytes
    // flags
    // address - where the senders info shuld be save
    // address length

    int bytes_received = recvfrom(socket_listen, read, 1024, 0, (struct sockaddr *)&client_address, &client_len);

    printf("Received (%d bytes): %.*s\n", bytes_received, bytes_received, read);

    printf("Remote address is: ");
    char address_buffer[100];
    char service_buffer[100];

    // (NI_NUMERICHOST | NI_NUMERICSERV) tells getnameinfo() that we want both the client address and port number to be in numeric form
    // if we want the protical name you pas NI_DGRAM insted (UDP)
    getnameinfo(((struct sockaddr *)&client_address),
                client_len,
                address_buffer, sizeof(address_buffer),
                service_buffer, sizeof(service_buffer),
                NI_NUMERICHOST | NI_NUMERICSERV);

    printf("%s %s\n", address_buffer, service_buffer);

    CLOSESOCKET(socket_listen);

//--------------------------------
// cleanup dependencies for win
#if defined(_WIN32)
    WSACleanup();
#endif

    printf("Finished.\n");

    return 0;
}