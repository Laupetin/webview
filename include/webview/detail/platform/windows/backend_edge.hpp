/*
 * MIT License
 *
 * Copyright (c) 2017 Serge Zaitsev
 * Copyright (c) 2022 Steffen André Langnes
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#ifndef WEBVIEW_DETAIL_PLATFORM_WINDOWS_BACKEND_EDGE_HPP
#define WEBVIEW_DETAIL_PLATFORM_WINDOWS_BACKEND_EDGE_HPP

#include "../../macros.hpp"

#if defined(WEBVIEW_PLATFORM_WINDOWS) && defined(WEBVIEW_EDGE)

//
// ====================================================================
//
// This implementation uses Win32 API to create a native window. It
// uses Edge/Chromium webview2 backend as a browser engine.
//
// ====================================================================
//

#include "../../errors.hpp"
#include "../../native_library.hpp"
#include "../../platform/windows/com_init_wrapper.hpp"
#include "../../platform/windows/webview2/loader.hpp"
#include "../../types.hpp"
#include "../../window_base.hpp"

#include <atomic>
#include <functional>
#include <memory>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <objbase.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <windows.h>

namespace webview
{
  namespace detail
  {
    using msg_cb_t = std::function<void(std::string)>;

    class webview2_com_handler : public ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler,
                                 public ICoreWebView2CreateCoreWebView2ControllerCompletedHandler,
                                 public ICoreWebView2WebMessageReceivedEventHandler,
                                 public ICoreWebView2PermissionRequestedEventHandler
    {
      using webview2_com_handler_cb_t = std::function<void(ICoreWebView2Controller*, ICoreWebView2* webview)>;

  public:
      webview2_com_handler(HWND hwnd, msg_cb_t msgCb, webview2_com_handler_cb_t cb);

      virtual ~webview2_com_handler() = default;
      webview2_com_handler(const webview2_com_handler& other) = delete;
      webview2_com_handler& operator=(const webview2_com_handler& other) = delete;
      webview2_com_handler(webview2_com_handler&& other) = delete;
      webview2_com_handler& operator=(webview2_com_handler&& other) = delete;

      ULONG STDMETHODCALLTYPE AddRef() override;

      ULONG STDMETHODCALLTYPE Release() override;

      HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppv) override;

      HRESULT STDMETHODCALLTYPE Invoke(HRESULT res, ICoreWebView2Environment* env) override;

      HRESULT STDMETHODCALLTYPE Invoke(HRESULT res, ICoreWebView2Controller* controller) override;

      HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) override;

      HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2* sender, ICoreWebView2PermissionRequestedEventArgs* args) override;

      // Set the function that will perform the initiating logic for creating
      // the WebView2 environment.
      void set_attempt_handler(std::function<HRESULT()> attempt_handler) noexcept;

      // Retry creating a WebView2 environment.
      // The initiating logic for creating the environment is defined by the
      // caller of set_attempt_handler().
      void try_create_environment() noexcept;

  private:
      HWND m_window;
      msg_cb_t m_msg_cb;
      webview2_com_handler_cb_t m_cb;
      std::atomic<ULONG> m_ref_count{1};
      std::function<HRESULT()> m_attempt_handler;
      unsigned int m_max_attempts = 60;
      unsigned int m_sleep_ms = 200;
      unsigned int m_attempts = 0;
    };

    class webview2_user_script_added_handler : public ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler
    {
  public:
      using callback_fn = std::function<void(HRESULT errorCode, LPCWSTR id)>;

      explicit webview2_user_script_added_handler(callback_fn cb);

      virtual ~webview2_user_script_added_handler() = default;
      webview2_user_script_added_handler(const webview2_user_script_added_handler& other) = delete;
      webview2_user_script_added_handler& operator=(const webview2_user_script_added_handler& other) = delete;
      webview2_user_script_added_handler(webview2_user_script_added_handler&& other) = delete;
      webview2_user_script_added_handler& operator=(webview2_user_script_added_handler&& other) = delete;

      ULONG STDMETHODCALLTYPE AddRef() override;

      ULONG STDMETHODCALLTYPE Release() override;

      HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppv) override;

      HRESULT STDMETHODCALLTYPE Invoke(const HRESULT res, const LPCWSTR id) override;

  private:
      callback_fn m_cb;
      std::atomic<ULONG> m_ref_count{1};
    };

    class win32_edge_engine : public window_base
    {
  public:
      win32_edge_engine();
      ~win32_edge_engine() override;

      win32_edge_engine(const win32_edge_engine& other) = delete;
      win32_edge_engine& operator=(const win32_edge_engine& other) = delete;
      win32_edge_engine(win32_edge_engine&& other) = delete;
      win32_edge_engine& operator=(win32_edge_engine&& other) = delete;

      noresult eval(const std::string& js) override;

  protected:
      void dispatch_impl(dispatch_fn_t f) override;

      void set_window_size_impl(unsigned width, unsigned height) override;
      void set_window_min_impl(unsigned width, unsigned height) override;
      void set_window_max_impl(unsigned width, unsigned height) override;
      void set_window_size_fixed_impl(bool value) override;

      noresult set_html_impl(const std::string& html) override;

      noresult navigate_impl(const std::string& url) override;

      void set_title_impl(const std::string& title) override;

      noresult add_page_init_script(const std::string& js) override;

      noresult on_window_opened_impl() override;

      void run_event_loop_while(const std::function<bool()>& fn) override;

      window* downcast_this() override;

  private:
      noresult window_init();
      noresult window_settings();
      void window_show();

      noresult embed(HWND wnd, msg_cb_t cb);

      void resize_widget() const;

      void resize_webview() const;

      void focus_webview() const;

      bool is_webview2_available() const noexcept;

      void on_dpi_changed(int dpi);

      SIZE get_size() const;

      SIZE get_scaled_size(int from_dpi, int to_dpi) const;

      void on_system_setting_change(const wchar_t* area) const;

      // The app is expected to call CoInitializeEx before
      // CreateCoreWebView2EnvironmentWithOptions.
      // Source: https://docs.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/webview2-idl#createcorewebview2environmentwithoptions
      com_init_wrapper m_com_init;
      HWND m_window = nullptr;
      HWND m_widget = nullptr;
      HWND m_message_window = nullptr;
      POINT m_min_size = POINT{0, 0};
      POINT m_max_size = POINT{0, 0};
      DWORD m_main_thread = GetCurrentThreadId();
      ICoreWebView2* m_webview = nullptr;
      ICoreWebView2Controller* m_controller = nullptr;
      webview2_com_handler* m_com_handler = nullptr;
      mswebview2::loader m_webview2_loader;
      int m_dpi{};
      bool m_is_window_shown{};
    };
  } // namespace detail
} // namespace webview

#endif // defined(WEBVIEW_PLATFORM_WINDOWS) && defined(WEBVIEW_EDGE)
#endif // WEBVIEW_BACKENDS_WIN32_EDGE_HPP
