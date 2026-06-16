#pragma once

#ifndef WEBWINDOWED_DETAIL_APP_BASE_HPP
#define WEBWINDOWED_DETAIL_APP_BASE_HPP

#include "plugin.hpp"
#include "types.hpp"
#include "window_base.hpp"

#include <memory>
#include <mutex>
#include <vector>

namespace webwindowed
{
  enum class app_shutdown_behaviour : unsigned char
  {
    on_main_window_closed,
    on_all_windows_closed
  };

  namespace detail
  {
    class window_base;

    class app_base
    {
  public:
      app_base();
      virtual ~app_base() = default;
      app_base(const app_base& other) = delete;
      app_base(app_base&& other) noexcept = delete;
      app_base& operator=(const app_base& other) = delete;
      app_base& operator=(app_base&& other) noexcept = delete;

      void set_shutdown_behaviour(app_shutdown_behaviour behaviour);
      void register_plugin(std::shared_ptr<plugin> plugin);

      noresult open_window(std::shared_ptr<window> window);

      noresult run(std::shared_ptr<window> window);

      virtual void terminate();

  protected:
      noresult on_plugin_setup_window(window& window, const plugin_window_context& context) const;
      noresult on_plugin_setup_environment_options(window& window, void* environment_options) const;
      void on_window_closed(const window_base* window);

      virtual noresult run_loop() = 0;

      bool m_stop_run_loop;
      app_shutdown_behaviour m_shutdown_behaviour;

      std::shared_ptr<window> m_main_window;
      std::vector<std::shared_ptr<window>> m_windows;
      std::mutex m_window_mutex;

      std::vector<std::shared_ptr<plugin>> m_plugins;

      friend class window_base;
    };
  } // namespace detail
} // namespace webwindowed

#endif
