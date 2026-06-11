#pragma once

#ifndef WEBVIEW_DETAIL_PLATFORM_LINUX_APP_GTK_IMPL_HPP
#define WEBVIEW_DETAIL_PLATFORM_LINUX_APP_GTK_IMPL_HPP

#include "../../macros.hpp"

#if defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)

#include "app_gtk.hpp"

#include <gtk/gtk.h>

namespace webview::detail
{
  WEBVIEW_IMPL noresult app_gtk::run_loop()
  {
    while (!m_stop_run_loop)
      g_main_context_iteration(nullptr, TRUE);

    return {};
  }
} // namespace webview::detail

#endif
#endif
