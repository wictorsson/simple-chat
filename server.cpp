#include <iostream>
#include <cstring>
#include <vector>
#include <thread>
#include <algorithm>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

std::vector<int> clientSockets;

void broadcastMessage(const char* message, int senderSocket) {
    for (int clientSocket : clientSockets) {
        if (clientSocket != senderSocket) {
            send(clientSocket, message, strlen(message), 0);
        }
    }
}

void handleClient(int clientSocket) {
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));

    // ADD the client socket to the list of connected clients
    clientSockets.push_back(clientSocket);

    while (true) {
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            std::cout << "Client " << clientSocket << " disconnected." << std::endl;
            break;
        }
        std::cout << "Client " << clientSocket << ": " << buffer << std::endl;
        // Broadcast the message to all clients
        broadcastMessage(buffer, clientSocket);
        memset(buffer, 0, sizeof(buffer)); // Clear the buffer
    }
    clientSockets.erase(std::remove(clientSockets.begin(), clientSockets.end(), clientSocket), clientSockets.end());
    close(clientSocket);
}

int main() {
    // CREATE a socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating server socket." << std::endl;
        return 1;
    }

    // BIND the socket to port
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080); 
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error binding socket." << std::endl;
        return 1;
    }

    // LISTEN for incoming connections
    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Error listening for connections." << std::endl;
        return 1;
    }

    std::cout << "Server listening on port 8080..." << std::endl;

    while (true) {
        // ACCEPT incoming client connections
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);

        if (clientSocket == -1) {
            std::cerr << "Error accepting connection." << std::endl;
            continue;
        }
        // CREATE a thread to handle the client
        std::thread clientThread(handleClient, clientSocket);
        // DETACH the thread to allow handling multiple clients 
        clientThread.detach(); 
    }
    close(serverSocket);
    return 0;
}
