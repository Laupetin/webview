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

    /**
     * \brief Exposes a hook for setting additional properties on the window and browser widget.
     * \param window The window that this hook is being called on.
     * \param context The context that this window was set up with.
     */
    virtual std::expected<void, std::string> on_setup_window(window& window, const plugin_window_context& context)
    {
      return {};
    }

    /**
     * \brief A Windows webview2 hook that exposes an object for setting environment options.
     * \param window The window that this hook is being called on.
     * \param environment_options An `ICoreWebView2EnvironmentOptions` interface pointer.
     */
    virtual std::expected<void, std::string> on_setup_environment_options(window& window, void* environment_options)
    {
      return {};
    }
  };
} // namespace webview

#endif
