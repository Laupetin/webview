#pragma once

#ifndef WEBVIEW_DETAIL_PLATFORM_WINDOWS_APP_WIN32_HPP
#define WEBVIEW_DETAIL_PLATFORM_WINDOWS_APP_WIN32_HPP

#include "../../macros.hpp"

#if defined(WEBVIEW_PLATFORM_WINDOWS) && defined(WEBVIEW_EDGE)

#include "../../app_base.hpp"

namespace webview
{
  namespace detail
  {
    class app_win32 final : public app_base
    {
  public:
      void terminate() override;

  protected:
      noresult run_loop() override;
    };
  } // namespace detail

  using app = detail::app_win32;
} // namespace webview

#endif
#endif
