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
using namespace cef::logging;

namespace FrameBridge {
namespace {
Sender* g_Instance = nullptr;
mutex g_mutex;

thread* bg_socket_handler_thread = nullptr;

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
    LogAtLevel(LOG_INFO, "Sender background handler started.");
    while (true) {
      auto status = this->GetStatus();
      switch (status) {
        case Status::Uninitialized:
        case Status::Initializing:
          LogAtLevel(LOG_INFO,
                     "Sender initialization not finished. Waiting for 500ms.");
          std::this_thread::sleep_for(std::chrono::milliseconds(500));
          break;
        case Status::Idle:
          LogAtLevel(LOG_INFO, "Waiting for incoming connections...");
          // Accept a client socket
          this->client_socket_ = this->listen_socket_.Accept(nullptr, nullptr);
          if (this->client_socket_.IsValid()) {
            this->SetStatus(Status::Communicating);
          } else {
            LogAtLevel(LOG_ERROR, "Accept failed.");
            Socket::Clenup();
            return;
          }
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
  const lock_guard<mutex> lock(g_mutex);
  if (g_Instance == nullptr) {
    LogAtLevel(LOG_INFO, "First time initializing frame bridge sender...");
    PlatformInit();
    g_Instance = new Sender();
    LogAtLevel(LOG_INFO, "Starting sender background handler...");
    g_Instance->BackgoundSocketHandler();
    LogAtLevel(LOG_INFO, "Initializing frame bridge sender socket...");

    // Initialize socket
    g_Instance->SetStatus(Status::Initializing);

    int i_result;
    struct addrinfo* result = NULL;
    struct addrinfo hints;

    // Resolve the server address and port
    LogAtLevel(LOG_INFO, "Resolving server addr...");
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    i_result = getaddrinfo(nullptr, "27015", &hints, &result);
    if (i_result != 0) {
      LogAtLevel(LOG_ERROR,
                 "getaddrinfo failed with error: " + std::to_string(i_result));
      Socket::Clenup();
      return g_Instance;
    }

    // Create socket or connecting to server
    LogAtLevel(LOG_INFO, "Creating socket...");
    g_Instance->listen_socket_ =
        Socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (!(g_Instance->listen_socket_.IsValid())) {
      LogAtLevel(LOG_ERROR, "Socket creation failed.");
      freeaddrinfo(result);
      Socket::Clenup();
      return g_Instance;
    }

    // Setup the TCP listening socket
    LogAtLevel(LOG_INFO, "Binding socket...");
    i_result = g_Instance->listen_socket_.Bind(result->ai_addr,
                                               (int)result->ai_addrlen);
    if (i_result == SOCKET_ERROR) {
      LogAtLevel(LOG_ERROR, "Bind failed.");
      freeaddrinfo(result);
      Socket::Clenup();
      return g_Instance;
    }
    freeaddrinfo(result);

    // Set listening
    LogAtLevel(LOG_INFO, "Setting listenning...");
    i_result = g_Instance->listen_socket_.Listen(SOMAXCONN);
    if (i_result == SOCKET_ERROR) {
      LogAtLevel(LOG_ERROR, "Listen error.");
      g_Instance->listen_socket_.Close();
      Socket::Clenup();
      return g_Instance;
    }

    LogAtLevel(LOG_INFO,
               "Socket initialization finished. Now sender is in idle mode.");
    g_Instance->SetStatus(Status::Idle);
  }
  return g_Instance;
}

// Client Socket
Sender::Sender() {}
Sender::~Sender() {
  if (listen_socket_.IsValid()) {
    listen_socket_.Close();
  }
  if (client_socket_.IsValid()) {
    client_socket_.Clenup();
  }
  Socket::Clenup();
}

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
