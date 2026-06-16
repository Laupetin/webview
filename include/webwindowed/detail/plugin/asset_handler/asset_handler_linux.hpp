#pragma once

#ifndef WEBWINDOWED_PLUGIN_DETAIL_ASSET_HANDLER_LINUX_HPP
#define WEBWINDOWED_PLUGIN_DETAIL_ASSET_HANDLER_LINUX_HPP

#include "../../../plugin/asset_handler.hpp"
#include "../../macros.hpp"
#include "asset_handler_impl.hpp"

#if defined(WEBWINDOWED_PLATFORM_LINUX) && defined(WEBWINDOWED_GTK)
#ifdef WEBWINDOWED_INCLUDE_IMPL

#include <iostream>

namespace webwindowed
{
  WEBWINDOWED_IMPL std::expected<void, std::string> asset_handler_plugin::on_setup_window(window& window, const plugin_window_context& context)
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

      bool file_found = false;

      std::string hostname;
      std::string asset;
      std::string query;
      split_asset_request_uri(uri, self->m_protocol_name, hostname, asset, query);

      if (hostname == "localhost")
      {
        const auto found_static_file = self->m_static_asset_lookup.find(asset);
        if (found_static_file != self->m_static_asset_lookup.end())
        {
          const void* data;
          size_t data_size;
          found_static_file->second.get_data(data, data_size);

          auto* stream = g_memory_input_stream_new_from_data(data, static_cast<gssize>(data_size), nullptr);

          webkit_uri_scheme_request_finish(
              request, stream, static_cast<gint64>(data_size), get_mime_type_for_file_name(found_static_file->second.get_file_name()));
          g_object_unref(stream);

          file_found = true;
        }

        if (!file_found)
        {
          const auto found_dynamic_file = self->m_dynamic_asset_lookup.find(asset);
          if (found_dynamic_file != self->m_dynamic_asset_lookup.end())
          {
            dynamic_asset_request dyn_request(std::move(query));
            dynamic_asset_response dyn_response(
                [&](const int code, const void* data, const size_t data_size, const std::string& content_type)
                {
                  auto* stream = g_memory_input_stream_new_from_data(data, static_cast<gssize>(data_size), nullptr);

                  auto* webkit_response = webkit_uri_scheme_response_new(stream, static_cast<gint64>(data_size));
                  webkit_uri_scheme_response_set_status(webkit_response, static_cast<guint>(code), "OK");

                  if (content_type.empty())
                    webkit_uri_scheme_response_set_content_type(webkit_response, get_mime_type_for_file_name(found_dynamic_file->second.get_file_name()));
                  else
                    webkit_uri_scheme_response_set_content_type(webkit_response, content_type.c_str());

                  webkit_uri_scheme_request_finish_with_response(request, webkit_response);

                  g_object_unref(webkit_response);
                  g_object_unref(stream);

                  file_found = true;
                });

            found_dynamic_file->second.call(dyn_request, dyn_response);
          }
        }
      }

      if (!file_found)
      {
        GError* error = g_error_new(G_SPAWN_ERROR, 123, "Could not find %s.", asset.c_str());
        webkit_uri_scheme_request_finish_error(request, error);
        g_error_free(error);
      }
    };

    webkit_web_context_register_uri_scheme(web_view_context, m_protocol_name.c_str(), cb, this, nullptr);

    return {};
  }

  WEBWINDOWED_IMPL std::expected<void, std::string> asset_handler_plugin::on_setup_environment_options(window& window, void* environment_options)
  {
    return {};
  }
} // namespace webwindowed

#endif
#endif
#endif
