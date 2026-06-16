#pragma once

#ifndef WEBWINDOWED_PLUGIN_FAVICON_HANDLER_HPP
#define WEBWINDOWED_PLUGIN_FAVICON_HANDLER_HPP

#include "../detail/plugin.hpp"

#include <expected>
#include <string>
#include <unordered_map>

namespace webwindowed
{
  namespace detail
  {
#if defined(WEBWINDOWED_PLATFORM_WINDOWS) && defined(WEBWINDOWED_EDGE)
    class unique_favicon
    {
  public:
      explicit unique_favicon(void* handle);
      ~unique_favicon();
      unique_favicon(const unique_favicon& other) = delete;
      unique_favicon(unique_favicon&& other) noexcept;
      unique_favicon& operator=(const unique_favicon& other) = delete;
      unique_favicon& operator=(unique_favicon&& other) noexcept;

  private:
      void* m_handle;
    };
#endif
  } // namespace detail

  class favicon_handler_plugin : public plugin
  {
public:
    std::expected<void, std::string> on_setup_window(window& window, const plugin_window_context& context) override;

#if defined(WEBWINDOWED_PLATFORM_WINDOWS) && defined(WEBWINDOWED_EDGE)
private:
    std::unordered_map<void*, detail::unique_favicon> m_icon_per_window;
#endif
  };
} // namespace webwindowed

#include "../detail/plugin/favicon_handler/favicon_handler_linux.hpp"
#include "../detail/plugin/favicon_handler/favicon_handler_windows.hpp"

#endif
