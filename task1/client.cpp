#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    //создание сокетв
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "ERROR  creating a socket: " << strerror(errno) << std::endl;
        return 1;
    }

    // задаем адрес сервера
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    // подключаение к серверу
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "ERROR connecting to the server: " << strerror(errno) << std::endl;
        close(clientSocket);
        return 1;
    }

    // чтение и отображение сообщений
    char buffer[1024];
    while (true) {
        std::string message;
        std::cout << "Enter a message: ";
        std::getline(std::cin, message);

        if (message == "exit") {
            break;
        }

        // отправка сообщения на сервер
        send(clientSocket, message.c_str(), message.size(), 0);

        // получение ответа от сервера
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived == -1) {
            std::cerr << "ERROR receiving data: " << strerror(errno) << std::endl;
            close(clientSocket);
            return 1;
        }

        // вывод ответа
        buffer[bytesReceived] = '\0';
        std::cout << "Server: " << buffer << std::endl;
    }

    close(clientSocket);

    return 0;
}
