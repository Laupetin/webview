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

#ifndef WEBVIEW_DETAIL_ENGINE_BASE_HPP
#define WEBVIEW_DETAIL_ENGINE_BASE_HPP

#include "../errors.hpp"
#include "../types.hpp"
#include "json.hpp"
#include "user_script.hpp"

#include <atomic>
#include <functional>
#include <list>
#include <map>
#include <sstream>
#include <string>

namespace webview::detail
{
  using binding_t = std::function<void(std::string, std::string, void*)>;
  using sync_binding_t = std::function<std::string(std::string)>;

  class binding_ctx_t
  {
public:
    binding_ctx_t(binding_t callback, void* arg)
        : m_callback(std::move(callback)),
          m_arg(arg)
    {
    }

    void call(std::string id, std::string args) const
    {
      if (m_callback)
        m_callback(std::move(id), std::move(args), m_arg);
    }

private:
    // This function is called upon execution of the bound JS function
    binding_t m_callback;
    // This user-supplied argument is passed to the callback
    void* m_arg;
  };

  class engine_base
  {
public:
    explicit engine_base(const bool owns_window)
        : m_owns_window(owns_window)
    {
    }

    virtual ~engine_base() = default;

    noresult navigate(const std::string& url)
    {
      if (url.empty())
        return navigate_impl("about:blank");

      return navigate_impl(url);
    }

    // Synchronous bind
    noresult bind(const std::string& name, sync_binding_t fn)
    {
      auto wrapper = [this, fn](const std::string& id, const std::string& req, void* /*arg*/)
      {
        resolve(id, 0, fn(req));
      };

      return bind(name, wrapper, nullptr);
    }

    // Asynchronous bind
    noresult bind(const std::string& name, binding_t fn, void* arg)
    {
      if (m_bindings.contains(name))
        return error_info{webview_error::DUPLICATE};

      m_bindings.emplace(name, binding_ctx_t(std::move(fn), arg));
      replace_bind_script();

      // Notify that a binding was created if the init script has already
      // set things up.
      eval("(function(){window.__webview_internal?.onBind(" + json_escape(name) + ");})();");

      return {};
    }

    noresult unbind(const std::string& name)
    {
      const auto found = m_bindings.find(name);
      if (found == m_bindings.end())
        return error_info{webview_error::NOT_FOUND};

      m_bindings.erase(found);
      replace_bind_script();
      // Notify that a binding was created if the init script has already
      // set things up.
      eval("(function(){window.__webview_internal?.onUnbind(" + json_escape(name) + ");})();");

      return {};
    }

    noresult resolve(const std::string& id, int status, const std::string& result)
    {
      return dispatch(std::bind(
          [id, status, this](const std::string& escaped_result)
          {
            eval("(function(){window.__webview_internal.onReply(" + id + "," + std::to_string(status) + "," + escaped_result + ");})();");
          },
          result.empty() ? "undefined" : result));
    }

    noresult notify(const std::string& event_key, const std::string& result)
    {
      return dispatch(std::bind(
          [this](const std::string& escaped_key, const std::string& escaped_result)
          {
            eval("(function(){window.__webview_internal.onNotify(" + escaped_key + "," + escaped_result + ");})();");
          },
          json_escape(event_key),
          result.empty() ? "undefined" : result));
    }

    virtual result<void*> window() = 0;
    virtual result<void*> widget() = 0;
    virtual result<void*> browser_controller() = 0;

    virtual noresult run() = 0;
    virtual noresult terminate() = 0;

    noresult dispatch(std::function<void()> f)
    {
      return dispatch_impl(std::move(f));
    }

    virtual noresult set_title(const std::string& title) = 0;

    virtual noresult set_window_min(int width, int height) = 0;
    virtual noresult set_window_max(int width, int height) = 0;
    virtual noresult set_window_size_fixed(bool value) = 0;

    noresult set_window_size(const int width, const int height)
    {
      auto res = set_window_size_impl(width, height);
      m_is_size_set = true;

      return res;
    }

    virtual noresult set_html(const std::string& html) = 0;

    noresult init(const std::string& js)
    {
      add_user_script(js);
      return {};
    }

    virtual noresult eval(const std::string& js) = 0;

protected:
    virtual noresult navigate_impl(const std::string& url) = 0;
    virtual noresult dispatch_impl(std::function<void()> f) = 0;
    virtual noresult set_window_size_impl(int width, int height) = 0;

    virtual user_script* add_user_script(const std::string& js)
    {
      return std::addressof(*m_user_scripts.emplace(m_user_scripts.end(), add_user_script_impl(js)));
    }

    virtual user_script add_user_script_impl(const std::string& js) = 0;

    virtual void remove_all_user_scripts_impl(const std::list<user_script>& scripts) = 0;

    virtual bool are_user_scripts_equal_impl(const user_script& first, const user_script& second) = 0;

    virtual user_script* replace_user_script(const user_script& old_script, const std::string& new_script_code)
    {
      remove_all_user_scripts_impl(m_user_scripts);
      user_script* old_script_ptr{};
      for (auto& script : m_user_scripts)
      {
        const auto is_old_script = are_user_scripts_equal_impl(script, old_script);
        script = add_user_script_impl(is_old_script ? new_script_code : script.get_code());
        if (is_old_script)
          old_script_ptr = std::addressof(script);
      }
      return old_script_ptr;
    }

    void replace_bind_script()
    {
      if (m_bind_script)
        m_bind_script = replace_user_script(*m_bind_script, create_bind_script());
      else
        m_bind_script = add_user_script(create_bind_script());
    }

    void add_init_script(const std::string& post_fn)
    {
      add_user_script(create_init_script(post_fn));
      m_is_init_script_added = true;
    }

    static std::string create_init_script(const std::string& post_fn)
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

    std::string create_bind_script() const
    {
      std::ostringstream js_names;

      js_names << '[';
      bool first = true;
      for (const auto& binding : m_bindings)
      {
        if (first)
        {
          first = false;
        }
        else
        {
          js_names << ',';
        }
        js_names << json_escape(binding.first);
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

    virtual void on_message(const std::string& msg)
    {
      const auto id = json_parse(msg, "id", 0);
      const auto name = json_parse(msg, "method", 0);
      const auto args = json_parse(msg, "params", 0);
      const auto found = m_bindings.find(name);

      if (found == m_bindings.end())
        return;

      const auto& context = found->second;
      dispatch(
          [=]
          {
            context.call(id, args);
          });
    }

    virtual void on_window_created()
    {
      inc_window_count();
    }

    virtual void on_window_destroyed(const bool skip_termination = false)
    {
      if (dec_window_count() <= 0)
      {
        if (!skip_termination)
          terminate();
      }
    }

    // Runs the event loop until the currently queued events have been processed.
    void deplete_run_loop_event_queue()
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

    // Runs the event loop while the passed-in function returns true.
    virtual void run_event_loop_while(const std::function<bool()>& fn) = 0;

    void dispatch_size_default()
    {
      if (!owns_window() || !m_is_init_script_added)
        return;

      dispatch(
          [this]
          {
            if (!m_is_size_set)
              set_window_size(INITIAL_WIDTH, INITIAL_HEIGHT);
          });
    }

    void set_default_size_guard(const bool guarded)
    {
      m_is_size_set = guarded;
    }

    bool owns_window() const
    {
      return m_owns_window;
    }

private:
    static std::atomic_uint& window_ref_count()
    {
      static std::atomic_uint ref_count{0};
      return ref_count;
    }

    static unsigned int inc_window_count()
    {
      return ++window_ref_count();
    }

    static unsigned int dec_window_count()
    {
      auto& count = window_ref_count();
      if (count > 0)
        return --count;

      return 0;
    }

    std::map<std::string, binding_ctx_t> m_bindings;
    user_script* m_bind_script{};
    std::list<user_script> m_user_scripts;

    bool m_is_init_script_added{};
    bool m_is_size_set{};
    bool m_owns_window{};

    static constexpr int INITIAL_WIDTH = 640;
    static constexpr int INITIAL_HEIGHT = 480;
  };

} // namespace webview::detail

#endif // WEBVIEW_DETAIL_ENGINE_BASE_HPP
