
#include "socket.h"
#include "kvhelper.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

using std::string;
using std::cout;
using std::endl;

extern int errno;

namespace mmtraining {

////////////////////////////////////////////////ClientSocket

ClientSocket::ClientSocket() : fd(-1) {
    bufoff = -1;
    buflen = 0;
    memset(buf, 0, sizeof(buf));
}

    
ClientSocket::ClientSocket(int fd) {
    this->fd = fd;
    bufoff = -1;
    memset(buf, 0, sizeof(buf));
}

ClientSocket::~ClientSocket() {
    // TODO: 释放资源
    //Close();
}
    
int ClientSocket::Connect(const char* ip, unsigned short port) {
    // TODO: 完成代码
    if (fd != -1) {
	return 0;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    if ((fd = socket(addr.sin_family, SOCK_STREAM, 0)) == -1) {
	cout << "client socket init error: " << strerror(errno) << endl;
	return -1;
    } else if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
	cout << "client connect error: " << strerror(errno) << endl;
	return -1;
    }

    return 0;
}
    
int ClientSocket::Write(const void* buffer, int bufferSize) {
    // TODO: 完成代码
    if (fd == -1) {
	return -1;
    }

    int nremain = bufferSize, nwrite = 0;
    int ret = 0;
    char *ptr = (char *)buffer;
    while (nremain > 0) {

	if ((nwrite = write(fd, ptr, nremain)) == -1) {
	    cout << "client socket write error: " << strerror(errno) << endl;
	    return -1;
	}
	
	ptr += nwrite;
	nremain -= nwrite;
	ret += nwrite;
    }

    char s[] = "";
    if (write(fd, s, 1) < 0) {
	cout << "client socket write error: " << strerror(errno) << endl;
	return -1;
    }
    
    return ret;
}

int ClientSocket::WriteLine(const void *buffer, int bufferSize) {
    // TODO: 完成代码
    if (fd == -1) {
	return -1;
    }

    char *buffer_ptr = (char *)buffer;
    int temp, pos = bufferSize;

    for (int i = 0; i < bufferSize; ++ i)
	if (buffer_ptr[i] == '\n') pos = i;

    string s(buffer_ptr, pos);
    s += "\n\0";
    int remain = s.size() + 1;
    char *s_ptr = (char *)s.c_str();

    
    while (remain > 0) {
	if ((temp = write(fd, s_ptr, remain)) < 0) {
	    cout << "client socket write error: " << strerror(errno) << endl;
	    return -1;
	}

	remain -= temp;
	s_ptr += temp;
    }

    return bufferSize;
}

int ClientSocket::Read(void* buffer, int bufferSize) {
    // TODO: 完成代码
    if (fd == -1) {
	return -1;
    }

    char *ptr = (char *)buffer;
    int nremain = bufferSize, nread = 0, ret = 0;    

    while (nremain > 0) {
	if ((nread = read(fd, ptr, nremain)) == -1) {
	    cout << "client socket read error: " << strerror(errno) << endl;
	    return -1;
	}
	
	ptr += nread;
	ret += nread;
	nremain -= nread;
    }

    return ret;
}

int ClientSocket::ReadLine(void *buffer, int bufferSize) {
    // TODO: 完成代码
    if (fd == -1) {
	return -1;
    }

    char *ptr = (char *)buffer;
    int ret = 0, temp;

    while (1) {
	temp = BufReadChar(ptr);
	if (temp == -1) {
	    ret = -1;
	    break;
	} else if (temp == -2) {
	    ret = 0;
	    break;
	} else {
	    /*string s(ptr, temp);
	      cout << "read:" << s << endl;*/
	    ptr += temp;
	}
    }
    //cout << "readfinish:" << (char*)buffer << endl;

    //cout << ret << " read finish:" << (char *)buffer << endl;
    return ret;
}

int ClientSocket::Close() {
    // TODO: 完成代码
    if (fd != -1) {
	close(fd);
	fd = -1;
    }

    return 0;
}


//-2 for finish, -1 for error
int ClientSocket::BufReadChar(void *buffer) {
    if (fd == -1) {
	return -1;
    }

    if (bufoff == -1) {
	int nread = 0;
	char *ptr = buf;
	memset(buf, 0, sizeof(buf));
	
    again:
	if ((nread = read(fd, ptr, LOCALBUFLEN)) == -1) {
	    if (errno == EAGAIN) goto again;
	    cout << "buf read char error: " << strerror(errno) << endl;
	    return -1;
	}

	//string s(ptr, nread);
	//cout << "ClientSocket::BufReadChar read: " << s << endl;

	if (nread == 0) return -2;
	bufoff = 0;
	buflen = nread;
    }


    int ret = 0, offset = bufoff;
    char *ptr = (char *)buffer;
    for (int i = 0; i < buflen - offset; ++ i) {
	ptr[i] = buf[offset + i];
	++ ret;
	++ bufoff;
	if (ptr[i] == '\n' || ptr[i] == '\0' || ptr[i] == '\r') {
	    ptr[i] = '\0';
	    ret = -2;
	    bufoff = -1;
	    buflen = 0;
	    break;
	}
    }

    if (bufoff == buflen) {
	bufoff = -1;
	buflen = 0;
    }
    return ret;
}

int ClientSocket::GetFD() {
    return fd;
}

////////////////////////////////////////////////ServerSocket

ServerSocket::ServerSocket() : fd(-1) {
    // TODO: 完成代码
    memset(ip, 0, sizeof(ip));
    port = 0;

}

ServerSocket::~ServerSocket() {
    // TODO: 释放资源
    if (fd != -1) {
	close(fd);
	fd = -1;
    } 


}

int ServerSocket::Listen(const char* ip, unsigned short port, bool nonBlocking) {
    // TODO: 完成代码
    
    if (fd != -1) {
	return -1;
    }

    struct sockaddr_in seraddr;
    memset(&seraddr, 0, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_port = htons(port);
    seraddr.sin_addr.s_addr = inet_addr(ip);

    strcpy(this->ip, ip);
    this->port = port;
    
    if ((fd = socket(seraddr.sin_family, SOCK_STREAM, 0)) == -1) {
	cout << "server socket listen error :" << strerror(errno) << endl;
	return -1;
    } else if (nonBlocking && SetNonBlocking(fd) != 0) {
	cout << "server socket set nonblocking error :" << endl;
	return -1;
    } else if (bind(fd, (struct sockaddr *)&seraddr, sizeof(seraddr)) == 1) {
	cout << "server bind error :" << strerror(errno) << endl;
	return -1;
    } else if (listen(fd, BACKLOG) == -1) {
	cout << "server listen error :" << endl;
	return -1;
    }

    return 0;
}

int ServerSocket::GetFD() {
    return fd;
}

ClientSocket* ServerSocket::Accept() {
    // TODO: 完成代码
    
    int clientfd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    if ((clientfd = accept(fd, (struct sockaddr *)&addr, &addrlen)) == -1) {
	cout << "server accept error:" << strerror(errno) << endl;
	return NULL;
    }

    ClientSocket* client = new ClientSocket(clientfd);

    if (client != NULL && client->Connect(ip, port) == -1) {
	cout << "server client init error as above" << endl;
	delete client;
	return NULL;
    }

    return client;
}

int ServerSocket::Accept1() {
    int clientfd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    if ((clientfd = accept(fd, (struct sockaddr *)&addr, &addrlen)) == -1) {
	cout << "server accept error:" << strerror(errno) << endl;
	return -1;
    }

    return clientfd;
}

} // namespace mmtraining
