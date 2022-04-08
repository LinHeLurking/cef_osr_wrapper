#include "sender.h"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>

using std::lock_guard;
using std::mutex;
using std::string;
using std::thread;

namespace FrameBridge {
namespace {
Sender* g_Instance = nullptr;
mutex g_mutex;

const string osr_socket_name = "osr.socket";
const int osr_socket_name_len = (int)osr_socket_name.size();

thread* bg_socket_handler_thread;

constexpr int recv_buf_len = 10000;
char recv_buf[recv_buf_len];

inline int PlatformInit() {
#if defined(OS_WIN)
  WSADATA wsaData;
  int iResult;

  // Initialize Winsock
  iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != 0) {
    // printf("WSAStartup failed: %d\n", iResult);
    return 1;
  }
#endif
  return 0;
}
}  // namespace

void Sender::BackgoundSocketHandler() {
  using Status = Sender::Status;
  auto task = [this]() {
    while (true) {
      auto status = this->GetStatus();
      switch (status) {
        case Status::Uninitialized:
        case Status::Initializing:
          std::this_thread::sleep_for(std::chrono::milliseconds(200));
          break;         
        case Status::Idle:
          this->client_socket_ = this->main_socket_.Accept(nullptr, nullptr);
          this->SetStatus(Status::Communicating);
        case Status::Communicating:
          while (true) {
            // Currently no Java -> C++ socket data.
            int len = this->client_socket_.Recv(recv_buf, recv_buf_len, 0);
            if (len == 0) {
              // Client closes.
              this->SetStatus(Status::Idle);
              break;
            }
          }
          break;
      }
    }
  };
  bg_socket_handler_thread = new thread(task);
}

Sender* Sender::GetInstance() {
  // Initialize global singleton instance
  lock_guard<mutex> lock(g_mutex);
  if (g_Instance == nullptr) {
    PlatformInit();
    g_Instance = new Sender();

    // Initialize socket
    g_Instance->SetStatus(Status::Initializing);
    sock_addr_t sock_addr;
    sock_addr.sa_family = AF_UNIX;
    strncpy(sock_addr.sa_data, osr_socket_name.c_str(), osr_socket_name_len);
    g_Instance->main_socket_.Bind(&sock_addr, osr_socket_name_len);
    g_Instance->main_socket_.Listen(5);
    g_Instance->SetStatus(Status::Idle);

    g_Instance->BackgoundSocketHandler();
  }
  return g_Instance;
}

// Client Socket
Sender::Sender()
    : main_socket_(Socket(AF_UNIX, SOCK_STREAM, 0)),
      client_socket_(Socket(AF_UNIX, SOCK_STREAM, 0)) {}
Sender::~Sender() {}

Sender::Status Sender::GetStatus() {
  const lock_guard<mutex> lock(mutex_);
  return status_;
}

void Sender::SetStatus(Sender::Status status) {
  const lock_guard<mutex> lock(mutex_);
  status_ = status;
}

int Sender::SendFrame(CefRenderHandler::PaintElementType type,
                      const CefRenderHandler::RectList& dirtyRects,
                      const void* buffer, int width, int height) {
  int buf_len = width * height * 4;
  auto status = GetStatus();
  switch (status) {
    case Status::Uninitialized:
    case Status::Initializing:
    case Status::Idle:
      // Connection not established
      return -1;
    case Status::Communicating:
      // Only after a connection was established, the client_socket_ is valid;
      client_socket_.Send((const char*)buffer, buf_len, 0);
      break;
  }
  return 0;
}
}  // namespace FrameBridge
