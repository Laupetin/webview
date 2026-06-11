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

#ifndef WEBVIEW_PLATFORM_LINUX_BACKEND_GTK_WEBKITGTK_HPP
#define WEBVIEW_PLATFORM_LINUX_BACKEND_GTK_WEBKITGTK_HPP

#include "../../macros.hpp"

#if defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)

//
// ====================================================================
//
// This implementation uses webkit2gtk backend. It requires GTK and
// WebKitGTK libraries. Proper compiler flags can be retrieved via:
//
//   pkg-config --cflags --libs gtk4 webkitgtk-6.0
//   pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.1
//   pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0
//
// ====================================================================
//

#include "../../../errors.hpp"
#include "../../../types.hpp"
#include "../../window_base.hpp"
#include "gtk_compat.hpp"
#include "webkitgtk_compat.hpp"

#include <fcntl.h>
#include <functional>
#include <gtk/gtk.h>
#include <jsc/jsc.h>
#include <list>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <webkit/webkit.h>

namespace webview
{
  namespace detail
  {
    class gtk_webkit_engine : public window_base
    {
  public:
      gtk_webkit_engine() = default;

      gtk_webkit_engine(const gtk_webkit_engine&) = delete;
      gtk_webkit_engine& operator=(const gtk_webkit_engine&) = delete;
      gtk_webkit_engine(gtk_webkit_engine&&) = delete;
      gtk_webkit_engine& operator=(gtk_webkit_engine&&) = delete;

      virtual ~gtk_webkit_engine()
      {
        if (m_window)
        {
          // Disconnect handlers to avoid callbacks invoked during destruction.
          g_signal_handlers_disconnect_by_data(GTK_WINDOW(m_window), this);
          gtk_window_close(GTK_WINDOW(m_window));
          on_window_destroyed();
        }

        if (m_webview)
          g_object_unref(m_webview);

        // Needed for the window to close immediately.
        deplete_run_loop_event_queue();
      }

      result<void*> window() override
      {
        if (m_window)
          return m_window;

        return std::unexpected(error_info{webview_error::INVALID_STATE});
      }

      result<void*> widget() override
      {
        if (m_webview)
          return m_webview;

        return std::unexpected(error_info{webview_error::INVALID_STATE});
      }

      result<void*> browser_controller() override
      {
        if (m_webview)
          return m_webview;

        return std::unexpected(error_info{webview_error::INVALID_STATE});
      }

      void set_window_min(const unsigned width, const unsigned height) override
      {
        gtk_widget_set_size_request(m_window, width, height);

        window_show();
      }

      void set_window_max(const unsigned width, const unsigned height) override
      {
        gtk_compat::window_set_max_size(GTK_WINDOW(m_window), width, height);

        window_show();
      }

      void set_window_size_fixed(const bool value) override
      {
        gtk_window_set_resizable(GTK_WINDOW(m_window), !value);
      }

      noresult eval(const std::string& js) override
      {
        // URI is null before content has begun loading.
        if (!webkit_web_view_get_uri(WEBKIT_WEB_VIEW(m_webview)))
          return {};

        webkit_web_view_evaluate_javascript(
            WEBKIT_WEB_VIEW(m_webview), js.c_str(), static_cast<gssize>(js.size()), nullptr, nullptr, nullptr, nullptr, nullptr);
        return {};
      }

  protected:
      void dispatch_impl(std::function<void()> f) override
      {
        g_idle_add_full(G_PRIORITY_HIGH_IDLE, (GSourceFunc)([](void *fn) -> int {
                      (*static_cast<dispatch_fn_t *>(fn))();
                      return G_SOURCE_REMOVE;
                    }),
                    new std::function<void()>(f),
                    [](void *fn) { delete static_cast<dispatch_fn_t *>(fn); });
      }

      void set_window_size_impl(int width, int height) override
      {
        gtk_compat::window_set_size(GTK_WINDOW(m_window), width, height);
        window_show();
      }

      noresult navigate_impl(const std::string& url) override
      {
        webkit_web_view_load_uri(WEBKIT_WEB_VIEW(m_webview), url.c_str());
        return {};
      }

      noresult set_html_impl(const std::string& html) override
      {
        webkit_web_view_load_html(WEBKIT_WEB_VIEW(m_webview), html.c_str(), nullptr);
        return {};
      }

      void set_title_impl(const std::string& title) override
      {
        gtk_window_set_title(GTK_WINDOW(m_window), title.c_str());
      }

      noresult add_page_init_script(const std::string& js) override
      {
        auto* wk_script =
            webkit_user_script_new(js.c_str(), WEBKIT_USER_CONTENT_INJECT_TOP_FRAME, WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START, nullptr, nullptr);

        webkit_user_content_manager_add_script(m_user_content_manager, wk_script);

        webkit_user_script_unref(wk_script);
        return {};
      }

      noresult on_window_opened_impl() override
      {
        auto result = window_init();
        if (!result.has_value())
          return std::move(result);

        result = window_settings();
        if (!result.has_value())
          return std::move(result);

        m_is_initialized = true;

        dispatch_size_default();

        return {};
      }

  private:
      static char* get_string_from_js_result(JSCValue* r)
      {
        return jsc_value_to_string(r);
      }

      noresult window_init()
      {
        if (!gtk_compat::init_check())
          return std::unexpected(error_info(webview_error::UNSPECIFIED, "GTK init failed"));

        m_window = gtk_compat::window_new();

        auto on_window_destroy = +[](GtkWidget*, gpointer arg)
        {
          auto* w = static_cast<gtk_webkit_engine*>(arg);
          w->m_window = nullptr;
          w->on_window_destroyed();
        };
        g_signal_connect(G_OBJECT(m_window), "destroy", G_CALLBACK(on_window_destroy), this);

        if (!m_initial_title.empty())
        {
          set_title_impl(m_initial_title);
        }

        // Initialize webview widget
        m_webview = webkit_web_view_new();
        g_object_ref_sink(m_webview);

        WebKitUserContentManager* manager = m_user_content_manager = webkit_web_view_get_user_content_manager(WEBKIT_WEB_VIEW(m_webview));
        webkitgtk_compat::connect_script_message_received(manager,
                                                          "__webview__",
                                                          [this](WebKitUserContentManager*, const std::string& r)
                                                          {
                                                            on_message(r);
                                                          });

        webkitgtk_compat::user_content_manager_register_script_message_handler(manager, "__webview__");

        return {};
      }

      noresult window_settings()
      {
        WebKitSettings* settings = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(m_webview));
        webkit_settings_set_javascript_can_access_clipboard(settings, true);

        if (m_debug)
        {
          webkit_settings_set_enable_write_console_messages_to_stdout(settings, true);
          webkit_settings_set_enable_developer_extras(settings, true);
        }

        auto res_expect = add_page_init_script(create_webview_init_script("(message) => window.webkit.messageHandlers.__webview__.postMessage(message)"));
        if (!res_expect.has_value())
          return res_expect;
        res_expect = add_page_init_script(create_bind_script());
        if (!res_expect.has_value())
          return res_expect;

        return {};
      }

      void window_show()
      {
        if (!m_is_window_shown)
        {
          gtk_compat::window_set_child(GTK_WINDOW(m_window), GTK_WIDGET(m_webview));
          gtk_compat::widget_set_visible(GTK_WIDGET(m_webview), true);

          gtk_widget_grab_focus(GTK_WIDGET(m_webview));
          gtk_compat::widget_set_visible(GTK_WIDGET(m_window), true);

          m_is_window_shown = true;
        }
      }

      void run_event_loop_while(const std::function<bool()>& fn) override
      {
        while (fn())
        {
          g_main_context_iteration(nullptr, TRUE);
        }
      }

      GtkWidget* m_window{};
      GtkWidget* m_webview{};
      WebKitUserContentManager* m_user_content_manager{};
      static inline bool m_stop_run_loop{};
      bool m_is_window_shown{};
    };

  } // namespace detail

  using window = detail::gtk_webkit_engine;
} // namespace webview

#endif // defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)
#endif // WEBVIEW_BACKENDS_GTK_WEBKITGTK_HPP
