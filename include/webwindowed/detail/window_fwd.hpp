#pragma once

#ifndef WEBWINDOWED_DETAIL_WINDOW_FWD_HPP
#define WEBWINDOWED_DETAIL_WINDOW_FWD_HPP

#include "macros.hpp"

namespace webwindowed
{
  namespace detail
  {
    class window_base;

#if defined(WEBWINDOWED_PLATFORM_LINUX) && defined(WEBWINDOWED_GTK)
    class gtk_webkit_engine;
#elif defined(WEBWINDOWED_PLATFORM_WINDOWS) && defined(WEBWINDOWED_EDGE)
    class win32_edge_engine;
#endif
  } // namespace detail

#if defined(WEBWINDOWED_PLATFORM_LINUX) && defined(WEBWINDOWED_GTK)
  using window = detail::gtk_webkit_engine;
#elif defined(WEBWINDOWED_PLATFORM_WINDOWS) && defined(WEBWINDOWED_EDGE)
  using window = detail::win32_edge_engine;
#endif
} // namespace webwindowed

#endif
