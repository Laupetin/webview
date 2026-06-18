#pragma once

#ifndef WEBWINDOWED_DETAIL_APP_BASE_IMPL_HPP
#define WEBWINDOWED_DETAIL_APP_BASE_IMPL_HPP

#include "app_base.hpp"
#include "macros.hpp"

#include <algorithm>

namespace webwindowed::detail
{
  WEBWINDOWED_IMPL app_base::app_base()
      : m_stop_run_loop(true),
        m_shutdown_behaviour(app_shutdown_behaviour::on_main_window_closed)
  {
  }

  WEBWINDOWED_IMPL void app_base::set_shutdown_behaviour(const app_shutdown_behaviour behaviour)
  {
    m_shutdown_behaviour = behaviour;
  }

  WEBWINDOWED_IMPL void app_base::register_plugin(std::shared_ptr<plugin> plugin)
  {
    m_plugins.emplace_back(std::move(plugin));
  }

  WEBWINDOWED_IMPL noresult app_base::open_window(std::shared_ptr<window> window)
  {
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

  WEBWINDOWED_IMPL noresult app_base::run(std::shared_ptr<window> window)
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

    result = run_loop();

    if (m_main_window)
      m_main_window->on_app_closed();

    for (auto& remaining_window : m_windows)
      remaining_window->on_app_closed();

    return result;
  }

  WEBWINDOWED_IMPL void app_base::terminate()
  {
    m_stop_run_loop = true;
  }

  WEBWINDOWED_IMPL noresult app_base::on_plugin_setup_window(window& window, const plugin_window_context& context) const
  {
    for (const auto& plugin : m_plugins)
    {
      auto result = plugin->on_setup_window(window, context);
      if (!result.has_value())
        return std::unexpected(error_info{webwindowed_error::UNSPECIFIED, std::move(result).error()});
    }

    return {};
  }

  WEBWINDOWED_IMPL noresult app_base::on_plugin_setup_environment_options(window& window, void* environment_options) const
  {
    for (const auto& plugin : m_plugins)
    {
      auto result = plugin->on_setup_environment_options(window, environment_options);
      if (!result.has_value())
        return std::unexpected(error_info{webwindowed_error::UNSPECIFIED, std::move(result).error()});
    }

    return {};
  }

  WEBWINDOWED_IMPL void app_base::on_window_closed(const window_base* window)
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
} // namespace webwindowed::detail

#endif
