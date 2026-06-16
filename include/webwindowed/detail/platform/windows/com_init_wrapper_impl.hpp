#pragma once

#ifndef WEBWINDOWED_DETAIL_PLATFORM_WINDOWS_COM_INIT_WRAPPER_IMPL_HPP
#define WEBWINDOWED_DETAIL_PLATFORM_WINDOWS_COM_INIT_WRAPPER_IMPL_HPP

#include "../../macros.hpp"

#if defined(WEBWINDOWED_PLATFORM_WINDOWS)

#include "com_init_wrapper.hpp"

#include <objbase.h>

#ifdef _MSC_VER
#pragma comment(lib, "ole32.lib")
#endif

namespace webwindowed::detail
{
  WEBWINDOWED_IMPL com_init_wrapper::com_init_wrapper()
      : m_initialized(false)
  {
  }

  WEBWINDOWED_IMPL com_init_wrapper::com_init_wrapper(const bool initialized)
      : m_initialized(initialized)
  {
  }

  WEBWINDOWED_IMPL com_init_wrapper::~com_init_wrapper()
  {
    if (m_initialized)
    {
      CoUninitialize();
      m_initialized = false;
    }
  }

  WEBWINDOWED_IMPL com_init_wrapper::com_init_wrapper(com_init_wrapper&& other) noexcept
  {
    *this = std::move(other);
  }

  WEBWINDOWED_IMPL com_init_wrapper& com_init_wrapper::operator=(com_init_wrapper&& other) noexcept
  {
    if (this == &other)
      return *this;

    m_initialized = other.m_initialized;
    other.m_initialized = false;

    return *this;
  }

  WEBWINDOWED_IMPL result<com_init_wrapper> com_init_wrapper::create(const DWORD dwCoInit)
  {
    // We can safely continue as long as COM was either successfully
    // initialized or already initialized.
    // RPC_E_CHANGED_MODE means that CoInitializeEx was already called with
    // a different concurrency model.
    switch (CoInitializeEx(nullptr, dwCoInit))
    {
    case S_OK:
    case S_FALSE:
      return com_init_wrapper(true);

    case RPC_E_CHANGED_MODE:
      return std::unexpected(error_info{webwindowed_error::INVALID_STATE, "CoInitializeEx already called with a different concurrency model"});

    default:
      return std::unexpected(error_info{webwindowed_error::UNSPECIFIED, "Unexpected result from CoInitializeEx"});
    }
  }
} // namespace webwindowed::detail

#endif // defined(WEBWINDOWED_PLATFORM_WINDOWS)
#endif // WEBWINDOWED_PLATFORM_WINDOWS_COM_INIT_WRAPPER_HPP
