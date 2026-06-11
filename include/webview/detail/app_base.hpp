#pragma once

#ifndef WEBVIEW_DETAIL_APP_BASE_HPP
#define WEBVIEW_DETAIL_APP_BASE_HPP

#include "types.hpp"
#include "window_base.hpp"

#include <memory>
#include <mutex>

namespace webview::detail
{
  enum class app_shutdown_behaviour : unsigned char
  {
    on_main_window_closed,
    on_all_windows_closed
  };

  class window_base;

  class app_base
  {
public:
    app_base() = default;
    virtual ~app_base() = default;
    app_base(const app_base& other) = delete;
    app_base(app_base&& other) noexcept = delete;
    app_base& operator=(const app_base& other) = delete;
    app_base& operator=(app_base&& other) noexcept = delete;

    void set_shutdown_behaviour(app_shutdown_behaviour behaviour);

    noresult run(std::shared_ptr<window_base> window);

    virtual void terminate();

protected:
    void on_window_closed(const window_base* window);

    virtual noresult run_loop() = 0;

    bool m_stop_run_loop;
    app_shutdown_behaviour m_shutdown_behaviour;

    std::shared_ptr<window_base> m_main_window;
    std::vector<std::shared_ptr<window_base>> m_windows;
    std::mutex m_window_mutex;

    friend class window_base;
  };
} // namespace webview::detail

#endif
