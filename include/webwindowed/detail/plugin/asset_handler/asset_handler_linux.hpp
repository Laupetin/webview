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
          auto* webkit_response = webkit_uri_scheme_response_new(stream, static_cast<gint64>(data_size));
          webkit_uri_scheme_response_set_status(webkit_response, 200, "OK");

          const auto content_type = get_mime_type_for_file_name(found_static_file->second.get_file_name());
          webkit_uri_scheme_response_set_content_type(webkit_response, content_type);

          auto* soup_headers = soup_message_headers_new(SOUP_MESSAGE_HEADERS_RESPONSE);
          soup_message_headers_replace(soup_headers, "Content-Type", content_type);

          if (self->m_allow_all_origins)
            soup_message_headers_replace(soup_headers, "Access-Control-Allow-Origin", "*");

          webkit_uri_scheme_response_set_http_headers(webkit_response, soup_headers);
          webkit_uri_scheme_request_finish_with_response(request, webkit_response);

          g_object_unref(webkit_response);
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
                  auto data_copy = new uint8_t[data_size];
                  memcpy(data_copy, data, data_size);
                  auto* stream = g_memory_input_stream_new_from_data(
                      data_copy,
                      static_cast<gssize>(data_size),
                      +[](gpointer data)
                      {
                        delete[] static_cast<uint8_t*>(data);
                      });

                  auto* webkit_response = webkit_uri_scheme_response_new(stream, static_cast<gint64>(data_size));
                  webkit_uri_scheme_response_set_status(webkit_response, static_cast<guint>(code), "OK");

                  auto* soup_headers = soup_message_headers_new(SOUP_MESSAGE_HEADERS_RESPONSE);
                  if (content_type.empty())
                  {
                    const auto generated_content_type = get_mime_type_for_file_name(found_dynamic_file->second.get_file_name());
                    webkit_uri_scheme_response_set_content_type(webkit_response, generated_content_type);
                    soup_message_headers_replace(soup_headers, "Content-Type", generated_content_type);
                  }
                  else
                  {
                    webkit_uri_scheme_response_set_content_type(webkit_response, content_type.c_str());
                    soup_message_headers_replace(soup_headers, "Content-Type", content_type.c_str());
                  }

                  if (self->m_allow_all_origins)
                    soup_message_headers_replace(soup_headers, "Access-Control-Allow-Origin", "*");

                  webkit_uri_scheme_response_set_http_headers(webkit_response, soup_headers);
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
        auto* stream = g_memory_input_stream_new();
        auto* webkit_response = webkit_uri_scheme_response_new(stream, 0);
        webkit_uri_scheme_response_set_status(webkit_response, 404, "Not found");

        auto* soup_headers = soup_message_headers_new(SOUP_MESSAGE_HEADERS_RESPONSE);
        if (self->m_allow_all_origins)
          soup_message_headers_replace(soup_headers, "Access-Control-Allow-Origin", "*");

        webkit_uri_scheme_response_set_http_headers(webkit_response, soup_headers);
        webkit_uri_scheme_request_finish_with_response(request, webkit_response);

        g_object_unref(webkit_response);
        g_object_unref(stream);
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
