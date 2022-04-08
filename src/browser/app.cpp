#include "app.h"

#include <include/cef_callback.h>
#include <include/cef_task.h>
#include <wrapper/cef_helpers.h>

#include <functional>

#include "client.h"

namespace {
class CreateBrowserTask : public CefTask {
 public:
  CreateBrowserTask(std::function<void()> f) : f_(std::move(f)) {}

  virtual void Execute() override { f_(); }

 private:
  std::function<void()> f_;
  IMPLEMENT_REFCOUNTING(CreateBrowserTask);
};
}  // namespace

namespace Osr {
void App::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

  CefRefPtr<Client> client(new Client(width_, height_));
  CefWindowInfo window_info;
  CefBrowserSettings browser_settings;

  if (osr_) {
    window_info.SetAsWindowless(nullptr);
  }

#if defined(OS_WIN)
  window_info.SetAsPopup(nullptr, "OSR Browser");
#endif

  CefBrowserHost::CreateBrowser(window_info, client, url_, browser_settings,
                                nullptr, nullptr);
}

CefRefPtr<CefClient> App::GetDefaultClient() { return Client::GetInstance(); }
}  // namespace Osr
