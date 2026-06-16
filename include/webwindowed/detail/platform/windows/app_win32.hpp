#pragma once

#ifndef WEBWINDOWED_DETAIL_PLATFORM_WINDOWS_APP_WIN32_HPP
#define WEBWINDOWED_DETAIL_PLATFORM_WINDOWS_APP_WIN32_HPP

#include "../../macros.hpp"

#if defined(WEBWINDOWED_PLATFORM_WINDOWS) && defined(WEBWINDOWED_EDGE)

#include "../../app_base.hpp"

namespace webwindowed
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
} // namespace webwindowed

#endif
#endif
