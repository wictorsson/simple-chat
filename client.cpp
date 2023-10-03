#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <string>

void receiveMessages(int clientSocket) {
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));

    while (true) {
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            std::cout << "Server disconnected." << std::endl;
            break;
        }
        std::cout << "\n" << buffer << std::endl;
        memset(buffer, 0, sizeof(buffer)); 
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <username> <server-ip>:<server-port>" << std::endl;
        return 1;
    }

    std::string username = argv[1];
    std::string serverAddressInput = argv[2];

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error creating client socket." << std::endl;
        return 1;
    }

    // EXTRACT server IP and port from user input
    char serverIp[128];
    int serverPort;
    if (sscanf(serverAddressInput.c_str(), "%[^:]:%d", serverIp, &serverPort) != 2) {
        std::cerr << "Invalid server address format. Use <server-ip>:<server-port>." << std::endl;
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr(serverIp);

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error connecting to server." << std::endl;
        return 1;
    }
    // CREATE a thread to receive messages from the server
    std::thread receiveThread(receiveMessages, clientSocket);
     // DETACH the thread to run it separately
    receiveThread.detach();

    while (true) {
        std::cout << "Message: ";
        std::string message;
        std::getline(std::cin, message);
        // COMBINE username and message
        std::string messageWithUsername = username + ": " + message;
        // SEND the message to the server
        send(clientSocket, messageWithUsername.c_str(), messageWithUsername.length(), 0);
    }

    close(clientSocket);
    return 0;
}
