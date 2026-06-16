#pragma once

#ifndef WEBWINDOWED_DETAIL_PLATFORM_WINDOWS_BACKEND_EDGE_IMPL_HPP
#define WEBWINDOWED_DETAIL_PLATFORM_WINDOWS_BACKEND_EDGE_IMPL_HPP

#include "../../macros.hpp"

#if defined(WEBWINDOWED_PLATFORM_WINDOWS) && defined(WEBWINDOWED_EDGE)

#include "../../errors.hpp"
#include "../../native_library.hpp"
#include "../../platform/windows/com_init_wrapper.hpp"
#include "../../platform/windows/dpi.hpp"
#include "../../platform/windows/iid.hpp"
#include "../../platform/windows/theme.hpp"
#include "../../platform/windows/webview2/loader.hpp"
#include "../../types.hpp"
#include "../../utility/string.hpp"
#include "../../window_base.hpp"
#include "backend_edge.hpp"

#include <atomic>
#include <cstdlib>
#include <functional>
#include <memory>
#include <utility>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <objbase.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <windows.h>
#include <wrl/event.h>

#ifdef _MSC_VER
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "version.lib")
#endif

namespace webwindowed::detail
{
  // ===================================
  // webview2_com_handler
  // ===================================

  WEBWINDOWED_IMPL webview2_com_handler::webview2_com_handler(const HWND hwnd, msg_cb_t msgCb, webview2_com_handler_cb_t cb)
      : m_window(hwnd),
        m_msg_cb(std::move(msgCb)),
        m_cb(std::move(cb))
  {
  }

  WEBWINDOWED_IMPL ULONG STDMETHODCALLTYPE webview2_com_handler::AddRef()
  {
    return ++m_ref_count;
  }

  WEBWINDOWED_IMPL ULONG STDMETHODCALLTYPE webview2_com_handler::Release()
  {
    if (m_ref_count > 1)
      return --m_ref_count;

    delete this;
    return 0;
  }

  WEBWINDOWED_IMPL HRESULT STDMETHODCALLTYPE webview2_com_handler::QueryInterface(REFIID riid, LPVOID* ppv)
  {
    using namespace mswebview2::cast_info;

    if (!ppv)
      return E_POINTER;

    // All of the COM interfaces we implement should be added here regardless
    // of whether they are required.
    // This is just to be on the safe side in case the WebView2 Runtime ever
    // requests a pointer to an interface we implement.
    // The WebView2 Runtime must at the very least be able to get a pointer to
    // ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler when we use
    // our custom WebView2 loader implementation, and observations have shown
    // that it is the only interface requested in this case. None have been
    // observed to be requested when using the official WebView2 loader.

    if (cast_if_equal_iid(this, riid, controller_completed, ppv) || cast_if_equal_iid(this, riid, environment_completed, ppv)
        || cast_if_equal_iid(this, riid, message_received, ppv) || cast_if_equal_iid(this, riid, permission_requested, ppv))
    {
      return S_OK;
    }

    return E_NOINTERFACE;
  }

  WEBWINDOWED_IMPL HRESULT STDMETHODCALLTYPE webview2_com_handler::Invoke(HRESULT res, ICoreWebView2Environment* env)
  {
    if (SUCCEEDED(res))
    {
      res = env->CreateCoreWebView2Controller(m_window, this);
      if (SUCCEEDED(res))
        return S_OK;
    }

    try_create_environment();
    return S_OK;
  }

  WEBWINDOWED_IMPL HRESULT STDMETHODCALLTYPE webview2_com_handler::Invoke(const HRESULT res, ICoreWebView2Controller* controller)
  {
    if (FAILED(res))
    {
      // See try_create_environment() regarding
      // HRESULT_FROM_WIN32(ERROR_INVALID_STATE).
      // The result is E_ABORT if the parent window has been destroyed already.
      switch (res)
      {
      case HRESULT_FROM_WIN32(ERROR_INVALID_STATE):
      case E_ABORT:
        return S_OK;

      default:
        break;
      }
      try_create_environment();
      return S_OK;
    }

    ICoreWebView2* webview;
    EventRegistrationToken token;
    controller->get_CoreWebView2(&webview);
    webview->add_WebMessageReceived(this, &token);
    webview->add_PermissionRequested(this, &token);

    m_cb(controller, webview);
    return S_OK;
  }

  WEBWINDOWED_IMPL HRESULT STDMETHODCALLTYPE webview2_com_handler::Invoke(ICoreWebView2* /*sender*/, ICoreWebView2WebMessageReceivedEventArgs* args)
  {
    LPWSTR message{};
    const auto res = args->TryGetWebMessageAsString(&message);
    if (SUCCEEDED(res))
      m_msg_cb(narrow_string(message));

    CoTaskMemFree(message);
    return S_OK;
  }

  WEBWINDOWED_IMPL HRESULT STDMETHODCALLTYPE webview2_com_handler::Invoke(ICoreWebView2* /*sender*/, ICoreWebView2PermissionRequestedEventArgs* args)
  {
    COREWEBVIEW2_PERMISSION_KIND kind;
    args->get_PermissionKind(&kind);
    if (kind == COREWEBVIEW2_PERMISSION_KIND_CLIPBOARD_READ)
      args->put_State(COREWEBVIEW2_PERMISSION_STATE_ALLOW);

    return S_OK;
  }

  WEBWINDOWED_IMPL void webview2_com_handler::set_attempt_handler(std::function<HRESULT()> attempt_handler) noexcept
  {
    m_attempt_handler = std::move(attempt_handler);
  }

  WEBWINDOWED_IMPL void webview2_com_handler::try_create_environment() noexcept
  {
    // WebView creation fails with HRESULT_FROM_WIN32(ERROR_INVALID_STATE) if
    // a running instance using the same user data folder exists, and the
    // Environment objects have different EnvironmentOptions.
    // Source: https://docs.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2environment?view=webview2-1.0.1150.38
    if (m_attempts < m_max_attempts)
    {
      ++m_attempts;
      const auto res = m_attempt_handler();
      if (SUCCEEDED(res))
        return;

      // Not entirely sure if this error code only applies to
      // CreateCoreWebView2Controller so we check here as well.
      if (res == HRESULT_FROM_WIN32(ERROR_INVALID_STATE))
        return;

      // Wait for m_sleep_ms before trying again.
      Sleep(m_sleep_ms);
      try_create_environment();
      return;
    }
    // Give up.
    m_cb(nullptr, nullptr);
  }

  // ===================================
  // webview2_user_script_added_handler
  // ===================================

  WEBWINDOWED_IMPL webview2_user_script_added_handler::webview2_user_script_added_handler(callback_fn cb)
      : m_cb(std::move(cb))
  {
  }

  WEBWINDOWED_IMPL ULONG STDMETHODCALLTYPE webview2_user_script_added_handler::AddRef()
  {
    return ++m_ref_count;
  }

  WEBWINDOWED_IMPL ULONG STDMETHODCALLTYPE webview2_user_script_added_handler::Release()
  {
    if (m_ref_count > 1)
      return --m_ref_count;

    delete this;
    return 0;
  }

  WEBWINDOWED_IMPL HRESULT STDMETHODCALLTYPE webview2_user_script_added_handler::QueryInterface(REFIID riid, LPVOID* ppv)
  {
    using namespace mswebview2::cast_info;

    if (!ppv)
      return E_POINTER;

    if (cast_if_equal_iid(this, riid, add_script_to_execute_on_document_created_completed, ppv))
      return S_OK;

    return E_NOINTERFACE;
  }

  WEBWINDOWED_IMPL HRESULT STDMETHODCALLTYPE webview2_user_script_added_handler::Invoke(const HRESULT res, const LPCWSTR id)
  {
    m_cb(res, id);
    return S_OK;
  }

  // ===================================
  // win32_edge_engine
  // ===================================

  WEBWINDOWED_IMPL win32_edge_engine::win32_edge_engine() {}

  WEBWINDOWED_IMPL win32_edge_engine::~win32_edge_engine()
  {
    if (m_com_handler)
    {
      m_com_handler->Release();
      m_com_handler = nullptr;
    }

    if (m_webview)
    {
      m_webview->Release();
      m_webview = nullptr;
    }

    if (m_controller)
    {
      m_controller->Release();
      m_controller = nullptr;
    }

    // Replace wndproc to avoid callbacks and other bad things during
    // destruction.
    const auto wnd_proc = reinterpret_cast<LONG_PTR>(+[](const HWND hwnd, const UINT msg, const WPARAM wp, const LPARAM lp) -> LRESULT
                                                     {
                                                       return DefWindowProcW(hwnd, msg, wp, lp);
                                                     });

    if (m_widget)
      SetWindowLongPtrW(m_widget, GWLP_WNDPROC, wnd_proc);

    if (m_window)
      SetWindowLongPtrW(m_window, GWLP_WNDPROC, wnd_proc);

    if (m_widget)
    {
      DestroyWindow(m_widget);
      m_widget = nullptr;
    }

    if (m_window)
    {
      DestroyWindow(m_window);
      on_window_destroyed();

      m_window = nullptr;
    }

    // Not strictly needed for windows to close immediately but aligns
    // behavior across backends.
    deplete_run_loop_event_queue();

    // We need the message window in order to deplete the event queue.
    if (m_message_window)
    {
      SetWindowLongPtrW(m_message_window, GWLP_WNDPROC, wnd_proc);
      DestroyWindow(m_message_window);
      m_message_window = nullptr;
    }
  }

  WEBWINDOWED_IMPL noresult win32_edge_engine::eval(const std::string& js)
  {
    // TODO: Skip if no content has begun loading yet. Can't check with
    //       ICoreWebView2::get_Source because it returns "about:blank".
    const auto wide_js = widen_string(js);
    const auto res = m_webview->ExecuteScript(wide_js.c_str(), nullptr);
    if (FAILED(res))
      return std::unexpected(error_info{webwindowed_error::UNSPECIFIED, "ExecuteScript failed"});

    return {};
  }

  WEBWINDOWED_IMPL void win32_edge_engine::dispatch_impl(dispatch_fn_t f)
  {
    PostMessageW(m_message_window, WM_APP, 0, reinterpret_cast<LPARAM>(new dispatch_fn_t(std::move(f))));
  }

  WEBWINDOWED_IMPL void win32_edge_engine::set_window_size_impl(const unsigned width, const unsigned height)
  {
    const auto dpi = get_window_dpi(m_window);
    m_dpi = dpi;

    const auto scaled_size = scale_size(width, height, get_default_window_dpi(), dpi);
    const auto frame_size = make_window_frame_size(m_window, scaled_size.cx, scaled_size.cy, dpi);
    SetWindowPos(m_window, nullptr, 0, 0, frame_size.cx, frame_size.cy, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE | SWP_FRAMECHANGED);
  }

  WEBWINDOWED_IMPL void win32_edge_engine::set_window_min_impl(const unsigned width, const unsigned height)
  {
    m_min_size.x = width;
    m_min_size.y = height;
  }

  WEBWINDOWED_IMPL void win32_edge_engine::set_window_max_impl(const unsigned width, const unsigned height)
  {
    m_max_size.x = width;
    m_max_size.y = height;
  }

  WEBWINDOWED_IMPL void win32_edge_engine::set_window_size_fixed_impl(const bool value)
  {
    auto style = GetWindowLong(m_window, GWL_STYLE);
    if (value)
      style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    else
      style |= (WS_THICKFRAME | WS_MAXIMIZEBOX);

    SetWindowLong(m_window, GWL_STYLE, style);
  }

  WEBWINDOWED_IMPL noresult win32_edge_engine::set_html_impl(const std::string& html)
  {
    const auto res = m_webview->NavigateToString(widen_string(html).c_str());
    if (FAILED(res))
      return std::unexpected(error_info{webwindowed_error::UNSPECIFIED, "NavigateToString failed"});

    return {};
  }

  WEBWINDOWED_IMPL noresult win32_edge_engine::navigate_impl(const std::string& url)
  {
    const auto wide_url = widen_string(url);
    const auto res = m_webview->Navigate(wide_url.c_str());
    if (FAILED(res))
      return std::unexpected(error_info{webwindowed_error::UNSPECIFIED, "Navigate failed"});

    return {};
  }

  WEBWINDOWED_IMPL void win32_edge_engine::set_title_impl(const std::string& title)
  {
    SetWindowTextW(m_window, widen_string(title).c_str());
  }

  WEBWINDOWED_IMPL noresult win32_edge_engine::add_page_init_script(const std::string& js)
  {
    const auto wide_js = widen_string(js);
    bool done{};

    webview2_user_script_added_handler handler{[&](const HRESULT res, const LPCWSTR id)
                                               {
                                                 done = true;
                                               }};

    const auto res = m_webview->AddScriptToExecuteOnDocumentCreated(wide_js.c_str(), &handler);
    if (SUCCEEDED(res))
    {
      // Sadly we need to pump the event loop in order to get the script ID.
      run_event_loop_while(
          [&]
          {
            return !done;
          });
    }

    return {};
  }

  WEBWINDOWED_IMPL noresult win32_edge_engine::on_window_opened_impl()
  {
    auto result = window_init();
    if (!result.has_value())
      return std::move(result);

    result = window_settings();
    if (!result.has_value())
      return std::move(result);

    m_is_initialized = true;

    dispatch_size_default();

    window_show();

    return {};
  }

  WEBWINDOWED_IMPL void win32_edge_engine::run_event_loop_while(const std::function<bool()>& fn)
  {
    MSG msg;
    while (fn() && GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }
  }

  WEBWINDOWED_IMPL window* win32_edge_engine::downcast_this()
  {
    return this;
  }

  WEBWINDOWED_IMPL noresult win32_edge_engine::window_init()
  {
    if (!is_webview2_available())
      return std::unexpected(error_info{webwindowed_error::MISSING_DEPENDENCY, "WebView2 is unavailable"});

    const HINSTANCE h_instance = GetModuleHandle(nullptr);

    auto com_init = com_init_wrapper::create(COINIT_APARTMENTTHREADED);
    if (!com_init.has_value())
      return std::unexpected(std::move(com_init).error());
    m_com_init = std::move(com_init).value();

    enable_dpi_awareness();

    const auto icon =
        static_cast<HICON>(LoadImage(h_instance, IDI_APPLICATION, IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR));

    // Create a top-level window.
    WNDCLASSEXW wc;
    ZeroMemory(&wc, sizeof(WNDCLASSEX));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.hInstance = h_instance;
    wc.lpszClassName = L"webview";
    wc.hIcon = icon;
    wc.lpfnWndProc = [](const HWND hwnd, const UINT msg, const WPARAM wp, const LPARAM lp) -> LRESULT
    {
      win32_edge_engine* w;

      if (msg == WM_NCCREATE)
      {
        auto* lpcs{reinterpret_cast<LPCREATESTRUCT>(lp)};
        w = static_cast<win32_edge_engine*>(lpcs->lpCreateParams);
        w->m_window = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(w));
        enable_non_client_dpi_scaling_if_needed(hwnd);
        apply_window_theme(hwnd);
      }
      else
      {
        w = reinterpret_cast<win32_edge_engine*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
      }

      if (!w)
      {
        return DefWindowProcW(hwnd, msg, wp, lp);
      }

      switch (msg)
      {
      case WM_SIZE:
        w->resize_widget();
        break;

      case WM_CLOSE:
        DestroyWindow(hwnd);
        break;

      case WM_DESTROY:
        w->m_window = nullptr;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        w->on_window_destroyed();
        break;

      case WM_GETMINMAXINFO:
      {
        auto lpmmi = reinterpret_cast<LPMINMAXINFO>(lp);

        if (w->m_max_size.x > 0 && w->m_max_size.y > 0)
        {
          lpmmi->ptMaxSize = w->m_max_size;
          lpmmi->ptMaxTrackSize = w->m_max_size;
        }
        if (w->m_min_size.x > 0 && w->m_min_size.y > 0)
        {
          lpmmi->ptMinTrackSize = w->m_min_size;
        }
      }
      break;

      case WM_GETDPISCALEDSIZE:
      {
        auto dpi = static_cast<int>(wp);
        auto* size{reinterpret_cast<SIZE*>(lp)};
        *size = w->get_scaled_size(w->m_dpi, dpi);
        return TRUE;
      }

      case WM_DPICHANGED:
      {
        // Windows 10: The size we get here is exactly what we supplied to WM_GETDPISCALEDSIZE.
        // Windows 11: The size we get here is NOT what we supplied to WM_GETDPISCALEDSIZE.
        // Due to this difference, don't use the suggested bounds.
        const auto dpi = static_cast<int>(HIWORD(wp));
        w->on_dpi_changed(dpi);
        break;
      }

      case WM_SETTINGCHANGE:
      {
        auto* area = reinterpret_cast<const wchar_t*>(lp);
        if (area)
          w->on_system_setting_change(area);
        break;
      }

      case WM_ACTIVATE:
        if (LOWORD(wp) != WA_INACTIVE)
          w->focus_webview();
        break;

      default:
        return DefWindowProcW(hwnd, msg, wp, lp);
      }

      return 0;
    };

    RegisterClassExW(&wc);

    CreateWindowW(L"webview", L"", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, nullptr, nullptr, h_instance, this);

    if (!m_window)
      return std::unexpected(error_info{webwindowed_error::INVALID_STATE, "Window is null"});

    m_dpi = get_window_dpi(m_window);

    // Create a window that WebView2 will be embedded into.
    WNDCLASSEXW widget_wc{};
    widget_wc.cbSize = sizeof(WNDCLASSEX);
    widget_wc.hInstance = h_instance;
    widget_wc.lpszClassName = L"webview_widget";
    widget_wc.lpfnWndProc = [](const HWND hwnd, const UINT msg, const WPARAM wp, const LPARAM lp) -> LRESULT
    {
      win32_edge_engine* w;

      if (msg == WM_NCCREATE)
      {
        auto* lpcs{reinterpret_cast<LPCREATESTRUCT>(lp)};
        w = static_cast<win32_edge_engine*>(lpcs->lpCreateParams);
        w->m_widget = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(w));
      }
      else
      {
        w = reinterpret_cast<win32_edge_engine*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
      }

      if (!w)
      {
        return DefWindowProcW(hwnd, msg, wp, lp);
      }

      switch (msg)
      {
      case WM_SIZE:
        w->resize_webview();
        break;

      case WM_DESTROY:
        w->m_widget = nullptr;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        break;

      default:
        return DefWindowProcW(hwnd, msg, wp, lp);
      }

      return 0;
    };

    RegisterClassExW(&widget_wc);
    CreateWindowExW(WS_EX_CONTROLPARENT, L"webview_widget", nullptr, WS_CHILD, 0, 0, 0, 0, m_window, nullptr, h_instance, this);
    if (!m_widget)
      return std::unexpected(error_info{webwindowed_error::INVALID_STATE, "Widget window is null"});

    // Create a message-only window for internal messaging.
    WNDCLASSEXW message_wc{};
    message_wc.cbSize = sizeof(WNDCLASSEX);
    message_wc.hInstance = h_instance;
    message_wc.lpszClassName = L"webview_message";
    message_wc.lpfnWndProc = [](const HWND hwnd, const UINT msg, const WPARAM wp, const LPARAM lp) -> LRESULT
    {
      win32_edge_engine* w;

      if (msg == WM_NCCREATE)
      {
        const auto* lpcs{reinterpret_cast<LPCREATESTRUCT>(lp)};
        w = static_cast<win32_edge_engine*>(lpcs->lpCreateParams);
        w->m_message_window = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(w));
      }
      else
      {
        w = reinterpret_cast<win32_edge_engine*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
      }

      if (!w)
        return DefWindowProcW(hwnd, msg, wp, lp);

      switch (msg)
      {
      case WM_APP:
        if (const auto f = reinterpret_cast<dispatch_fn_t*>(lp))
        {
          (*f)();
          delete f;
        }
        break;

      case WM_DESTROY:
        w->m_message_window = nullptr;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        break;

      default:
        return DefWindowProcW(hwnd, msg, wp, lp);
      }
      return 0;
    };

    RegisterClassExW(&message_wc);
    CreateWindowExW(0, L"webview_message", nullptr, 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, h_instance, this);

    if (!m_message_window)
      return std::unexpected(error_info{webwindowed_error::INVALID_STATE, "Message window is null"});

    return {};
  }

  WEBWINDOWED_IMPL noresult win32_edge_engine::window_settings()
  {
    auto cb = std::bind(&win32_edge_engine::on_message, this, std::placeholders::_1);
    return embed(m_widget, cb);
  }

  WEBWINDOWED_IMPL void win32_edge_engine::window_show()
  {
    if (!m_is_window_shown)
    {
      ShowWindow(m_window, SW_SHOW);
      UpdateWindow(m_window);
      SetFocus(m_window);
      m_is_window_shown = true;
    }
  }

  WEBWINDOWED_IMPL noresult win32_edge_engine::embed(const HWND wnd, msg_cb_t cb)
  {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    flag.test_and_set();

    wchar_t current_exe_path[MAX_PATH];
    GetModuleFileNameW(nullptr, current_exe_path, MAX_PATH);
    const wchar_t* current_exe_name = PathFindFileNameW(current_exe_path);

    wchar_t data_path[MAX_PATH];
    if (!SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, data_path)))
      return std::unexpected(error_info{webwindowed_error::UNSPECIFIED, "SHGetFolderPathW failed"});

    wchar_t user_data_folder[MAX_PATH];
    PathCombineW(user_data_folder, data_path, current_exe_name);

    m_com_handler = new webview2_com_handler(wnd,
                                             std::move(cb),
                                             [&](ICoreWebView2Controller* controller, ICoreWebView2* webview)
                                             {
                                               if (!controller || !webview)
                                               {
                                                 flag.clear();
                                                 return;
                                               }

                                               controller->AddRef();
                                               webview->AddRef();
                                               m_controller = controller;
                                               m_webview = webview;
                                               flag.clear();
                                             });

    m_com_handler->set_attempt_handler(
        [&]
        {
          const auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
          Microsoft::WRL::ComPtr<ICoreWebView2EnvironmentOptions> interface_options;
          if (FAILED(options->QueryInterface(IID_PPV_ARGS(&interface_options))))
            return HRESULT_FROM_WIN32(ERROR_INVALID_STATE);

          const auto result = call_plugin_setup_environment_options(*this, interface_options.Get());
          if (!result.has_value())
            return HRESULT_FROM_WIN32(ERROR_INVALID_STATE);

          return m_webview2_loader.create_environment_with_options(nullptr, user_data_folder, options.Get(), m_com_handler);
        });
    m_com_handler->try_create_environment();

    // Pump the message loop until WebView2 has finished initialization.
    bool got_quit_msg = false;
    MSG msg;

    while (flag.test_and_set() && GetMessageW(&msg, nullptr, 0, 0) >= 0)
    {
      if (msg.message == WM_QUIT)
      {
        got_quit_msg = true;
        break;
      }

      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }

    if (got_quit_msg)
      return std::unexpected(error_info{webwindowed_error::CANCELED});

    if (!m_controller || !m_webview)
      return std::unexpected(error_info{webwindowed_error::INVALID_STATE});

    ICoreWebView2Settings* settings = nullptr;
    auto res = m_webview->get_Settings(&settings);
    if (res != S_OK)
      return std::unexpected(error_info{webwindowed_error::UNSPECIFIED, "get_Settings failed"});

    res = settings->put_AreDevToolsEnabled(m_debug ? TRUE : FALSE);
    if (res != S_OK)
      return std::unexpected(error_info{webwindowed_error::UNSPECIFIED, "put_AreDevToolsEnabled failed"});

    res = settings->put_IsStatusBarEnabled(FALSE);
    if (res != S_OK)
      return std::unexpected(error_info{webwindowed_error::UNSPECIFIED, "put_IsStatusBarEnabled failed"});

    auto res_expect = add_page_init_script(create_webwindowed_init_script("(message) => window.chrome.webview.postMessage(message)"));
    if (!res_expect.has_value())
      return std::move(res_expect);
    res_expect = add_page_init_script(create_bind_script());
    if (!res_expect.has_value())
      return std::move(res_expect);

    resize_webview();

    res_expect = call_plugin_setup_window(*this, plugin_window_context(m_window, m_widget, m_controller));
    if (!res_expect.has_value())
      return std::move(res_expect);

    m_controller->put_IsVisible(TRUE);

    ShowWindow(m_widget, SW_SHOW);
    UpdateWindow(m_widget);
    focus_webview();

    return {};
  }

  WEBWINDOWED_IMPL void win32_edge_engine::resize_widget() const
  {
    if (m_widget)
    {
      RECT r{};

      if (GetClientRect(GetParent(m_widget), &r))
        MoveWindow(m_widget, r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE);
    }
  }

  WEBWINDOWED_IMPL void win32_edge_engine::resize_webview() const
  {
    if (m_widget && m_controller)
    {
      RECT bounds{};
      if (GetClientRect(m_widget, &bounds))
        m_controller->put_Bounds(bounds);
    }
  }

  WEBWINDOWED_IMPL void win32_edge_engine::focus_webview() const
  {
    if (m_controller)
      m_controller->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
  }

  WEBWINDOWED_IMPL bool win32_edge_engine::is_webview2_available() const noexcept
  {
    LPWSTR version_info = nullptr;
    const auto res = m_webview2_loader.get_available_browser_version_string(nullptr, &version_info);
    // The result will be equal to HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)
    // if the WebView2 runtime is not installed.
    const auto ok = SUCCEEDED(res) && version_info;
    if (version_info)
      CoTaskMemFree(version_info);

    return ok;
  }

  WEBWINDOWED_IMPL void win32_edge_engine::on_dpi_changed(const int dpi)
  {
    const auto scaled_size = get_scaled_size(m_dpi, dpi);
    const auto frame_size = make_window_frame_size(m_window, scaled_size.cx, scaled_size.cy, dpi);
    SetWindowPos(m_window, nullptr, 0, 0, frame_size.cx, frame_size.cy, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE | SWP_FRAMECHANGED);
    m_dpi = dpi;
  }

  WEBWINDOWED_IMPL SIZE win32_edge_engine::get_size() const
  {
    RECT bounds;
    GetClientRect(m_window, &bounds);
    const auto width = bounds.right - bounds.left;
    const auto height = bounds.bottom - bounds.top;

    return {width, height};
  }

  WEBWINDOWED_IMPL SIZE win32_edge_engine::get_scaled_size(const int from_dpi, const int to_dpi) const
  {
    const auto size = get_size();

    return scale_size(size.cx, size.cy, from_dpi, to_dpi);
  }

  WEBWINDOWED_IMPL void win32_edge_engine::on_system_setting_change(const wchar_t* area) const
  {
    // Detect light/dark mode change in system.
    if (lstrcmpW(area, L"ImmersiveColorSet") == 0)
      apply_window_theme(m_window);
  }
} // namespace webwindowed::detail

#endif // defined(WEBWINDOWED_PLATFORM_WINDOWS) && defined(WEBWINDOWED_EDGE)
#endif // WEBWINDOWED_BACKENDS_WIN32_EDGE_HPP
