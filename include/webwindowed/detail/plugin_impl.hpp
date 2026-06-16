#pragma once

#ifndef WEBWINDOWED_DETAIL_PLUGIN_IMPL_HPP
#define WEBWINDOWED_DETAIL_PLUGIN_IMPL_HPP

#include "macros.hpp"
#include "plugin.hpp"

namespace webwindowed
{
  WEBWINDOWED_IMPL plugin_window_context::plugin_window_context(void* window, void* widget, void* browser_controller)
      : m_window(window),
        m_widget(widget),
        m_browser_controller(browser_controller)
  {
  }

  WEBWINDOWED_IMPL std::optional<void*> plugin_window_context::window() const
  {
    return m_window;
  }

  WEBWINDOWED_IMPL std::optional<void*> plugin_window_context::widget() const
  {
    return m_widget;
  }

  WEBWINDOWED_IMPL std::optional<void*> plugin_window_context::browser_controller() const
  {
    return m_browser_controller;
  }
} // namespace webwindowed

#endif
