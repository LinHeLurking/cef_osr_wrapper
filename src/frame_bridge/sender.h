#pragma once

#include <include/cef_app.h>
#include <include/cef_browser.h>
#include <include/cef_client.h>
#include <include/cef_render_handler.h>

#include <mutex>
#include <thread>

#include "../portable_socket/socket.h"

namespace FrameBridge {
class Sender {
 public:
  enum class Status { Uninitialized, Initializing, Idle, Communicating };

  // Thread-safe helper to initialize and create an instance.
  static Sender* GetInstance();

  int SendFrame(CefRenderHandler::PaintElementType type,
                const CefRenderHandler::RectList& dirtyRects,
                const void* buffer, int width, int height);

  Status GetStatus();

  void SetStatus(Status status);

  void BackgoundSocketHandler();

 private:
  Sender();
  ~Sender();
  Socket main_socket_;
  Socket client_socket_;
  Status status_ = Status::Uninitialized;
  std::mutex mutex_;
};
}  // namespace FrameBridge
