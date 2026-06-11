#pragma once

#ifndef WEBVIEW_DETAIL_PLATFORM_LINUX_APP_GTK
#define WEBVIEW_DETAIL_PLATFORM_LINUX_APP_GTK

#include "../../macros.hpp"

#if defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)

#include "../../app_base.hpp"

#include <gtk/gtk.h>

namespace webview
{
  namespace detail
  {
    class app_gtk final : public app_base
    {
  protected:
      noresult run_loop() override
      {
        while (!m_stop_run_loop)
          g_main_context_iteration(nullptr, TRUE);

        return {};
      }
    };
  } // namespace detail

  using app = detail::app_gtk;
} // namespace webview

#endif
#endif
