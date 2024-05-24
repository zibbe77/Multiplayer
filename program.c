#include "program.h"

typedef struct ClientConnectInfo
{
    char port[5];
    char address[17];
} ClientConnectInfo;

int gameState = 0;

// player

int player1PosX = 100;
int player1PosY = 100;

int player2PosX = 100;
int player2PosY = 100;

int player = 0;

pthread_mutex_t mutex;

// Connection variables

char *serverPort = "8080";
char *serverAdress = "127.0.0.1";
char *clientConnectPort = "8000";

// Server Stuff
//----------------------
void *serverThreed()
{
    printf("Configuring local address...\n");

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bind_address;
    getaddrinfo(0, serverPort, &hints, &bind_address);

    printf("Creating socket...\n");
    SOCKET socket_listen = socket(
        bind_address->ai_family,
        bind_address->ai_socktype,
        bind_address->ai_protocol);

    if (!ISVALIDSOCKET(socket_listen))
    {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        pthread_exit(NULL);
    }

    printf("Binding socket to local address...\n");

    if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen))
    {
        fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
        pthread_exit(NULL);
    }
    freeaddrinfo(bind_address);

    fd_set master;
    FD_ZERO(&master);
    FD_SET(socket_listen, &master);
    SOCKET max_socket = socket_listen;

    printf("Waiting for connections...\n");

    while (1)
    {
        fd_set reads;
        reads = master;
        if (select(max_socket + 1, &reads, 0, 0, 0) < 0)
        {
            fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
            pthread_exit(NULL);
        }

        if (FD_ISSET(socket_listen, &reads))
        {
            struct sockaddr_storage client_address;
            socklen_t client_len = sizeof(client_address);
            char read[1024];
            int bytes_received = recvfrom(socket_listen, read, 1024, 0, (struct sockaddr *)&client_address, &client_len);
            // if (bytes_received < 1)
            // {
            //     fprintf(stderr, "connection closed. (%d)\n",
            //             GETSOCKETERRNO());
            //     pthread_exit(NULL);
            // }

            if (bytes_received >= 1)
            {
                pthread_mutex_lock(&mutex);
                switch (player)
                {
                case 1:
                    player2PosY = *((int *)&read[8]);
                    player2PosX = *((int *)&read[12]);
                    break;
                case 2:
                    player1PosY = *((int *)&read[0]);
                    player1PosX = *((int *)&read[4]);
                    break;
                }

                pthread_mutex_unlock(&mutex);

                // printf("Received (%d bytes): %.*s \n", bytes_received, bytes_received, read);
                printf("x - %i y - %i \n", player1PosX, player1PosY);
                // printf("x - %i y - %i \n", player2PosX, player2PosY);

                // for (int j = 0; j < bytes_received; ++j)
                // {
                //     read[j] = toupper(read[j]);
                // }

                // sendto(socket_listen, read, bytes_received, 0, (struct sockaddr *)&client_address, client_len);
            }
        }
    }

    printf("Closing listening socket...\n");
    CLOSESOCKET(socket_listen);
    printf("Finished.\n");
}

// Client stuff
//----------------------
void *clientThreed()
{
    ClientConnectInfo clientInfo;

    for (char i = 0; i < 5; i++)
    {
        clientInfo.port[i] = clientConnectPort[i];
    }
    for (char i = 0; i < 17; i++)
    {
        clientInfo.address[i] = serverAdress[i];
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
    if (getaddrinfo(clientInfo.address, clientInfo.port, &hints, &peer_address))
    {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        pthread_exit(NULL);
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
        pthread_exit(NULL);
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
        pthread_exit(NULL);
    }
    freeaddrinfo(peer_address);

    printf("Connected.\n");

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
            pthread_exit(NULL);
        }
        char read[4096];

        pthread_mutex_lock(&mutex);
        *((int *)&read[0]) = player1PosY;
        *((int *)&read[4]) = player1PosX;
        *((int *)&read[8]) = player2PosY;
        *((int *)&read[12]) = player2PosX;

        pthread_mutex_unlock(&mutex);

        int bytes_sent = send(socket_peer, read, 16, 0);
        // printf("Sent %d bytes.\n", bytes_sent);
    }
    printf("Closing socket...\n");
    CLOSESOCKET(socket_peer);
}

int main()
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

    InitWindow(1000, 1000, "game");
    SetTargetFPS(120);

    pthread_mutex_init(&mutex, NULL);

    pthread_t t1;
    pthread_t t2;

    while (!WindowShouldClose())
    {
        switch (gameState)
        {
        case 0:
            if (IsKeyDown(KEY_G))
            {
                gameState = 1;
                player = 1;
                serverPort = "8080";
                clientConnectPort = "8000";
                pthread_create(&t1, NULL, &serverThreed, NULL);
            }
            if (IsKeyDown(KEY_H))
            {
                gameState = 1;
                player = 2;
                serverPort = "8000";
                clientConnectPort = "8080";
                pthread_create(&t1, NULL, &serverThreed, NULL);
            }

            // Draw
            BeginDrawing();

            DrawText("Press G for player 1 and H for player 2", 200, 200, 20, GREEN);

            EndDrawing();

            break;
        case 1:

            if (IsKeyDown(KEY_T))
            {
                switch (player)
                {
                case 1:
                    serverAdress = "127.0.0.1";
                    pthread_create(&t2, NULL, &clientThreed, NULL);
                    gameState = 2;
                    break;
                case 2:
                    // Change server adress here
                    serverAdress = "127.0.0.1";
                    pthread_create(&t2, NULL, &clientThreed, NULL);
                    gameState = 2;
                    break;
                }
            }
            BeginDrawing();

            ClearBackground(BLACK);
            switch (player)
            {
            case 1:
                DrawText("Player 1", 200, 100, 20, WHITE);
                break;
            case 2:
                DrawText("Player 2", 200, 100, 20, WHITE);
                break;
            }

            DrawText("Press T to start", 200, 300, 20, GREEN);

            EndDrawing();

            break;
        case 2:
            switch (player)
            {
            case 1:
                if (IsKeyDown(KEY_W))
                {
                    pthread_mutex_lock(&mutex);
                    player1PosY--;
                    pthread_mutex_unlock(&mutex);
                }
                if (IsKeyDown(KEY_S))
                {
                    pthread_mutex_lock(&mutex);
                    player1PosY++;
                    pthread_mutex_unlock(&mutex);
                }
                if (IsKeyDown(KEY_D))
                {
                    pthread_mutex_lock(&mutex);
                    player1PosX++;
                    pthread_mutex_unlock(&mutex);
                }
                if (IsKeyDown(KEY_A))
                {
                    pthread_mutex_lock(&mutex);
                    player1PosX--;
                    pthread_mutex_unlock(&mutex);
                }
                break;
            case 2:
                if (IsKeyDown(KEY_I))
                {
                    pthread_mutex_lock(&mutex);
                    player2PosY--;
                    pthread_mutex_unlock(&mutex);
                }
                if (IsKeyDown(KEY_K))
                {
                    pthread_mutex_lock(&mutex);
                    player2PosY++;
                    pthread_mutex_unlock(&mutex);
                }
                if (IsKeyDown(KEY_L))
                {
                    pthread_mutex_lock(&mutex);
                    player2PosX++;
                    pthread_mutex_unlock(&mutex);
                }
                if (IsKeyDown(KEY_J))
                {
                    pthread_mutex_lock(&mutex);
                    player2PosX--;
                    pthread_mutex_unlock(&mutex);
                }
                break;
            }

            BeginDrawing();
            ClearBackground(BLACK);

            DrawCircle(player1PosX, player1PosY, 20, BLUE);
            DrawCircle(player2PosX, player2PosY, 20, RED);

            EndDrawing();
            break;

        default:
            break;
        }
    }

//--------------------------------
// cleanup dependencies for win
#if defined(_WIN32)
    WSACleanup();
#endif
    pthread_join(t1, NULL);
    printf("Cosing program");
    return 0;
}