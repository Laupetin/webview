#pragma once

#ifndef WEBVIEW_PLUGIN_DETAIL_FAVICON_HANDLER_LINUX_HPP
#define WEBVIEW_PLUGIN_DETAIL_FAVICON_HANDLER_LINUX_HPP

#include "../../../plugin/title_handler.hpp"
#include "../../macros.hpp"

#if defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)
#ifdef WEBVIEW_INCLUDE_IMPL

namespace webview
{
  WEBVIEW_IMPL std::expected<void, std::string> favicon_handler_plugin::on_setup_window(window& window, const plugin_window_context& context)
  {
    // The icon system on Linux works a bit different from on Windows
    // and doesn't really support this kind of dynamic icon setting
    // we skip it for now

    return {};
  }
} // namespace webview

#endif
#endif
#endif
