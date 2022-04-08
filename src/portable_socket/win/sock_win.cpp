#pragma once

#include "../socket.h"

#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
// Windows support for AF_UNIX
#include <afunix.h>

Socket::Socket(int domain, int type, int protocol) {
  socket_ = socket(domain, type, protocol);
}
Socket::Socket(raw_socket_t skt) : socket_(skt) {}
Socket::~Socket() {}

int Socket::Bind(const sock_addr_t* addr, socklen_t addr_len) {
  return bind(socket_, addr, addr_len);
}

int Socket::Connect(const sock_addr_t* addr, socklen_t addr_len) {
  return connect(socket_, addr, addr_len);
}

int Socket::Listen(socklen_t backlog) {
  return listen(socket_, backlog);
}

Socket Socket::Accept(sock_addr_t* addr, socklen_t* addr_len) {
  raw_socket_t skt = accept(socket_, addr, addr_len);
  return Socket(skt);
}

int Socket::Send(const char* buf, int len, int flags) {
  return send(socket_, buf, len, flags);
}

int Socket::Recv(char* buf, int len, int flags) {
  return recv(socket_, buf, len, flags);
}
