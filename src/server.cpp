#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <strings.h>
#include <string.h>
#include <cerrno>


int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <ip> <port>\n";
        return 1;
    }

    // 创建监听端口
    int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (listenfd < 0) {
        perror("socket");
        return 1;
    }

    // 设置属性
    int on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    setsockopt(listenfd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
    setsockopt(listenfd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on));

    // 绑定端口
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        perror("inet_pton");
        close(listenfd);
        return 1;
    }
    servaddr.sin_port = htons(atoi(argv[2]));

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind");
        close(listenfd);
        return 1;
    }

    // 监听端口
    if (listen(listenfd, 128) < 0) {
        perror("listen");
        close(listenfd);
        return 1;
    }

    int epollfd = epoll_create1(0);
    if (epollfd < 0) {
        perror("epoll_create1");
        close(listenfd);
        return 1;
    }

    epoll_event ev, events[20];
    ev.data.fd = listenfd;
    ev.events = EPOLLIN;

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) < 0) {
        perror("epoll_ctl ADD listenfd");
        close(listenfd);
        close(epollfd);
        return 1;
    }

    while (1) {
        int nfds = epoll_wait(epollfd, events, 20, -1);
        if (nfds < 0) {
            perror("epoll_wait");
            close(listenfd);
            close(epollfd);
            return 1;
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].events & (EPOLLIN | EPOLLPRI)) {
                if (events[i].data.fd == listenfd) {
                    sockaddr_in cliaddr;
                    socklen_t clilen = sizeof(cliaddr);
                    int connfd = accept4(listenfd, (struct sockaddr *)&cliaddr, &clilen, SOCK_NONBLOCK);
                    if (connfd < 0) {
                        perror("accept");
                        continue;
                    }

                    printf("accept new connection from %s:%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

                    ev.data.fd = connfd;
                    ev.events = EPOLLIN | EPOLLET;
                    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev) < 0) {
                        perror("epoll_ctl ADD connfd");
                        close(connfd);
                    }
                } else {
                char buf[1024];
                    while (1) {
                        bzero(buf, sizeof(buf));
                        int n = read(events[i].data.fd, buf, sizeof(buf));
                        if (n < 0) {
                            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                                break;
                            } else {
                                perror("read");
                                close(events[i].data.fd);
                                break;
                            }
                        } else if (n == 0) {
                            printf("client closed\n");
                            close(events[i].data.fd);
                            break;
                        } else {
                            if (send(events[i].data.fd, buf, n, 0) < 0) {
                                perror("send");
                                close(events[i].data.fd);
                                break;
                            }
                        }
                    }
                }               
            } else if (events[i].events & EPOLLHUP) {
                printf("client closed\n");
                close(events[i].data.fd);
            } else {
                close(events[i].data.fd);
            }            
        }
    }

    close(listenfd);
    close(epollfd);
    return 0;
}