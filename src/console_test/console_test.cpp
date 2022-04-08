#include <iostream>

#include "../portable_socket/socket.h"

int main() {
  std::cout << "Hello World!" << std::endl;
#if defined(OS_WIN)
  std::cout << "Windows!" << std::endl;
#endif
  return 0;
}