#include <QThread>
#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <iostream>

#define SERV_PORT 8000

class UdpUtil : public QThread {
public:
    UdpUtil() = default;

    int ifReceiveInfoFlag = 0;
    int color = 0;
    QByteArray colorThreshold;

private:
    void run() override;
};

// udp接收线程，用于接收从上位机发来的信息
void UdpUtil::run() {
    /* sock_fd --- socket文件描述符 创建udp套接字*/
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("socket");
        exit(1);
    }
    /* 将套接字和IP、端口绑定 */
    struct sockaddr_in addr_serv
            {
            };
    int len;
    memset(&addr_serv, 0, sizeof(struct sockaddr_in)); // 每个字节都用0填充
    addr_serv.sin_family = AF_INET;                    // 使用IPV4地址
    addr_serv.sin_port = htons(SERV_PORT);             // 端口


    /* INADDR_ANY表示不管是哪个网卡接收到数据，只要目的端口是SERV_PORT，就会被该应用程序接收到 */
    addr_serv.sin_addr.s_addr = htonl(INADDR_ANY); // 自动获取IP地址
    len = sizeof(addr_serv);

    /* 绑定socket */
    if (bind(sock_fd, (struct sockaddr *) &addr_serv, sizeof(addr_serv)) < 0) {
        perror("bind error:");
        exit(1);
    }
    long recv_num;
    char recv_buf[20];
    struct sockaddr_in addr_client
            {
            };
    while (true) {
        recv_num = recvfrom(sock_fd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *) &addr_client,
                            (socklen_t *) &len);
        if (recv_num < 0) {
            perror("recvfrom error:");
            break;
        }

        // printf("server wait:\n");
        // recv_buf[recv_num] = '\0';
        int index = (unsigned char) recv_buf[0];

        // std::cout<<index<<std::endl;
        switch (index) {
            case 0:
                ifReceiveInfoFlag = 1; // choose color and return this color threshold
                color = (int) (unsigned char) recv_buf[1];
                break;
            case 1:
                ifReceiveInfoFlag = 2; // set color threshold
                for (int i = 0; i <= 5; i++) {
                    colorThreshold[i] = recv_buf[i + 1];
                }
                break;
            case 2:
                ifReceiveInfoFlag = 3; // save color threshold
                break;
            default:
                break;
        }
        //        for (int i = 0; i < 2; i++)
        //            printf("receive %d\n", recv_buf[0]);
        //        printf("server receive %d bytes: %s\n", recv_num, recv_buf);
        //
        //        send_num = sendto(sock_fd, send_buf, recv_num, 0, (struct sockaddr *) &addr_client, len);
        //        if (send_num < 0) {
        //            perror("sendto error:");
        //            exit(1);
        //        }

    }
    close(sock_fd);
    exit(1);
}
