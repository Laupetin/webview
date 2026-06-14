#pragma once

#ifndef WEBVIEW_DETAIL_PLATFORM_WINDOWS_REG_KEY_IMPL_HPP
#define WEBVIEW_DETAIL_PLATFORM_WINDOWS_REG_KEY_IMPL_HPP

#include "../../macros.hpp"

#if defined(WEBVIEW_PLATFORM_WINDOWS)

#include "reg_key.hpp"

#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#ifdef _MSC_VER
#pragma comment(lib, "advapi32.lib")
#endif

namespace webview::detail
{
  WEBVIEW_IMPL reg_key::reg_key(const HKEY root_key, const wchar_t* sub_key, const DWORD options, const REGSAM sam_desired)
  {
    HKEY handle;
    const auto status = RegOpenKeyExW(root_key, sub_key, options, sam_desired, &handle);

    if (status == ERROR_SUCCESS)
      m_handle = handle;
  }

  WEBVIEW_IMPL reg_key::reg_key(const HKEY root_key, const std::wstring& sub_key, const DWORD options, const REGSAM sam_desired)
      : reg_key(root_key, sub_key.c_str(), options, sam_desired)
  {
  }

  WEBVIEW_IMPL reg_key::~reg_key()
  {
    if (m_handle)
    {
      RegCloseKey(m_handle);
      m_handle = nullptr;
    }
  }

  WEBVIEW_IMPL bool reg_key::is_open() const
  {
    return !!m_handle;
  }

  WEBVIEW_IMPL bool reg_key::get_handle() const
  {
    return m_handle;
  }

  WEBVIEW_IMPL std::wstring reg_key::query_string(const wchar_t* name) const
  {
    std::wstring result;
    query_bytes(name, result);

    // Remove trailing null-characters.
    for (std::size_t length = result.size(); length > 0; --length)
    {
      if (result[length - 1] != 0)
      {
        result.resize(length);
        break;
      }
    }

    return result;
  }

  WEBVIEW_IMPL unsigned int reg_key::query_uint(const wchar_t* name, const unsigned int default_value) const
  {
    std::vector<char> data;
    query_bytes(name, data);
    if (data.size() < sizeof(DWORD))
      return default_value;

    return static_cast<unsigned int>(*reinterpret_cast<DWORD*>(data.data()));
  }
} // namespace webview::detail

#endif // defined(WEBVIEW_PLATFORM_WINDOWS)
#endif // WEBVIEW_PLATFORM_WINDOWS_REG_KEY_HPP
