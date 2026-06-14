#pragma once

#ifndef WEBVIEW_PLUGIN_TITLE_HANDLER_HPP
#define WEBVIEW_PLUGIN_TITLE_HANDLER_HPP

#include "../detail/plugin.hpp"

#include <expected>
#include <string>

namespace webview
{
  class title_handler_plugin : public plugin
  {
public:
    std::expected<void, std::string> on_setup_window(window& window, const plugin_window_context& context) override;
  };
} // namespace webview

#include "detail/title_handler_linux.hpp"
#include "detail/title_handler_windows.hpp"

#endif
