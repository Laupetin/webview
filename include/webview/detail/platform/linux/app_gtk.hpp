#pragma once

#ifndef WEBVIEW_DETAIL_PLATFORM_LINUX_APP_GTK_HPP
#define WEBVIEW_DETAIL_PLATFORM_LINUX_APP_GTK_HPP

#include "../../macros.hpp"

#if defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)

#include "../../app_base.hpp"

namespace webview
{
  namespace detail
  {
    class app_gtk final : public app_base
    {
  protected:
      noresult run_loop() override;
    };
  } // namespace detail

  using app = detail::app_gtk;
} // namespace webview

#endif
#endif
