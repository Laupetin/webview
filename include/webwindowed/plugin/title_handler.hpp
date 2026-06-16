#pragma once

#ifndef WEBWINDOWED_PLUGIN_TITLE_HANDLER_HPP
#define WEBWINDOWED_PLUGIN_TITLE_HANDLER_HPP

#include "../detail/plugin.hpp"

#include <expected>
#include <string>

namespace webwindowed
{
  class title_handler_plugin : public plugin
  {
public:
    std::expected<void, std::string> on_setup_window(window& window, const plugin_window_context& context) override;
  };
} // namespace webwindowed

#include "../detail/plugin/title_handler/title_handler_linux.hpp"
#include "../detail/plugin/title_handler/title_handler_windows.hpp"

#endif
