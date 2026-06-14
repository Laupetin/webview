#pragma once

#ifndef WEBVIEW_DETAIL_PLUGIN_IMPL_HPP
#define WEBVIEW_DETAIL_PLUGIN_IMPL_HPP

#include "macros.hpp"
#include "plugin.hpp"

namespace webview
{
  WEBVIEW_IMPL plugin_window_context::plugin_window_context(void* window, void* widget, void* browser_controller)
      : m_window(window),
        m_widget(widget),
        m_browser_controller(browser_controller)
  {
  }

  WEBVIEW_IMPL std::optional<void*> plugin_window_context::window() const
  {
    return m_window;
  }

  WEBVIEW_IMPL std::optional<void*> plugin_window_context::widget() const
  {
    return m_widget;
  }

  WEBVIEW_IMPL std::optional<void*> plugin_window_context::browser_controller() const
  {
    return m_browser_controller;
  }
} // namespace webview

#endif
