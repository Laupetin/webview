#pragma once

#ifndef WEBWINDOWED_PLUGIN_DETAIL_ASSET_HANDLER_IMPL_HPP
#define WEBWINDOWED_PLUGIN_DETAIL_ASSET_HANDLER_IMPL_HPP

#include "../../../plugin/asset_handler.hpp"
#include "../../macros.hpp"

#include <format>
#include <string>

#ifdef WEBWINDOWED_INCLUDE_IMPL

namespace webwindowed
{
  WEBWINDOWED_IMPL static_asset::static_asset(std::string file_name, const void* data, const size_t data_size)
      : m_file_name(std::move(file_name)),
        m_data(data),
        m_data_size(data_size)
  {
  }

  WEBWINDOWED_IMPL static_asset::static_asset(std::string file_name, std::vector<char> data_buffer)
      : m_file_name(std::move(file_name)),
        m_data_buffer(std::move(data_buffer))
  {
    m_data = m_data_buffer.data();
    m_data_size = m_data_buffer.size();
  }

  WEBWINDOWED_IMPL const std::string& static_asset::get_file_name() const
  {
    return m_file_name;
  }

  WEBWINDOWED_IMPL void static_asset::get_data(const void*& data, size_t& data_size) const
  {
    data = m_data;
    data_size = m_data_size;
  }

  WEBWINDOWED_IMPL dynamic_asset_request::dynamic_asset_request(std::string query)
      : m_query(std::move(query))
  {
  }

  WEBWINDOWED_IMPL std::optional<std::string> dynamic_asset_request::get_query(const std::string& query_name) const
  {
    size_t current_pos = 0;
    while (current_pos < m_query.size())
    {
      const auto next_value = m_query.find_first_of("=&", current_pos);

      std::string key;
      if (next_value == std::string::npos)
      {
        key = std::string(m_query, current_pos, m_query.size() - current_pos);
        if (key == query_name)
          return "";

        current_pos = m_query.size();
      }
      else
      {
        key = std::string(m_query, current_pos, next_value - current_pos);

        if (m_query[next_value] == '&')
        {
          if (key == query_name)
            return "";
          current_pos = next_value + 1;
        }
        else
        {
          // m_query[next_value] == '='
          auto next_value_end = m_query.find_first_of("&", next_value + 1);
          if (next_value_end == std::string::npos)
            next_value_end = next_value_end = m_query.size();

          if (key == query_name)
            return std::string(m_query, next_value + 1, next_value_end - next_value - 1);
          current_pos = next_value_end + 1;
        }
      }
    }

    return std::nullopt;
  }

  WEBWINDOWED_IMPL dynamic_asset_response::dynamic_asset_response(cb_t callback)
      : m_callback(std::move(callback))
  {
  }

  WEBWINDOWED_IMPL void dynamic_asset_response::set_content_type(std::string content_type)
  {
    m_content_type = std::move(content_type);
  }

  WEBWINDOWED_IMPL void dynamic_asset_response::send_response(const int code) const
  {
    send_response(code, nullptr, 0);
  }

  WEBWINDOWED_IMPL void dynamic_asset_response::send_response(const void* data, const size_t data_size) const
  {
    send_response(200, data, data_size);
  }

  WEBWINDOWED_IMPL void dynamic_asset_response::send_response(const int code, const void* data, const size_t data_size) const
  {
    m_callback(code, data, data_size, m_content_type);
  }

  WEBWINDOWED_IMPL dynamic_asset::dynamic_asset(std::string file_name, cb_t callback)
      : m_file_name(std::move(file_name)),
        m_callback(std::move(callback))
  {
  }

  WEBWINDOWED_IMPL const std::string& dynamic_asset::get_file_name() const
  {
    return m_file_name;
  }

  WEBWINDOWED_IMPL void dynamic_asset::call(const dynamic_asset_request& request, dynamic_asset_response& response) const
  {
    m_callback(request, response);
  }

  WEBWINDOWED_IMPL asset_handler_plugin::asset_handler_plugin()
      : m_protocol_name("webwindowed"),
        m_allow_all_origins(false)
  {
  }

  WEBWINDOWED_IMPL const std::string& asset_handler_plugin::get_protocol_name() const
  {
    return m_protocol_name;
  }

  WEBWINDOWED_IMPL void asset_handler_plugin::set_allow_all_origins(bool allow_all_origins)
  {
    m_allow_all_origins = allow_all_origins;
  }

  WEBWINDOWED_IMPL void asset_handler_plugin::set_protocol_name(std::string protocol_name)
  {
    m_protocol_name = std::move(protocol_name);
  }

  WEBWINDOWED_IMPL void asset_handler_plugin::add_static_asset(static_asset static_asset)
  {
    m_static_asset_lookup.emplace(static_asset.get_file_name(), std::move(static_asset));
  }

  WEBWINDOWED_IMPL void asset_handler_plugin::add_dynamic_asset(dynamic_asset dynamic_asset)
  {
    m_dynamic_asset_lookup.emplace(dynamic_asset.get_file_name(), std::move(dynamic_asset));
  }

  WEBWINDOWED_IMPL std::string asset_handler_plugin::get_url_for_asset(const std::string& asset_name)
  {
    return std::format("{}://localhost/{}", m_protocol_name, asset_name);
  }

  WEBWINDOWED_IMPL const char* get_mime_type_for_file_name(const std::string& fileName)
  {
    const char* mime_type;

    if (fileName.ends_with(".html"))
      mime_type = "text/html";
    else if (fileName.ends_with(".js"))
      mime_type = "text/javascript";
    else if (fileName.ends_with(".css"))
      mime_type = "text/css";
    else
      mime_type = "application/octet-stream";

    return mime_type;
  }

  WEBWINDOWED_IMPL void split_asset_request_uri(
      const std::string& uri, const std::string& protocol_name, std::string& out_hostname, std::string& out_asset, std::string& out_query)
  {
    size_t trim_start_count = protocol_name.size() + std::char_traits<char>::length("://");
    size_t trim_end_count = 0u;

    auto hostname_end = uri.find_first_of('/', trim_start_count);
    if (hostname_end == std::string::npos)
      hostname_end = uri.size();
    out_hostname = uri.substr(trim_start_count, hostname_end - trim_start_count);

    trim_start_count = hostname_end;
    if (uri[trim_start_count] == '/')
      trim_start_count++;

    const auto anchor_start = uri.find_first_of('#', trim_start_count);
    if (anchor_start != std::string::npos)
      trim_end_count = std::max<size_t>(uri.size() - anchor_start, trim_end_count);
    const auto query_start = uri.find_first_of('?', trim_start_count);
    if (query_start != std::string::npos)
    {
      trim_end_count = std::max<size_t>(uri.size() - query_start, trim_end_count);

      const auto query_end = anchor_start != std::string::npos && anchor_start > query_start ? anchor_start : uri.size();
      out_query = uri.substr(query_start + 1, query_end - query_start - 1);
    }

    out_asset = uri.substr(trim_start_count, uri.size() - trim_end_count - trim_start_count);
  }
} // namespace webwindowed

#endif
#endif
