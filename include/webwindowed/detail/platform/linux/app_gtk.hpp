#pragma once

#ifndef WEBWINDOWED_DETAIL_PLATFORM_LINUX_APP_GTK_HPP
#define WEBWINDOWED_DETAIL_PLATFORM_LINUX_APP_GTK_HPP

#include "../../macros.hpp"

#if defined(WEBWINDOWED_PLATFORM_LINUX) && defined(WEBWINDOWED_GTK)

#include "../../app_base.hpp"

namespace webwindowed
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
} // namespace webwindowed

#endif
#endif
