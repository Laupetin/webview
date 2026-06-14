#pragma once

#ifndef WEBVIEW_PLUGIN_DETAIL_TITLE_HANDLER_LINUX_HPP
#define WEBVIEW_PLUGIN_DETAIL_TITLE_HANDLER_LINUX_HPP

#include "../../../plugin/title_handler.hpp"
#include "../../macros.hpp"

#if defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)
#ifdef WEBVIEW_INCLUDE_IMPL

#include <gtk/gtk.h>

namespace webview
{
  WEBVIEW_IMPL std::expected<void, std::string> title_handler_plugin::on_setup_window(window& window, const plugin_window_context& context)
  {
    const auto maybe_browser_controller = context.browser_controller();
    if (!maybe_browser_controller.has_value())
      return std::unexpected("Failed to get browser controller");
    const auto web_view_widget = static_cast<GtkWidget*>(maybe_browser_controller.value());
    const auto web_view = WEBKIT_WEB_VIEW(web_view_widget);

    const auto maybe_window = context.window();
    if (!maybe_window.has_value())
      return std::unexpected("Failed to get window");
    const auto window_widget = static_cast<GtkWidget*>(maybe_window.value());

    auto on_title_changed = +[](GObject* widget, GParamSpec* param_spec, GtkWindow* gtk_window)
    {
      gtk_window_set_title(gtk_window, webkit_web_view_get_title(WEBKIT_WEB_VIEW(widget)));
    };

    g_signal_connect(G_OBJECT(web_view), "notify::title", G_CALLBACK(on_title_changed), GTK_WINDOW(window_widget));

    return {};
  }
} // namespace webview

#endif
#endif
#endif
