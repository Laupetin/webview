#pragma once

#ifndef WEBWINDOWED_PLUGIN_DETAIL_FAVICON_HANDLER_WINDOWS_HPP
#define WEBWINDOWED_PLUGIN_DETAIL_FAVICON_HANDLER_WINDOWS_HPP

#include "../../../plugin/favicon_handler.hpp"
#include "../../macros.hpp"

#if defined(WEBWINDOWED_PLATFORM_WINDOWS) && defined(WEBWINDOWED_EDGE)
#ifdef WEBWINDOWED_INCLUDE_IMPL

#include "../../platform/windows/webview2/loader.hpp"

#include <Windows.h>
#include <gdiplus.h>
#include <wrl/event.h>

#ifdef _MSC_VER
#pragma comment(lib, "gdiplus.lib")
#endif

namespace webwindowed
{
  namespace detail
  {
    WEBWINDOWED_IMPL unique_favicon::unique_favicon(void* handle)
        : m_handle(handle)
    {
    }

    WEBWINDOWED_IMPL unique_favicon::~unique_favicon()
    {
      if (m_handle)
      {
        DestroyIcon(static_cast<HICON>(m_handle));
        m_handle = nullptr;
      }
    }

    WEBWINDOWED_IMPL unique_favicon::unique_favicon(unique_favicon&& other) noexcept
        : m_handle(other.m_handle)
    {
      other.m_handle = nullptr;
    }

    WEBWINDOWED_IMPL unique_favicon& unique_favicon::operator=(unique_favicon&& other) noexcept
    {
      m_handle = other.m_handle;
      other.m_handle = nullptr;
      return *this;
    }
  } // namespace detail

  WEBWINDOWED_IMPL std::expected<void, std::string> favicon_handler_plugin::on_setup_window(window& window, const plugin_window_context& context)
  {
    const auto maybe_browser_controller = context.browser_controller();
    if (!maybe_browser_controller.has_value())
      return std::unexpected("Failed to get browser controller");
    const auto controller = static_cast<ICoreWebView2Controller*>(maybe_browser_controller.value());

    const auto maybe_window = context.window();
    if (!maybe_window.has_value())
      return std::unexpected("Failed to get window");
    auto window_hwnd = static_cast<HWND>(maybe_window.value());

    Microsoft::WRL::ComPtr<ICoreWebView2> core;
    if (FAILED(controller->get_CoreWebView2(&core)))
      return std::unexpected("Failed to get webview");

    Microsoft::WRL::ComPtr<ICoreWebView2_15> core15;
    if (FAILED(core->QueryInterface(IID_PPV_ARGS(&core15))))
      return std::unexpected("Failed to get core15");

    const Gdiplus::GdiplusStartupInput gdi_plus_startup_input;
    ULONG_PTR gdiPlusToken;
    Gdiplus::GdiplusStartup(&gdiPlusToken, &gdi_plus_startup_input, nullptr);
    EventRegistrationToken token;
    if (FAILED(core15->add_FaviconChanged(Microsoft::WRL::Callback<ICoreWebView2FaviconChangedEventHandler>(
                                              [this, core15, window_hwnd](ICoreWebView2* sender, IUnknown* args) -> HRESULT
                                              {
                                                LPWSTR url;

                                                if (FAILED(core15->get_FaviconUri(&url)))
                                                {
                                                  return S_FALSE;
                                                }

                                                const std::wstring strUrl(url);
                                                CoTaskMemFree(url);

                                                if (strUrl.empty())
                                                {
                                                  m_icon_per_window.erase(m_icon_per_window.find(window_hwnd));
                                                  SendMessage(window_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)NULL);
                                                }
                                                else
                                                {
                                                  if (FAILED(core15->GetFavicon(
                                                          COREWEBVIEW2_FAVICON_IMAGE_FORMAT_PNG,
                                                          Microsoft::WRL::Callback<ICoreWebView2GetFaviconCompletedHandler>(
                                                              [this, window_hwnd](HRESULT errorCode, IStream* iconStream) -> HRESULT
                                                              {
                                                                Gdiplus::Bitmap iconBitmap(iconStream);
                                                                HICON icon_handle;
                                                                if (iconBitmap.GetHICON(&icon_handle) == Gdiplus::Status::Ok)
                                                                {
                                                                  SendMessage(window_hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(icon_handle));
                                                                  m_icon_per_window.emplace(window_hwnd, icon_handle);
                                                                }
                                                                else
                                                                {
                                                                  m_icon_per_window.erase(m_icon_per_window.find(window_hwnd));
                                                                  SendMessage(window_hwnd, WM_SETICON, ICON_SMALL, NULL);
                                                                }

                                                                return S_OK;
                                                              })
                                                              .Get())))
                                                  {
                                                    return S_FALSE;
                                                  }
                                                }

                                                return S_OK;
                                              })
                                              .Get(),
                                          &token)))
    {
      return std::unexpected("Failed to add favicon handler");
    }

    return {};
  }
} // namespace webwindowed

#endif
#endif
#endif
