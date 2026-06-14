/*
 * MIT License
 *
 * Copyright (c) 2017 Serge Zaitsev
 * Copyright (c) 2022 Steffen André Langnes
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#ifndef WEBVIEW_DETAIL_PLATFORM_WINDOWS_LOADER_HPP
#define WEBVIEW_DETAIL_PLATFORM_WINDOWS_LOADER_HPP

#include "../../../macros.hpp"

#if defined(WEBVIEW_PLATFORM_WINDOWS) && defined(WEBVIEW_EDGE)

#include "../../../native_library.hpp"
#include "../iid.hpp"
#include "../version.hpp"

#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "WebView2.h" // amalgamate(skip)

#include <objbase.h>
#include <windows.h>

namespace webview::detail
{
  // Enable built-in WebView2Loader implementation by default.
#ifndef WEBVIEW_MSWEBVIEW2_BUILTIN_IMPL
#define WEBVIEW_MSWEBVIEW2_BUILTIN_IMPL 1
#endif

  // Link WebView2Loader.dll explicitly by default only if the built-in
  // implementation is enabled.
#ifndef WEBVIEW_MSWEBVIEW2_EXPLICIT_LINK
#define WEBVIEW_MSWEBVIEW2_EXPLICIT_LINK WEBVIEW_MSWEBVIEW2_BUILTIN_IMPL
#endif

  // Explicit linking of WebView2Loader.dll should be used along with
  // the built-in implementation.
#if WEBVIEW_MSWEBVIEW2_BUILTIN_IMPL == 1 && WEBVIEW_MSWEBVIEW2_EXPLICIT_LINK != 1
#undef WEBVIEW_MSWEBVIEW2_EXPLICIT_LINK
#error Please set WEBVIEW_MSWEBVIEW2_EXPLICIT_LINK=1.
#endif

#if WEBVIEW_MSWEBVIEW2_BUILTIN_IMPL == 1
  // Gets the last component of a Windows native file path.
  // For example, if the path is "C:\a\b" then the result is "b".
  template<typename T> std::basic_string<T> get_last_native_path_component(const std::basic_string<T>& path)
  {
    auto pos = path.find_last_of(static_cast<T>('\\'));
    if (pos != std::basic_string<T>::npos)
      return path.substr(pos + 1);

    return std::basic_string<T>();
  }
#endif // WEBVIEW_MSWEBVIEW2_BUILTIN_IMPL

  namespace mswebview2
  {
    static constexpr IID IID_ICoreWebView2CreateCoreWebView2ControllerCompletedHandler{
        0x6C4819F3, 0xC9B7, 0x4260, {0x81, 0x27, 0xC9, 0xF5, 0xBD, 0xE7, 0xF6, 0x8C}
    };
    static constexpr IID IID_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler{
        0x4E8A3389, 0xC9D8, 0x4BD2, {0xB6, 0xB5, 0x12, 0x4F, 0xEE, 0x6C, 0xC1, 0x4D}
    };
    static constexpr IID IID_ICoreWebView2PermissionRequestedEventHandler{
        0x15E1C6A3, 0xC72A, 0x4DF3, {0x91, 0xD7, 0xD0, 0x97, 0xFB, 0xEC, 0x6B, 0xFD}
    };
    static constexpr IID IID_ICoreWebView2WebMessageReceivedEventHandler{
        0x57213F19, 0x00E6, 0x49FA, {0x8E, 0x07, 0x89, 0x8E, 0xA0, 0x1E, 0xCB, 0xD2}
    };
    static constexpr IID IID_ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler{
        0xB99369F3, 0x9B11, 0x47B5, {0xBC, 0x6F, 0x8E, 0x78, 0x95, 0xFC, 0xEA, 0x17}
    };

#if WEBVIEW_MSWEBVIEW2_BUILTIN_IMPL == 1
    enum class webview2_runtime_type
    {
      INSTALLED = 0,
      EMBEDDED = 1
    };

    namespace webview2_symbols
    {
      using CreateWebViewEnvironmentWithOptionsInternal_t =
          HRESULT(STDMETHODCALLTYPE*)(bool, webview2_runtime_type, PCWSTR, IUnknown*, ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*);
      using DllCanUnloadNow_t = HRESULT(STDMETHODCALLTYPE*)();

      static constexpr auto CreateWebViewEnvironmentWithOptionsInternal =
          library_symbol<CreateWebViewEnvironmentWithOptionsInternal_t>("CreateWebViewEnvironmentWithOptionsInternal");
      static constexpr auto DllCanUnloadNow = library_symbol<DllCanUnloadNow_t>("DllCanUnloadNow");
    } // namespace webview2_symbols
#endif // WEBVIEW_MSWEBVIEW2_BUILTIN_IMPL

#if WEBVIEW_MSWEBVIEW2_EXPLICIT_LINK == 1
    namespace webview2_symbols
    {
      using CreateCoreWebView2EnvironmentWithOptions_t = HRESULT(STDMETHODCALLTYPE*)(PCWSTR,
                                                                                     PCWSTR,
                                                                                     ICoreWebView2EnvironmentOptions*,
                                                                                     ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*);
      using GetAvailableCoreWebView2BrowserVersionString_t = HRESULT(STDMETHODCALLTYPE*)(PCWSTR, LPWSTR*);

      static constexpr auto CreateCoreWebView2EnvironmentWithOptions =
          library_symbol<CreateCoreWebView2EnvironmentWithOptions_t>("CreateCoreWebView2EnvironmentWithOptions");
      static constexpr auto GetAvailableCoreWebView2BrowserVersionString =
          library_symbol<GetAvailableCoreWebView2BrowserVersionString_t>("GetAvailableCoreWebView2BrowserVersionString");
    } // namespace webview2_symbols
#endif // WEBVIEW_MSWEBVIEW2_EXPLICIT_LINK

    class loader
    {
  public:
      HRESULT create_environment_with_options(PCWSTR browser_dir,
                                              PCWSTR user_data_dir,
                                              ICoreWebView2EnvironmentOptions* env_options,
                                              ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* created_handler) const;

      HRESULT
      get_available_browser_version_string(const PCWSTR browser_dir, LPWSTR* version) const;

  private:
#if WEBVIEW_MSWEBVIEW2_BUILTIN_IMPL == 1
      struct client_info_t
      {
        bool found{};
        std::wstring dll_path;
        std::wstring version;
        webview2_runtime_type runtime_type{};

        client_info_t() = default;
        client_info_t(bool found, std::wstring dll_path, std::wstring version, const webview2_runtime_type runtime_type);
      };

      HRESULT create_environment_with_options_impl(PCWSTR browser_dir,
                                                   PCWSTR user_data_dir,
                                                   ICoreWebView2EnvironmentOptions* env_options,
                                                   ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* created_handler) const;

      HRESULT
      get_available_browser_version_string_impl(PCWSTR browser_dir, LPWSTR* version) const;

      client_info_t find_available_client(PCWSTR browser_dir) const;

      static std::wstring make_client_dll_path(const std::wstring& dir);

      static client_info_t find_installed_client(unsigned int min_api_version, bool system, const std::wstring& release_channel);

      static client_info_t find_embedded_client(unsigned int min_api_version, const std::wstring& dir);

      // The minimum WebView2 API version we need regardless of the SDK release
      // actually used. The number comes from the SDK release version,
      // e.g. 1.0.1150.38. To be safe the SDK should have a number that is greater
      // than or equal to this number. The Edge browser webview client must
      // have a number greater than or equal to this number.
      static constexpr unsigned int API_VERSION = 1150;

      static constexpr auto CLIENT_STATE_REG_SUB_KEY = L"SOFTWARE\\Microsoft\\EdgeUpdate\\ClientState\\";

      // GUID for the stable release channel.
      static constexpr auto STABLE_RELEASE_GUID = L"{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}";

      static constexpr auto DEFAULT_RELEASE_CHANNEL_GUID = STABLE_RELEASE_GUID;
#endif // WEBVIEW_MSWEBVIEW2_BUILTIN_IMPL

#if WEBVIEW_MSWEBVIEW2_EXPLICIT_LINK == 1
      native_library m_lib{L"WebView2Loader.dll"};
#endif
    };

    namespace cast_info
    {
      static constexpr auto controller_completed =
          cast_info_t<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>{IID_ICoreWebView2CreateCoreWebView2ControllerCompletedHandler};

      static constexpr auto environment_completed =
          cast_info_t<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>{IID_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler};

      static constexpr auto message_received = cast_info_t<ICoreWebView2WebMessageReceivedEventHandler>{IID_ICoreWebView2WebMessageReceivedEventHandler};

      static constexpr auto permission_requested = cast_info_t<ICoreWebView2PermissionRequestedEventHandler>{IID_ICoreWebView2PermissionRequestedEventHandler};

      static constexpr auto add_script_to_execute_on_document_created_completed =
          cast_info_t<ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler>{IID_ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler};
    } // namespace cast_info
  } // namespace mswebview2
} // namespace webview::detail

#endif // defined(WEBVIEW_PLATFORM_WINDOWS) && defined(WEBVIEW_EDGE)
#endif // WEBVIEW_BACKENDS_WEBVIEW2_LOADER_HPP
