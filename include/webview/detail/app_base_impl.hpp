#pragma once

#ifndef WEBVIEW_DETAIL_APP_BASE_IMPL_HPP
#define WEBVIEW_DETAIL_APP_BASE_IMPL_HPP

#include "app_base.hpp"
#include "macros.hpp"

#include <algorithm>

namespace webview::detail
{
  WEBVIEW_IMPL app_base::app_base()
      : m_stop_run_loop(true),
        m_shutdown_behaviour(app_shutdown_behaviour::on_main_window_closed)
  {
  }

  WEBVIEW_IMPL void app_base::set_shutdown_behaviour(const app_shutdown_behaviour behaviour)
  {
    m_shutdown_behaviour = behaviour;
  }

  WEBVIEW_IMPL void app_base::register_plugin(std::shared_ptr<plugin> plugin)
  {
    m_plugins.emplace_back(std::move(plugin));
  }

  WEBVIEW_IMPL noresult app_base::open_window(std::shared_ptr<window> window)
  {
    webview::window* window_ptr = window.get();
    {
      std::lock_guard lock(m_window_mutex);
      m_windows.emplace_back(std::move(window));
    }

    if (!m_stop_run_loop)
    {
      auto result = window->on_window_opened(this);
      if (!result.has_value())
        return std::move(result);
    }

    return {};
  }

  WEBVIEW_IMPL noresult app_base::run(std::shared_ptr<window> window)
  {
    m_stop_run_loop = false;

    auto result = window->on_window_opened(this);
    if (!result.has_value())
      return std::move(result);

    for (auto& pre_opened_window : m_windows)
    {
      result = pre_opened_window->on_window_opened(this);
      if (!result.has_value())
        return std::move(result);
    }

    {
      std::lock_guard lock(m_window_mutex);
      m_main_window = std::move(window);
    }

    return run_loop();
  }

  WEBVIEW_IMPL void app_base::terminate()
  {
    m_windows.clear();
    m_main_window.reset();
    m_stop_run_loop = true;
  }

  WEBVIEW_IMPL noresult app_base::on_plugin_setup_window(window& window)
  {
    for (const auto& plugin : m_plugins)
    {
      auto result = plugin->on_setup_window(window);
      if (!result.has_value())
        return std::move(result);
    }

    return {};
  }

  WEBVIEW_IMPL void app_base::on_window_closed(const window_base* window)
  {
    std::lock_guard lock(m_window_mutex);
    if (m_main_window.get() == window)
    {
      m_main_window.reset();
      if (m_shutdown_behaviour == app_shutdown_behaviour::on_main_window_closed || m_windows.empty())
        terminate();
    }
    else
    {
      const auto found_window = std::ranges::find_if(m_windows,
                                                     [&window](const std::shared_ptr<window_base>& maybe_target_window)
                                                     {
                                                       return maybe_target_window.get() == window;
                                                     });
      if (found_window != m_windows.end())
      {
        m_windows.erase(found_window);
      }

      if (m_windows.empty() && !m_main_window)
        terminate();
    }
  }
} // namespace webview::detail

#endif
