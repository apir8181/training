
#ifndef MM_TRAINING_SOCKET_H
#define MM_TRAINING_SOCKET_H

#include <string>
#define LOCALBUFLEN 256
#define BACKLOG 10

namespace mmtraining {

/**
 * 客户端 socket
 */
class ClientSocket {
public:
    /**
     * 初始化, this is used for server
     */
    ClientSocket();
    
    /**
     * 初始化, this is used for server
     */
    ClientSocket(int fd);

    /**
     * 析构函数
     */
    ~ClientSocket();
    
    /**
     * 连接服务器
     */
    int Connect(const char* ip, unsigned short port);
    
    /**
     * 发送数据
     * @return -1: 发送失败, 其他: 发送字节数
     */
    int Write(const void* buffer, int bufferSize);
    
    /**
     * 发送数据, 自动补全换行符
     * @return -1: 发送失败, 其他: 发送字节数
     */
    int WriteLine(const void *buffer, int bufferSize);
    
    /**
     * 接收数据
     * @return -1: 接收失败, 其他: 接收字节数
     */
    int Read(void* buffer, int bufferSize);
    
    /**
     * 接收一行数据
     * @return -1: 接收失败, 其他: 接收字节数
     */
    int ReadLine(void* buffer, int bufferSize);
    
    /**
     * 关闭连接
     */
    int Close();

    int GetFD();
    
private:

    int fd;

    char buf[LOCALBUFLEN];
    int bufoff, buflen;
    int BufReadChar(void *buffer);

};

/**
 * 服务端 socket
 */
class ServerSocket {
public:
    /**
     * 构造函数
     */
    ServerSocket();

    /**
     * 析构函数
     */
    ~ServerSocket();
    
    /**
     * 监听端口
     */
    int Listen(const char* ip, unsigned short port, bool nonBlocking);
    
    /**
     * accept 新连接
     */
    ClientSocket* Accept();

    /**
     * only return clientfd
     */
    int Accept1();

    int GetFD();
    
private:

    char ip[256];
    unsigned short port;
    int fd;
};

} // namespace mmtraining

#endif // MM_TRAINING_SOCKET_H
