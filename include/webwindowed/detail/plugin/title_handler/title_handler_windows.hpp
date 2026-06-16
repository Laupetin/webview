#pragma once

#ifndef WEBWINDOWED_PLUGIN_DETAIL_TITLE_HANDLER_WINDOWS_HPP
#define WEBWINDOWED_PLUGIN_DETAIL_TITLE_HANDLER_WINDOWS_HPP

#include "../../../plugin/title_handler.hpp"
#include "../../macros.hpp"

#if defined(WEBWINDOWED_PLATFORM_WINDOWS) && defined(WEBWINDOWED_EDGE)
#ifdef WEBWINDOWED_INCLUDE_IMPL

#include "../../platform/windows/webview2/loader.hpp"

#include <Windows.h>
#include <wrl/event.h>

namespace webwindowed
{
  WEBWINDOWED_IMPL std::expected<void, std::string> title_handler_plugin::on_setup_window(window& window, const plugin_window_context& context)
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

    EventRegistrationToken token;
    if (FAILED(core->add_DocumentTitleChanged(Microsoft::WRL::Callback<ICoreWebView2DocumentTitleChangedEventHandler>(
                                                  [window_hwnd](ICoreWebView2* sender, IUnknown* args) -> HRESULT
                                                  {
                                                    LPWSTR title;

                                                    if (FAILED(sender->get_DocumentTitle(&title)))
                                                      return S_FALSE;

                                                    SetWindowTextW(window_hwnd, title);
                                                    CoTaskMemFree(title);

                                                    return S_OK;
                                                  })
                                                  .Get(),
                                              &token)))
    {
      return std::unexpected("Failed to add title handler");
    }

    return {};
  }
} // namespace webwindowed

#endif
#endif
#endif
