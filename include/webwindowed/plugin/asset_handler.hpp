#pragma once

#ifndef WEBWINDOWED_PLUGIN_ASSET_HANDLER_HPP
#define WEBWINDOWED_PLUGIN_ASSET_HANDLER_HPP

#include "../detail/plugin.hpp"

#include <expected>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace webwindowed
{
  class static_asset
  {
public:
    static_asset(std::string file_name, const void* data, size_t data_size);
    static_asset(std::string file_name, std::vector<char> data_buffer);

    [[nodiscard]] const std::string& get_file_name() const;
    void get_data(const void*& data, size_t& data_size) const;

private:
    std::string m_file_name;
    const void* m_data;
    size_t m_data_size;
    std::vector<char> m_data_buffer;
  };

  class dynamic_asset_request
  {
public:
    explicit dynamic_asset_request(std::string query);

    std::optional<std::string> get_query(const std::string&) const;

private:
    std::string m_query;
  };

  class dynamic_asset_response
  {
public:
    using cb_t = std::function<void(int code, const void* data, size_t data_size, const std::string& content_type)>;

    explicit dynamic_asset_response(cb_t callback);

    void set_content_type(std::string content_type);
    void send_response(int code) const;
    void send_response(const void* data, size_t data_size) const;
    void send_response(int code, const void* data, size_t data_size) const;

private:
    std::string m_content_type;
    cb_t m_callback;
  };

  class dynamic_asset
  {
public:
    using cb_t = std::function<void(const dynamic_asset_request& request, dynamic_asset_response& response)>;

    dynamic_asset(std::string file_name, cb_t callback);

    [[nodiscard]] const std::string& get_file_name() const;
    void call(const dynamic_asset_request& request, dynamic_asset_response& response) const;

private:
    std::string m_file_name;
    cb_t m_callback;
  };

  class asset_handler_plugin : public plugin
  {
public:
    asset_handler_plugin();

    [[nodiscard]] const std::string& get_protocol_name() const;
    void set_protocol_name(std::string protocol_name);

    void add_static_asset(static_asset static_asset);
    void add_dynamic_asset(dynamic_asset dynamic_asset);

    std::string get_url_for_asset(const std::string& asset_name);

    std::expected<void, std::string> on_setup_window(window& window, const plugin_window_context& context) override;
    std::expected<void, std::string> on_setup_environment_options(window& window, void* environment_options) override;

private:
    std::string m_protocol_name;
    std::unordered_map<std::string, static_asset> m_static_asset_lookup;
    std::unordered_map<std::string, dynamic_asset> m_dynamic_asset_lookup;
  };
} // namespace webwindowed

#include "../detail/plugin/asset_handler/asset_handler_impl.hpp"
#include "../detail/plugin/asset_handler/asset_handler_linux.hpp"
#include "../detail/plugin/asset_handler/asset_handler_windows.hpp"

#endif
