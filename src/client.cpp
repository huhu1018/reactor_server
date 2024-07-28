#include <iostream>
#include <sys/socket.h>
#include <bits/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>


int main(int argc, char* argv[]){

    if(argc != 3) {
        std::cout << "Usage: " << argv[0] << " <ip> <port>" << std::endl;
        return 1;
    }

    int sockfd;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cout << "Error: socket()" << std::endl;
        return 1;
    }

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);    
    serv_addr.sin_port = htons(atoi(argv[2]));

    if(connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "Error: connect()" << std::endl;
        return 1;
    }

    std::cout << "Connected to server" << std::endl;

    while(true) {
        char buffer[1024];
        std::cout << "Client input: ";
        std::cin.getline(buffer, 1024);

        // 发送数据
        if(send(sockfd, buffer, strlen(buffer), 0) < 0) {
            std::cout << "Error: send()" << std::endl;
            return 1;
        }

        // 接收数据
        bzero(buffer, 1024);
        if(recv(sockfd, buffer, 1024, 0) < 0) {
            std::cout << "Error: recv()" << std::endl;
            return 1;
        }

        // 打印数据
        std::cout << "Server: " << buffer << std::endl;

        // 输入exit则退出
        if(strcmp(buffer, "exit") == 0) {
            break;
        }
   }

   return 0;

}