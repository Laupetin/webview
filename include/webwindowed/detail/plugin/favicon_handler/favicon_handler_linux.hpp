#pragma once

#ifndef WEBWINDOWED_PLUGIN_DETAIL_FAVICON_HANDLER_LINUX_HPP
#define WEBWINDOWED_PLUGIN_DETAIL_FAVICON_HANDLER_LINUX_HPP

#include "../../../plugin/title_handler.hpp"
#include "../../macros.hpp"

#if defined(WEBWINDOWED_PLATFORM_LINUX) && defined(WEBWINDOWED_GTK)
#ifdef WEBWINDOWED_INCLUDE_IMPL

namespace webwindowed
{
  WEBWINDOWED_IMPL std::expected<void, std::string> favicon_handler_plugin::on_setup_window(window& window, const plugin_window_context& context)
  {
    // The icon system on Linux works a bit different from on Windows
    // and doesn't really support this kind of dynamic icon setting
    // we skip it for now

    return {};
  }
} // namespace webwindowed

#endif
#endif
#endif
