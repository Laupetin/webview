#pragma once

#ifndef WEBVIEW_DETAIL_PLATFORM_WINDOWS_LOADER_IMPL_HPP
#define WEBVIEW_DETAIL_PLATFORM_WINDOWS_LOADER_IMPL_HPP

#include "../../../macros.hpp"

#if defined(WEBVIEW_PLATFORM_WINDOWS) && defined(WEBVIEW_EDGE)

#include "../reg_key.hpp"
#include "loader.hpp"

#include <string>

#ifdef _MSC_VER
#pragma comment(lib, "ole32.lib")
#endif

namespace webview::detail
{
  namespace mswebview2
  {
    WEBVIEW_IMPL HRESULT loader::create_environment_with_options(const PCWSTR browser_dir,
                                                                 const PCWSTR user_data_dir,
                                                                 ICoreWebView2EnvironmentOptions* env_options,
                                                                 ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* created_handler) const
    {
#if WEBVIEW_MSWEBVIEW2_EXPLICIT_LINK == 1
      if (m_lib.is_loaded())
      {
        if (const auto fn = m_lib.get(webview2_symbols::CreateCoreWebView2EnvironmentWithOptions))
          return fn(browser_dir, user_data_dir, env_options, created_handler);
      }
#if WEBVIEW_MSWEBVIEW2_BUILTIN_IMPL == 1
      return create_environment_with_options_impl(browser_dir, user_data_dir, env_options, created_handler);
#else
      return S_FALSE;
#endif
#else
      return ::CreateCoreWebView2EnvironmentWithOptions(browser_dir, user_data_dir, env_options, created_handler);
#endif // WEBVIEW_MSWEBVIEW2_EXPLICIT_LINK
    }

    WEBVIEW_IMPL HRESULT loader::get_available_browser_version_string(const PCWSTR browser_dir, LPWSTR* version) const
    {
#if WEBVIEW_MSWEBVIEW2_EXPLICIT_LINK == 1
      if (m_lib.is_loaded())
      {
        if (const auto fn = m_lib.get(webview2_symbols::GetAvailableCoreWebView2BrowserVersionString))
          return fn(browser_dir, version);
      }
#if WEBVIEW_MSWEBVIEW2_BUILTIN_IMPL == 1
      return get_available_browser_version_string_impl(browser_dir, version);
#else
      return S_FALSE;
#endif
#else
      return ::GetAvailableCoreWebView2BrowserVersionString(browser_dir, version);
#endif // WEBVIEW_MSWEBVIEW2_EXPLICIT_LINK
    }

#if WEBVIEW_MSWEBVIEW2_BUILTIN_IMPL == 1
    WEBVIEW_IMPL loader::client_info_t::client_info_t(const bool found, std::wstring dll_path, std::wstring version, const webview2_runtime_type runtime_type)
        : found(found),
          dll_path(std::move(dll_path)),
          version(std::move(version)),
          runtime_type(runtime_type)
    {
    }

    WEBVIEW_IMPL HRESULT loader::create_environment_with_options_impl(const PCWSTR browser_dir,
                                                                      const PCWSTR user_data_dir,
                                                                      ICoreWebView2EnvironmentOptions* env_options,
                                                                      ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* created_handler) const
    {
      const auto found_client = find_available_client(browser_dir);
      if (!found_client.found)
        return -1;

      auto client_dll = native_library(found_client.dll_path);
      if (const auto fn = client_dll.get(webview2_symbols::CreateWebViewEnvironmentWithOptionsInternal))
        return fn(true, found_client.runtime_type, user_data_dir, env_options, created_handler);

      if (const auto fn = client_dll.get(webview2_symbols::DllCanUnloadNow))
      {
        if (!fn())
          client_dll.detach();
      }

      return ERROR_SUCCESS;
    }

    WEBVIEW_IMPL HRESULT loader::get_available_browser_version_string_impl(const PCWSTR browser_dir, LPWSTR* version) const
    {
      if (!version)
        return -1;

      const auto found_client = find_available_client(browser_dir);
      if (!found_client.found)
        return -1;

      const auto info_length_bytes = found_client.version.size() * sizeof(found_client.version[0]);
      const auto info = static_cast<LPWSTR>(CoTaskMemAlloc(info_length_bytes));
      if (!info)
        return -1;

      CopyMemory(info, found_client.version.c_str(), info_length_bytes);
      *version = info;

      return 0;
    }

    WEBVIEW_IMPL loader::client_info_t loader::find_available_client(const PCWSTR browser_dir) const
    {
      if (browser_dir)
        return find_embedded_client(API_VERSION, browser_dir);

      auto found_client = find_installed_client(API_VERSION, true, DEFAULT_RELEASE_CHANNEL_GUID);
      if (!found_client.found)
        found_client = find_installed_client(API_VERSION, false, DEFAULT_RELEASE_CHANNEL_GUID);

      return found_client;
    }

    WEBVIEW_IMPL std::wstring loader::make_client_dll_path(const std::wstring& dir)
    {
      auto dll_path = dir;
      if (!dll_path.empty())
      {
        const auto last_char = dir[dir.size() - 1];
        if (last_char != L'\\' && last_char != L'/')
          dll_path += L'\\';
      }
      dll_path += L"EBWebView\\";
#if defined(_M_X64) || defined(__x86_64__)
      dll_path += L"x64";
#elif defined(_M_IX86) || defined(__i386__)
      dll_path += L"x86";
#elif defined(_M_ARM64) || defined(__aarch64__)
      dll_path += L"arm64";
#else
#error WebView2 integration for this platform is not yet supported.
#endif
      dll_path += L"\\EmbeddedBrowserWebView.dll";

      return dll_path;
    }

    WEBVIEW_IMPL loader::client_info_t loader::find_installed_client(const unsigned int min_api_version, const bool system, const std::wstring& release_channel)
    {
      std::wstring sub_key = CLIENT_STATE_REG_SUB_KEY;
      sub_key += release_channel;

      auto root_key = system ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
      reg_key key(root_key, sub_key, 0, KEY_READ | KEY_WOW64_32KEY);

      if (!key.is_open())
        return {};

      auto ebwebview_value = key.query_string(L"EBWebView");

      auto client_version_string = get_last_native_path_component(ebwebview_value);
      auto client_version = parse_version(client_version_string);

      if (client_version[2] < min_api_version)
      {
        // Our API version is greater than the runtime API version.
        return {};
      }

      auto client_dll_path = make_client_dll_path(ebwebview_value);

      return {true, std::move(client_dll_path), std::move(client_version_string), webview2_runtime_type::INSTALLED};
    }

    WEBVIEW_IMPL loader::client_info_t loader::find_embedded_client(const unsigned int min_api_version, const std::wstring& dir)
    {
      auto client_dll_path = make_client_dll_path(dir);

      auto client_version_string = get_file_version_string(client_dll_path);
      auto client_version = parse_version(client_version_string);

      if (client_version[2] < min_api_version)
      {
        // Our API version is greater than the runtime API version.
        return {};
      }

      return {true, std::move(client_dll_path), std::move(client_version_string), webview2_runtime_type::EMBEDDED};
    }

#endif // WEBVIEW_MSWEBVIEW2_BUILTIN_IMPL
  } // namespace mswebview2
} // namespace webview::detail

#endif // defined(WEBVIEW_PLATFORM_WINDOWS) && defined(WEBVIEW_EDGE)
#endif // WEBVIEW_BACKENDS_WEBVIEW2_LOADER_HPP
