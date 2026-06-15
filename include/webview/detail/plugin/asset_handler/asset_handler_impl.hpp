#pragma once

#ifndef WEBVIEW_PLUGIN_DETAIL_ASSET_HANDLER_IMPL_HPP
#define WEBVIEW_PLUGIN_DETAIL_ASSET_HANDLER_IMPL_HPP

#include "../../../plugin/asset_handler.hpp"
#include "../../macros.hpp"

#include <format>
#include <string>

#ifdef WEBVIEW_INCLUDE_IMPL

namespace webview
{
  WEBVIEW_IMPL asset_handler_plugin::asset_handler_plugin()
      : m_protocol_name("webview")
  {
  }

  WEBVIEW_IMPL asset_handler_plugin::asset_handler_plugin(const asset* assets, const size_t asset_count)
      : asset_handler_plugin()
  {
    register_assets(assets, asset_count);
  }

  WEBVIEW_IMPL const std::string& asset_handler_plugin::get_protocol_name() const
  {
    return m_protocol_name;
  }

  WEBVIEW_IMPL void asset_handler_plugin::set_protocol_name(std::string protocol_name)
  {
    m_protocol_name = std::move(protocol_name);
  }

  WEBVIEW_IMPL void asset_handler_plugin::register_assets(const asset* assets, const size_t asset_count)
  {
    for (auto asset_index = 0u; asset_index < asset_count; ++asset_index)
    {
      const auto& asset = assets[asset_index];

      m_asset_lookup.emplace(asset.filename, asset);
    }
  }

  WEBVIEW_IMPL std::string asset_handler_plugin::get_url_for_asset(const std::string& asset_name)
  {
    return std::format("{}://localhost/{}", m_protocol_name, asset_name);
  }

  WEBVIEW_IMPL const char* get_mime_type_for_file_name(const std::string& fileName)
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

  WEBVIEW_IMPL void split_asset_request_uri(const std::string& uri, const std::string& protocol_name, std::string& out_hostname, std::string& out_asset)
  {
    auto trim_start_count = protocol_name.size() + std::char_traits<char>::length("://");
    auto trim_end_count = 0;

    auto hostname_end = uri.find_first_of('/', trim_start_count);
    if (hostname_end == std::string::npos)
      hostname_end = uri.size();
    out_hostname = uri.substr(trim_start_count, hostname_end - trim_start_count);

    trim_start_count = hostname_end;
    if (uri[trim_start_count] == '/')
      trim_start_count++;

    out_asset = uri.substr(trim_start_count, uri.size() - trim_end_count - trim_start_count);
  }
} // namespace webview

#endif
#endif
