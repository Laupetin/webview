#pragma once

#ifndef WEBVIEW_DETAIL_WINDOW_FWD_HPP
#define WEBVIEW_DETAIL_WINDOW_FWD_HPP

#include "macros.hpp"

namespace webview
{
  namespace detail
  {
    class window_base;

#if defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)
    class gtk_webkit_engine;
#elif defined(WEBVIEW_PLATFORM_WINDOWS) && defined(WEBVIEW_EDGE)
    class win32_edge_engine;
#endif
  } // namespace detail

#if defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)
  using window = detail::gtk_webkit_engine;
#elif defined(WEBVIEW_PLATFORM_WINDOWS) && defined(WEBVIEW_EDGE)
  using window = detail::win32_edge_engine;
#endif
} // namespace webview

#endif
