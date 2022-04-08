#pragma once
#include <include/cef_client.h>
#include <include/cef_life_span_handler.h>
#include <include/cef_load_handler.h>
#include <include/cef_render_handler.h>

#include <list>

namespace Osr {
class Client : public CefClient,
               public CefRenderHandler,
               public CefLoadHandler,
               public CefLifeSpanHandler {
 public:
  Client(int prefered_width, int prefered_height);
  ~Client();

  static Client* GetInstance();

  // Inherited via CefRenderHandler
  virtual void GetViewRect(CefRefPtr<CefBrowser> browser,
                           CefRect& rect) override;
  virtual void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
                       const RectList& dirtyRects, const void* buffer,
                       int width, int height) override;

  virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override {
    return this;
  }
  virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override {
    return this;
  }
  virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }

  // CefLifeSpanHandler methods:
  virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
  virtual bool DoClose(CefRefPtr<CefBrowser> browser) override;
  virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

  // CefLoadHandler methods:
  virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame, ErrorCode errorCode,
                           const CefString& errorText,
                           const CefString& failedUrl) override;

 private:
  // Default ref-count impl
  IMPLEMENT_REFCOUNTING(Client);
  int width_, height_;
  std::list<CefRefPtr<CefBrowser>> browser_list_;
};
}  // namespace Osr
