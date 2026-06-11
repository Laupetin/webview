#pragma once

#ifndef WEBVIEW_DETAIL_APP_BASE_IMPL_HPP
#define WEBVIEW_DETAIL_APP_BASE_IMPL_HPP

#include "app_base.hpp"
#include "macros.hpp"

#include <algorithm>

namespace webview::detail
{
  WEBVIEW_IMPL void app_base::set_shutdown_behaviour(const app_shutdown_behaviour behaviour)
  {
    m_shutdown_behaviour = behaviour;
  }

  WEBVIEW_IMPL noresult app_base::run(std::shared_ptr<window_base> window)
  {
    m_stop_run_loop = false;

    auto result = window->on_window_opened(this);
    if (!result.has_value())
      return std::move(result);

    {
      std::lock_guard lock(m_window_mutex);
      if (m_shutdown_behaviour == app_shutdown_behaviour::on_all_windows_closed)
        m_windows.emplace_back(std::move(window));
      else
        m_main_window = std::move(window);
    }

    return run_loop();
  }

  WEBVIEW_IMPL void app_base::terminate()
  {
    m_stop_run_loop = true;
  }

  WEBVIEW_IMPL void app_base::on_window_closed(const window_base* window)
  {
    std::lock_guard lock(m_window_mutex);
    if (m_main_window.get() == window)
    {
      terminate();
      m_main_window.reset();
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
        if (m_windows.empty() && !m_main_window)
          terminate();
      }
    }
  }
} // namespace webview::detail

#endif
