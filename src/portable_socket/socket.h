#pragma once

#if defined(OS_WIN)
#include <WinSock2.h>
#include <Windows.h>
#endif

#if defined(OS_WIN)
typedef SOCKET raw_socket_t;
typedef struct sockaddr sock_addr_t;
typedef int socklen_t;
#elif defined(OS_LINUX)
typedef struct sockaddr sock_addr_t;
typedef int socket_t;  // File descriptor
#endif

class Socket {
 public:
  Socket(int domain, int type, int protocol);
  ~Socket();

  int Bind(const sock_addr_t *addr, socklen_t addr_len);
  int Connect(const sock_addr_t *addr, socklen_t addr_len);
  int Listen(socklen_t backlog);
  Socket Accept(sock_addr_t *addr, socklen_t *addr_len);
  int Send(const char *buf, int len, int flags);
  int Recv(char *buf, int len, int flags);

 private:
  raw_socket_t socket_;
  Socket(raw_socket_t skt);
};
