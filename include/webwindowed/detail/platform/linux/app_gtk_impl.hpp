#pragma once

#ifndef WEBWINDOWED_DETAIL_PLATFORM_LINUX_APP_GTK_IMPL_HPP
#define WEBWINDOWED_DETAIL_PLATFORM_LINUX_APP_GTK_IMPL_HPP

#include "../../macros.hpp"

#if defined(WEBWINDOWED_PLATFORM_LINUX) && defined(WEBWINDOWED_GTK)

#include "app_gtk.hpp"

#include <gtk/gtk.h>

namespace webwindowed::detail
{
  WEBWINDOWED_IMPL noresult app_gtk::run_loop()
  {
    while (!m_stop_run_loop)
      g_main_context_iteration(nullptr, TRUE);

    return {};
  }
} // namespace webwindowed::detail

#endif
#endif
