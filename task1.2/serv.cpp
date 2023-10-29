#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <vector>

struct ThreadData {
    int clientSocket; // ��������� ��� �������� ������ ������ ��������� �������
};

std::vector<int> clientSockets; // ������ ��� �������� ������� ��������

void SendMessageToAllClients(const std::string& message) {
    // �������� ��������� ���� ��������
    for (int clientSocket : clientSockets) {
        send(clientSocket, message.c_str(), message.size(), 0);
    }
}

void BroadcastMessage(const std::string& message, int senderSocket) {
    // �������� ��������� ���� �������� ����� �����������
    for (int clientSocket : clientSockets) {
        if (clientSocket != senderSocket) {
            send(clientSocket, message.c_str(), message.size(), 0);
        }
    }
}

void* HandleClient(void* data) {
    // ��������� �������������� � �������� � ������
    ThreadData* threadData = static_cast<ThreadData*>(data);
    int clientSocket = threadData->clientSocket;

    char buffer[1024];
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            break;
        }

        buffer[bytesReceived] = '\0';
        std::cout << "Received from client: " << buffer << std::endl;
        std::string clientMessage = "Client said: ";
        clientMessage += buffer;
        BroadcastMessage(clientMessage, clientSocket);
    }

    close(clientSocket);

    for (auto it = clientSockets.begin(); it != clientSockets.end(); ++it) {
        if (*it == clientSocket) {
            clientSockets.erase(it);
            break;
        }
    }

    delete threadData;
    pthread_exit(NULL);
}

void* ServerInputThread(void*) {
    // ���� � ���������� � �������� ��������� ��������
    std::string input;
    while (true) {
        std::getline(std::cin, input);
        SendMessageToAllClients("Server: " + input);
    }
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0); // ����� ��� �������

    if (serverSocket == -1) {
        std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
        return 1;
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345); //��������� ������ � ����� �������
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        // �������� ������ � ������ � �����
        std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 5) == -1) {
        // ������ ������������� �����������
        std::cerr << "Error starting listening: " << strerror(errno) << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "Server is waiting for connections..." << std::endl;

    pthread_t inputThread;
    if (pthread_create(&inputThread, NULL, ServerInputThread, NULL) != 0) {
        std::cerr << "Error creating server input thread" << std::endl;
    }

    while (true) {
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if (clientSocket == -1) {
            std::cerr << "Error accepting connection: " << strerror(errno) << std::endl;
            continue;
        }

        clientSockets.push_back(clientSocket); // ���������� ������ ������� � ������

        std::cout << "Client connected: " << inet_ntoa(clientAddress.sin_addr) << std::endl;

        pthread_t thread;
        ThreadData* data = new ThreadData;
        data->clientSocket = clientSocket;
        if (pthread_create(&thread, NULL, HandleClient, data) != 0) {
            std::cerr << "Error creating thread" << std::endl;
            close(clientSocket);
            delete data;
        }
    }

    close(serverSocket); // �������� ������ �������

    return 0;
}
