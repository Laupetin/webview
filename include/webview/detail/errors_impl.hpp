#pragma once

#ifndef WEBVIEW_DETAIL_ERRORS_IMPL_HPP
#define WEBVIEW_DETAIL_ERRORS_IMPL_HPP

#include "errors.hpp"
#include "macros.hpp"

namespace webview
{
  WEBVIEW_IMPL error_info::error_info()
      : m_code(webview_error::UNSPECIFIED)
  {
  }

  WEBVIEW_IMPL error_info::error_info(const webview_error code, std::string message) noexcept
      : m_code(code),
        m_message(std::move(message))
  {
  }

  WEBVIEW_IMPL webview_error error_info::code() const
  {
    return m_code;
  }

  WEBVIEW_IMPL const std::string& error_info::message() const
  {
    return m_message;
  }
} // namespace webview

#endif // WEBVIEW_ERRORS_HPP
