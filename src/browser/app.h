#pragma once

#include <Windows.h>

#include <string>

#include "include/cef_app.h"

namespace Osr {
class App : public CefApp, public CefBrowserProcessHandler {
 public:
  App(int prefered_width, int prefered_height, std::string url,
      bool osr)
      : width_(prefered_width),
        height_(prefered_height),
        url_(std::move(url)),
        osr_(osr) {}

  // CefApp methods:
  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
    return this;
  }

  // CefBrowserProcessHandler methods:
  void OnContextInitialized() override;
  CefRefPtr<CefClient> GetDefaultClient() override;

 private:
  IMPLEMENT_REFCOUNTING(App);
  int width_, height_;
  std::string url_;
  bool osr_;
};
}  // namespace Osr