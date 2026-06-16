#pragma once

#ifndef WEBWINDOWED_DETAIL_ERRORS_IMPL_HPP
#define WEBWINDOWED_DETAIL_ERRORS_IMPL_HPP

#include "errors.hpp"
#include "macros.hpp"

namespace webwindowed
{
  WEBWINDOWED_IMPL error_info::error_info()
      : m_code(webwindowed_error::UNSPECIFIED)
  {
  }

  WEBWINDOWED_IMPL error_info::error_info(const webwindowed_error code, std::string message) noexcept
      : m_code(code),
        m_message(std::move(message))
  {
  }

  WEBWINDOWED_IMPL webwindowed_error error_info::code() const
  {
    return m_code;
  }

  WEBWINDOWED_IMPL const std::string& error_info::message() const
  {
    return m_message;
  }
} // namespace webwindowed

#endif // WEBWINDOWED_ERRORS_HPP
