#pragma once

#ifndef WEBVIEW_PLUGIN_DETAIL_ASSET_HANDLER_LINUX_HPP
#define WEBVIEW_PLUGIN_DETAIL_ASSET_HANDLER_LINUX_HPP

#include "../../../plugin/asset_handler.hpp"
#include "../../macros.hpp"
#include "asset_handler_impl.hpp"

#if defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)
#ifdef WEBVIEW_INCLUDE_IMPL

#include <iostream>

namespace webview
{
  WEBVIEW_IMPL std::expected<void, std::string> asset_handler_plugin::on_setup_window(window& window, const plugin_window_context& context)
  {
    const auto maybe_widget = context.browser_controller();
    if (!maybe_widget.has_value())
      return std::unexpected("Failed to get browser controller");
    const auto widget = static_cast<GtkWidget*>(maybe_widget.value());
    const auto web_view = WEBKIT_WEB_VIEW(widget);
    const auto web_view_context = webkit_web_view_get_context(web_view);

    const auto cb = +[](WebKitURISchemeRequest* request, gpointer user_data)
    {
      auto* self = static_cast<asset_handler_plugin*>(user_data);
      const gchar* uri = webkit_uri_scheme_request_get_uri(request);

      std::string hostname;
      std::string asset;
      split_asset_request_uri(uri, self->m_protocol_name, hostname, asset);

      const auto foundUiFile = self->m_asset_lookup.find(asset);
      if (foundUiFile != self->m_asset_lookup.end())
      {
        gsize stream_length = foundUiFile->second.data_size;
        GInputStream* stream = g_memory_input_stream_new_from_data(foundUiFile->second.data, foundUiFile->second.data_size, nullptr);

        webkit_uri_scheme_request_finish(request, stream, stream_length, get_mime_type_for_file_name(foundUiFile->second.filename));
        g_object_unref(stream);
      }
      else
      {
        GError* error = g_error_new(G_SPAWN_ERROR, 123, "Could not find %s.", asset.c_str());
        webkit_uri_scheme_request_finish_error(request, error);
        g_error_free(error);
        return;
      }
    };

    webkit_web_context_register_uri_scheme(web_view_context, m_protocol_name.c_str(), cb, this, nullptr);

    return {};
  }

  WEBVIEW_IMPL std::expected<void, std::string> asset_handler_plugin::on_setup_environment_options(window& window, void* environment_options)
  {
    return {};
  }
} // namespace webview

#endif
#endif
#endif
