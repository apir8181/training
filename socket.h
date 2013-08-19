
#ifndef MM_TRAINING_SOCKET_H
#define MM_TRAINING_SOCKET_H

#include <string>
#define LOCALBUFLEN 256
#define BACKLOG 10

namespace mmtraining {

/**
 * �ͻ��� socket
 */
class ClientSocket {
public:
    /**
     * ��ʼ��, this is used for server
     */
    ClientSocket();
    
    /**
     * ��ʼ��, this is used for server
     */
    ClientSocket(int fd);

    /**
     * ��������
     */
    ~ClientSocket();
    
    /**
     * ���ӷ�����
     */
    int Connect(const char* ip, unsigned short port);
    
    /**
     * ��������
     * @return -1: ����ʧ��, ����: �����ֽ���
     */
    int Write(const void* buffer, int bufferSize);
    
    /**
     * ��������, �Զ���ȫ���з�
     * @return -1: ����ʧ��, ����: �����ֽ���
     */
    int WriteLine(const void *buffer, int bufferSize);
    
    /**
     * ��������
     * @return -1: ����ʧ��, ����: �����ֽ���
     */
    int Read(void* buffer, int bufferSize);
    
    /**
     * ����һ������
     * @return -1: ����ʧ��, ����: �����ֽ���
     */
    int ReadLine(void* buffer, int bufferSize);
    
    /**
     * �ر�����
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
 * ����� socket
 */
class ServerSocket {
public:
    /**
     * ���캯��
     */
    ServerSocket();

    /**
     * ��������
     */
    ~ServerSocket();
    
    /**
     * �����˿�
     */
    int Listen(const char* ip, unsigned short port, bool nonBlocking);
    
    /**
     * accept ������
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
