#include "client.h"

#include <cef_app.h>
#include <cef_parser.h>
#include <include/base/cef_callback.h>
#include <include/wrapper/cef_closure_task.h>
#include <include/wrapper/cef_helpers.h>

#include <cstdint>
#include <fstream>
#include <iostream>

#include "../frame_bridge/sender.h"

using std::cout;
using std::endl;
using std::string;
using std::wcout;
using std::wstring;

namespace {
// Client instance
Osr::Client* g_instance = nullptr;
void check_instance_null() { DCHECK(g_instance == nullptr); }
void check_instance_notnull() { DCHECK(g_instance != nullptr); }

// HTML string print visitor
class HtmlPrintVisitor : public CefStringVisitor {
 public:
  HtmlPrintVisitor() {}

  // Called asynchronously when the HTML contents are available.
  virtual void Visit(const CefString& string) override {
    cef::logging::LogAtLevel(cef::logging::LOG_INFO, string.ToString());
  }

  IMPLEMENT_REFCOUNTING(HtmlPrintVisitor);
};

// Returns a data: URI with the specified contents.
std::string GetDataURI(const std::string& data, const std::string& mime_type) {
  return "data:" + mime_type + ";base64," +
         CefURIEncode(CefBase64Encode(data.data(), data.size()), false)
             .ToString();
}
}  // namespace

namespace Osr {

Client::Client(int prefered_width, int prefered_height)
    : width_(prefered_width), height_(prefered_height) {
  check_instance_null();
  g_instance = this;
}

Client::~Client() {
  check_instance_notnull();
  g_instance = nullptr;
}

Client* Client::GetInstance() { return g_instance; }

void Client::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
  rect.x = rect.y = 0;
  rect.width = width_;
  rect.height = height_;
}

void Client::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
                     const RectList& dirtyRects, const void* buffer, int width,
                     int height) {
  auto sender = FrameBridge::Sender::GetInstance();
  if (sender == nullptr) {
    return;
  }
  // Frames during sender initialization stage will be dropped,
  // without being sent out.
  sender->SendFrame(type, dirtyRects, buffer, width, height);
}

void Client::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  browser_list_.push_back(browser);
}

bool Client::DoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  return false;
}

void Client::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  for (auto it = browser_list_.begin(); it != browser_list_.end(); ++it) {
    if ((*it)->IsSame(browser)) {
      browser_list_.erase(it);
      break;
    }
  }
  if (browser_list_.empty()) {
    CefQuitMessageLoop();
  }
}

void Client::OnLoadError(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame, ErrorCode errorCode,
                         const CefString& errorText,
                         const CefString& failedUrl) {
  CEF_REQUIRE_UI_THREAD();
  // Don't display an error for downloaded files.
  if (errorCode == ERR_ABORTED) return;

  // Display a load error message using a data: URI.
  std::stringstream ss;
  ss << "<html><body bgcolor=\"white\">"
        "<h2>Failed to load URL "
     << std::string(failedUrl) << " with error " << std::string(errorText)
     << " (" << errorCode << ").</h2></body></html>";

  frame->LoadURL(GetDataURI(ss.str(), "text/html"));
}
}  // namespace Osr