#pragma once

#ifndef WEBVIEW_PLUGIN_ASSET_HANDLER_HPP
#define WEBVIEW_PLUGIN_ASSET_HANDLER_HPP

#include "../detail/plugin.hpp"

#include <expected>
#include <string>
#include <unordered_map>

namespace webview
{
  struct asset
  {
    const char* filename;
    const void* data;
    const size_t data_size;
  };

  class asset_handler_plugin : public plugin
  {
public:
    asset_handler_plugin();
    asset_handler_plugin(const asset* assets, size_t asset_count);

    [[nodiscard]] const std::string& get_protocol_name() const;
    void set_protocol_name(std::string protocol_name);
    void register_assets(const asset* assets, size_t asset_count);

    std::string get_url_for_asset(const std::string& asset_name);

    std::expected<void, std::string> on_setup_window(window& window, const plugin_window_context& context) override;
    std::expected<void, std::string> on_setup_environment_options(window& window, void* environment_options) override;

private:
    std::string m_protocol_name;
    std::unordered_map<std::string, asset> m_asset_lookup;
  };
} // namespace webview

#include "../detail/plugin/asset_handler/asset_handler_impl.hpp"
#include "../detail/plugin/asset_handler/asset_handler_linux.hpp"
#include "../detail/plugin/asset_handler/asset_handler_windows.hpp"

#endif
