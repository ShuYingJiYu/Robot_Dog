#ifndef UDP_UTIL_HPP
#define UDP_UTIL_HPP

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <QThread>

#define SERV_PORT 8000

class UdpThread : public QThread {
public:
    UdpThread() = default;

    int receive = 0;
    int color = 0;

    // 创建UDP套接字
    int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);

    unsigned char color_threshold[6]{};
private:
    void run() override {
        if (udp_socket == -1) {
            cerr << "udp_socket error" << endl;
            exit(1);
        }

        // 设置本地地址和端口
        sockaddr_in addr_serv{};
        addr_serv.sin_family = AF_INET;
        addr_serv.sin_port = htons(SERV_PORT); // 端口
        addr_serv.sin_addr.s_addr = htonl(INADDR_ANY); // IP地址

        // 绑定套接字到本地地址和端口
        if (bind(udp_socket, (sockaddr *) &addr_serv, sizeof(addr_serv)) < 0) {
            cerr << "bind error" << endl;
            close(udp_socket);
            exit(1);
        }

        // 接收数据
        ssize_t buffer_read;
        unsigned char buffer[20];
        while (true) {
            buffer_read = recvfrom(udp_socket, buffer, sizeof(buffer), 0, nullptr, nullptr);
            if (buffer_read == -1) {
                cerr << "receive error" << endl;
                close(udp_socket);
                exit(1);
            }

            int index = buffer[0];
            switch (index) {
                case 0:
                    receive = 1;
                    color = (int) buffer[1];
                    break;
                case 1:
                    receive = 2;
                    // copy color threshold
                    memcpy(color_threshold, buffer + 1, 6);
                    break;
                case 2:
                    receive = 3;
                    break;
                default:
                    break;
            }
//            cout << "receive: " << receive << endl;
//            for (int i = 0; i < buffer_read; i++) {
//                cout << (int) buffer[i] << " ";
//            }
        }

        close(udp_socket);
    };
};

#endif //UDP_UTIL_HPP