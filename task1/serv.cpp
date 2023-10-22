#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    // создание сокета
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
        return 1;
    }

    // задаём адрес сервера
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345);
    serverAddress.sin_addr.s_addr = INADDR_ANY; //слушаем на всех доступных сетевых интерфейсах

    // привязываем сокет к адресу
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
        close(serverSocket);
        return 1;
    }

    // начало прослушивания на порту
    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Error starting listening: " << strerror(errno) << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "Server is waiting for connections..." << std::endl;

    // приём входящего соединения
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
    if (clientSocket == -1) {
        std::cerr << "Error accepting connection: " << strerror(errno) << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "Client connected: " << inet_ntoa(clientAddress.sin_addr) << std::endl;

    // чтение и обработка сообщений от клиента
    char buffer[1024];
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            break;
        }

        buffer[bytesReceived] = '\0';
        std::cout << "Received from the client: " << buffer << std::endl;

        //отправить автоматический ответ клиенту
       // const char* response = "Hello, Client!";
        //send(clientSocket, response, strlen(response), 0);

        // написать ответ клиенту
        std::string serverMessage;
        std::cout << "Enter a message for the client: ";
        std::getline(std::cin, serverMessage);
        send(clientSocket, serverMessage.c_str(), serverMessage.size(), 0);
    }

    close(clientSocket);

    close(serverSocket);

    return 0;
}

