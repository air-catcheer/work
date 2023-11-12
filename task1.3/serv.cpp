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
    int clientSocket; // структура для передачи данных потоку обработки клиента
};

std::vector<int> clientSockets; // вектор для хранения сокетов клиентов

void SendMessageToAllClients(const std::string& message) {
    // отправка сообщений всем клиентам
    for (int clientSocket : clientSockets) {
        send(clientSocket, message.c_str(), message.size(), 0);
    }
}

void BroadcastMessage(const std::string& message, int senderSocket) {
    // отправка сообщений всем клиентам кроме отправителя
    for (int clientSocket : clientSockets) {
        if (clientSocket != senderSocket) {
            send(clientSocket, message.c_str(), message.size(), 0);
        }
    }
}

void* HandleClient(void* data) {
    // обработка взаимодействий с клиентом в потоке
    ThreadData* threadData = static_cast<ThreadData*>(data);
    int clientSocket = threadData->clientSocket;

    char buffer[1024];
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            break;
        }

        buffer[bytesReceived] = '\0';

        int clientNumber = -1;
        for (size_t i = 0; i < clientSockets.size(); ++i) {
            if (clientSockets[i] == clientSocket) {
                clientNumber = static_cast<int>(i);
                break;
            }
        }

        if (clientNumber != -1) {
            std::string clientMessage = "Client " + std::to_string(clientNumber) + " said: ";

            // отпарвяем оригинальное сообщение:
            clientMessage += buffer;

            std::cout << clientMessage << std::endl;

            BroadcastMessage(clientMessage, clientSocket);
        }
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
    // ввод с клавиатуры и отправка сообщений клиентам
    std::string input;
    while (true) {
        std::getline(std::cin, input);

        // Если это первый клиент, то отправляем оригинальное сообщение
        if (!clientSockets.empty()) {
            int firstClientSocket = clientSockets[0];
            send(firstClientSocket, ("Server: " + input).c_str(), input.size() + 8, 0);
        }

        // Если это не первый клиент, то отправляем сообщение с увеличением символов на +1
        if (clientSockets.size() > 1) {
            for (size_t i = 1; i < clientSockets.size(); ++i) {
                int clientSocket = clientSockets[i];
                std::string modifiedInput = input;  // Создаем копию строки перед изменением
                for (char &ch : modifiedInput) {
                    ch = (ch - 'a' + 1) % 26 + 'a';
                }
                send(clientSocket, ("Server: " + modifiedInput).c_str(), modifiedInput.size() + 8, 0);
            }
        }
    }
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0); //сокет для сервера

    if (serverSocket == -1) {
        std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
        return 1;
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345); //настройка адреса и порта сервера
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        // привязка сокета к адресу и порту
        std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 5) == -1) {
        // Начало прослушивания подключений
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

        clientSockets.push_back(clientSocket); // добавление сокета клиента в вектор

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

    close(serverSocket); // закрытие сокета сервера

    return 0;
}
