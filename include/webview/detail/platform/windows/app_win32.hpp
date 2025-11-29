#pragma once

#ifndef WEBVIEW_DETAIL_PLATFORM_WINDOWS_APP_WIN32
#define WEBVIEW_DETAIL_PLATFORM_WINDOWS_APP_WIN32

#include "../../../macros.hpp"
#include "../../app_base.hpp"

#if defined(WEBVIEW_PLATFORM_WINDOWS) && defined(WEBVIEW_EDGE)

#include <windows.h>

namespace webview
{
  namespace detail
  {

    class app_win32 final : public app_base
    {
  public:
      void terminate() override
      {
        app_base::terminate();
        PostQuitMessage(0);
      }

  protected:
      noresult run_loop() override
      {
        MSG msg;
        while (!m_stop_run_loop && GetMessageW(&msg, nullptr, 0, 0) > 0)
        {
          TranslateMessage(&msg);
          DispatchMessageW(&msg);
        }

        return {};
      }
    };
  } // namespace detail

  using app = detail::app_win32;
} // namespace webview

#endif
#endif
