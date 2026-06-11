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

#ifndef WEBVIEW_DETAIL_WINDOW_BASE_IMPL_HPP
#define WEBVIEW_DETAIL_WINDOW_BASE_IMPL_HPP

#include "../macros.hpp"
#include "app_base.hpp"
#include "json.hpp"
#include "window_base.hpp"

#include <format>
#include <sstream>
#include <string>

namespace webview::detail
{
  WEBVIEW_IMPL initial_navigation::initial_navigation(const bool is_html, std::string value)
      : m_is_html(is_html),
        m_value(std::move(value))
  {
  }

  WEBVIEW_IMPL window_base::window_base()
      : m_is_initialized(false),
        m_debug(false),
        m_app(nullptr),
        m_initial_width(DEFAULT_INITIAL_WIDTH),
        m_initial_height(DEFAULT_INITIAL_HEIGHT)
  {
  }

  WEBVIEW_IMPL noresult window_base::set_html(const std::string& html)
  {
    if (m_is_initialized)
      return set_html_impl(html);

    m_initial_navigation.emplace(true, html);
    return {};
  }

  WEBVIEW_IMPL noresult window_base::navigate(const std::string& url)
  {
    if (m_is_initialized)
    {
      if (url.empty())
        return navigate_impl("about:blank");

      return navigate_impl(url);
    }

    m_initial_navigation.emplace(false, url);
    return {};
  }

  WEBVIEW_IMPL void window_base::set_title(const std::string& title)
  {
    if (m_is_initialized)
    {
      set_title_impl(title);
    }
    else
    {
      m_initial_title = title;
    }
  }

  WEBVIEW_IMPL void window_base::set_window_size(const unsigned width, const unsigned height)
  {
    if (m_is_initialized)
    {
      set_window_size_impl(width, height);
    }

    m_initial_width = width;
    m_initial_height = height;
  }

  WEBVIEW_IMPL void window_base::set_debug(const bool debug)
  {
    m_debug = debug;
  }

  WEBVIEW_IMPL void window_base::set_commands(std::shared_ptr<command_collection> commands)
  {
    m_commands = std::move(commands);
  }

  WEBVIEW_IMPL void window_base::promise_resolve(const std::string& promise_id, const std::string& response_json_str)
  {
    promise_resolve_with_status(promise_id, response_json_str, 0);
  }

  WEBVIEW_IMPL void window_base::promise_reject(const std::string& promise_id, const std::string& response_json_str)
  {
    promise_resolve_with_status(promise_id, response_json_str, 1);
  }

  WEBVIEW_IMPL void window_base::notify(const std::string& event_key, const std::string& message_json_str)
  {
    dispatch_eval(std::format(
        "(function(){{window.__webview_internal.onNotify({},{});}})();", json_escape(event_key), message_json_str.empty() ? "undefined" : message_json_str));
  }

  WEBVIEW_IMPL void window_base::dispatch(std::function<void()> f)
  {
    dispatch_impl(std::move(f));
  }

  WEBVIEW_IMPL std::string window_base::create_webview_init_script(const std::string& post_fn)
  {

    return std::string(R"INIT_SCRIPT(
(function() {
  const postFunc = )INIT_SCRIPT")
           + post_fn + R"INIT_SCRIPT(;
  let callKey = 0;
  const promises = {};
  const binds = {};
  const eventListeners = {};

  function call(method, ...params) {
    const id = ++callKey;
    const promise = new Promise((resolve, reject) => {
      promises[id] = { resolve, reject };
    });

    postFunc(
      JSON.stringify({
        id,
        method,
        params,
      })
    );

    return promise;
  }

  function onBind(name) {
    if (binds[name]) {
      throw new Error('Bind "' + name + '" already exists');
    }

    binds[name] = (...params) => call(name, ...params);
  }

  function onUnbind(name) {
    if (!binds[name]) {
      throw new Error('Bind "' + name + '" does not exist');
    }

    delete binds[name];
  }

  function onReply(id, status, result) {
    const promise = promises[id];
    delete promises[id];

    if (status === 0) {
      promise.resolve(result);
    } else {
      promise.reject(result);
    }
  }

  function onNotify(eventKey, payload) {
    const listeners = eventListeners[eventKey];
    if (!listeners) {
      return;
    }

    listeners.forEach((listener) => listener(payload));
  }

  function webviewAddEventListener(eventKey, fn) {
    let listeners = eventListeners[eventKey];
    if (listeners) {
      listeners.push(fn);
    } else {
      eventListeners[eventKey] = [fn];
    }
  }

  function webviewRemoveEventListener(eventKey, fn) {
    const listeners = eventListeners[eventKey];
    if (!listeners) {
      return;
    }

    const fnIndex = listeners.indexOf(fn);
    if (fnIndex >= 0) {
      listeners.splice(fnIndex, 1);
      return true;
    }

    return false;
  }

  window.__webview_internal = Object.freeze({
    call,
    onReply,
    onBind,
    onUnbind,
    onNotify,
  });

  window.webviewBinds = binds;
  window.webviewAddEventListener = webviewAddEventListener;
  window.webviewRemoveEventListener = webviewRemoveEventListener;
})();
)INIT_SCRIPT";
  }

  WEBVIEW_IMPL std::string window_base::create_bind_script() const
  {
    std::ostringstream js_names;

    js_names << '[';
    bool first = true;
    if (m_commands)
    {
      for (const auto& binding : m_commands->m_commands)
      {
        if (first)
        {
          first = false;
        }
        else
        {
          js_names << ',';
        }
        js_names << json_escape(binding->name());
      }
    }
    js_names << ']';

    return std::string(R"INIT_SCRIPT(
  (function() {
    const methods = )INIT_SCRIPT")
           + js_names.str() + R"INIT_SCRIPT(
    methods.forEach((name) => {
      window.__webview_internal.onBind(name);
    });
  })();
    )INIT_SCRIPT";
  }

  WEBVIEW_IMPL void window_base::on_message(const std::string& msg)
  {
    const auto id = json_parse(msg, "id", 0);
    const auto name = json_parse(msg, "method", 0);
    const auto args = json_parse(msg, "params", 0);
    const auto found_command = m_commands->find_by_name(name);

    if (!found_command)
      return;

    dispatch(
        [=, this]
        {
          found_command->call_handler(id, *this, args);
        });
  }

  WEBVIEW_IMPL void window_base::promise_resolve_with_status(const std::string& promise_id, const std::string& response_json_str, int status)
  {
    dispatch_eval(std::format(
        "(function(){{window.__webview_internal.onReply({},{},{});}})();", promise_id, status, response_json_str.empty() ? "undefined" : response_json_str));
  }

  WEBVIEW_IMPL void window_base::dispatch_eval(std::string js)
  {
    dispatch(
        [js2 = std::move(js), this]
        {
          eval(js2).value();
        });
  }

  // Runs the event loop until the currently queued events have been processed.
  WEBVIEW_IMPL void window_base::deplete_run_loop_event_queue()
  {
    bool done{};

    dispatch(
        [&]
        {
          done = true;
        });

    run_event_loop_while(
        [&]
        {
          return !done;
        });
  }

  WEBVIEW_IMPL void window_base::dispatch_size_default()
  {
    if (!m_is_initialized)
      return;

    dispatch(
        [this]
        {
          set_window_size_impl(m_initial_width, m_initial_height);
        });
  }

  WEBVIEW_IMPL noresult window_base::on_window_opened(app_base* app)
  {
    m_app = app;
    auto result = on_window_opened_impl();
    if (!result.has_value())
      return std::move(result);

    if (m_is_initialized && m_initial_navigation)
    {
      if (m_initial_navigation->m_is_html)
      {
        result = set_html_impl(m_initial_navigation->m_value);
        if (!result.has_value())
          return std::move(result);
      }
      else
      {
        result = navigate_impl(m_initial_navigation->m_value);
        if (!result.has_value())
          return std::move(result);
      }
    }

    return {};
  }

  WEBVIEW_IMPL void window_base::on_window_destroyed() const
  {
    if (m_app)
      m_app->on_window_closed(this);
  }
} // namespace webview::detail

#endif // WEBVIEW_DETAIL_ENGINE_BASE_HPP
