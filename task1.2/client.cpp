#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
        return 1;
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP-адрес сервера

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error connecting to server: " << strerror(errno) << std::endl;
        close(clientSocket);
        return 1;
    }

    std::cout << "Connected to the server." << std::endl;

    char message[1024];
    while (true) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(0, &read_fds);  // Добавляем стандартный ввод в множество

        FD_SET(clientSocket, &read_fds);  // Добавляем сокет в множество

        int max_fd = (clientSocket > 0) ? clientSocket : 0;

        // Выбираем активное событие
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            std::cerr << "Error in select" << std::endl;
            break;
        }

        if (FD_ISSET(0, &read_fds)) {
            // Есть данные для отправки на сервер
            std::string input;
            std::getline(std::cin, input);
            send(clientSocket, input.c_str(), input.size(), 0);
        }

        if (FD_ISSET(clientSocket, &read_fds)) {
            // Есть данные от сервера
            int bytesReceived = recv(clientSocket, message, sizeof(message), 0);
            if (bytesReceived <= 0) {
                std::cerr << "Connection closed by the server" << std::endl;
                break;
            }
            message[bytesReceived] = '\0';
            std::cout << "Received from the server: " << message << std::endl;
        }
    }

    close(clientSocket);

    return 0;
}
