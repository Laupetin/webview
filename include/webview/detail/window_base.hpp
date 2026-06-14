/*
 * MIT License
 *
 * Copyright (c) 2017 Serge Zaitsev
 * Copyright (c) 2022 Steffen André Langnes
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#ifndef WEBVIEW_DETAIL_WINDOW_BASE_HPP
#define WEBVIEW_DETAIL_WINDOW_BASE_HPP

#include "commands.hpp"
#include "errors.hpp"
#include "plugin.hpp"

#include <format>
#include <functional>
#include <optional>
#include <string>

namespace webview
{
  class command_collection;
  class plugin;
  class plugin_window_context;

  namespace detail
  {
    class app_base;

    class initial_navigation
    {
  public:
      initial_navigation(bool is_html, std::string value);

      bool m_is_html;
      std::string m_value;
    };

    class initial_size
    {
  public:
      initial_size(unsigned width, unsigned height);

      unsigned m_width;
      unsigned m_height;
    };

    class window_base
    {
  public:
      window_base();
      virtual ~window_base() = default;

      void register_plugin(std::shared_ptr<plugin> plugin);

      noresult set_html(const std::string& html);
      noresult navigate(const std::string& url);
      void set_title(const std::string& title);

      void set_window_size(unsigned width, unsigned height);
      void set_window_min(unsigned width, unsigned height);
      void set_window_max(unsigned width, unsigned height);
      void set_window_size_fixed(bool value);

      void set_debug(bool debug);
      void set_commands(std::shared_ptr<command_collection> commands);

      void promise_resolve(const std::string& promise_id, const std::string& response_json_str);
      void promise_reject(const std::string& promise_id, const std::string& response_json_str);
      void notify(const std::string& event_key, const std::string& message_json_str);

      void dispatch(std::function<void()> f);

      virtual noresult eval(const std::string& js) = 0;

  protected:
      virtual noresult set_html_impl(const std::string& html) = 0;
      virtual noresult navigate_impl(const std::string& url) = 0;
      virtual void dispatch_impl(std::function<void()> f) = 0;
      virtual void set_title_impl(const std::string& title) = 0;

      virtual void set_window_size_impl(unsigned width, unsigned height) = 0;
      virtual void set_window_min_impl(unsigned width, unsigned height) = 0;
      virtual void set_window_max_impl(unsigned width, unsigned height) = 0;
      virtual void set_window_size_fixed_impl(bool value) = 0;

      virtual noresult add_page_init_script(const std::string& js) = 0;

      static std::string create_webview_init_script(const std::string& post_fn);

      std::string create_bind_script() const;

      virtual void on_message(const std::string& msg);

      void promise_resolve_with_status(const std::string& promise_id, const std::string& response_json_str, int status);

      void dispatch_eval(std::string js);

      // Runs the event loop until the currently queued events have been processed.
      void deplete_run_loop_event_queue();

      // Runs the event loop while the passed-in function returns true.
      virtual void run_event_loop_while(const std::function<bool()>& fn) = 0;

      void dispatch_size_default();

      noresult on_window_opened(app_base* app);
      virtual noresult on_window_opened_impl() = 0;

      noresult call_plugin_setup_window(window& window, const plugin_window_context& context) const;
      void on_window_destroyed() const;

      virtual window* downcast_this() = 0;

      bool m_is_initialized;
      bool m_debug;
      std::string m_initial_title;

  private:
      std::shared_ptr<command_collection> m_commands;
      app_base* m_app;
      std::vector<std::shared_ptr<plugin>> m_plugins;

      initial_size m_initial_size;
      std::optional<initial_size> m_initial_min;
      std::optional<initial_size> m_initial_max;
      std::optional<bool> m_initial_fixed;
      std::optional<initial_navigation> m_initial_navigation;

      static constexpr int DEFAULT_INITIAL_WIDTH = 640;
      static constexpr int DEFAULT_INITIAL_HEIGHT = 480;

      friend class app_base;
    };
  } // namespace detail
} // namespace webview

#endif // WEBVIEW_DETAIL_ENGINE_BASE_HPP
