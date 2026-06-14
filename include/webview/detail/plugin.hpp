#pragma once

#ifndef WEBVIEW_DETAIL_PLUGIN_HPP
#define WEBVIEW_DETAIL_PLUGIN_HPP

#include "window.hpp"

#include <optional>

namespace webview
{
  class plugin_window_context
  {
public:
    plugin_window_context(void* window, void* widget, void* browser_controller);

    std::optional<void*> window() const;
    std::optional<void*> widget() const;
    std::optional<void*> browser_controller() const;

private:
    void* m_window;
    void* m_widget;
    void* m_browser_controller;
  };

  class plugin
  {
public:
    plugin() = default;
    virtual ~plugin() = default;

    virtual std::expected<void, std::string> on_setup_window(window& window, const plugin_window_context& context)
    {
      return {};
    };
  };
} // namespace webview

#endif
