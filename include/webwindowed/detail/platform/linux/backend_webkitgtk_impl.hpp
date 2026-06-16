#pragma once

#ifndef WEBWINDOWED_PLATFORM_LINUX_BACKEND_GTK_WEBKITGTK_IMPL_HPP
#define WEBWINDOWED_PLATFORM_LINUX_BACKEND_GTK_WEBKITGTK_IMPL_HPP

#include "../../macros.hpp"

#if defined(WEBWINDOWED_PLATFORM_LINUX) && defined(WEBWINDOWED_GTK)

//
// ====================================================================
//
// This implementation uses webkit2gtk backend. It requires GTK and
// WebKitGTK libraries. Proper compiler flags can be retrieved via:
//
//   pkg-config --cflags --libs gtk4 webkitgtk-6.0
//
// ====================================================================
//

#include "backend_webkitgtk.hpp"
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

namespace webwindowed::detail
{
  WEBWINDOWED_IMPL gtk_webkit_engine::~gtk_webkit_engine()
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

  WEBWINDOWED_IMPL noresult gtk_webkit_engine::eval(const std::string& js)
  {
    // Might be null in case of termination
    if (m_webview)
    {
      // URI is null before content has begun loading.
      if (!webkit_web_view_get_uri(WEBKIT_WEB_VIEW(m_webview)))
        return {};

      webkit_web_view_evaluate_javascript(WEBKIT_WEB_VIEW(m_webview), js.c_str(), static_cast<gssize>(js.size()), nullptr, nullptr, nullptr, nullptr, nullptr);
    }
    return {};
  }

  WEBWINDOWED_IMPL void gtk_webkit_engine::dispatch_impl(std::function<void()> f)
  {
    g_idle_add_full(G_PRIORITY_HIGH_IDLE, (GSourceFunc)([](void *fn) -> int {
                      (*static_cast<dispatch_fn_t *>(fn))();
                      return G_SOURCE_REMOVE;
                    }),
                    new std::function<void()>(f),
                    [](void *fn) { delete static_cast<dispatch_fn_t *>(fn); });
  }

  WEBWINDOWED_IMPL void gtk_webkit_engine::set_window_size_impl(unsigned width, unsigned height)
  {
    // GTK 4 can set a default window size, but unlike GTK 3 it can't resize
    // the window after it has been set up.
    gtk_window_set_default_size(GTK_WINDOW(m_window), width, height);
  }

  WEBWINDOWED_IMPL void gtk_webkit_engine::set_window_min_impl(const unsigned width, const unsigned height)
  {
    gtk_widget_set_size_request(m_window, width, height);
  }

  WEBWINDOWED_IMPL void gtk_webkit_engine::set_window_max_impl(const unsigned width, const unsigned height)
  {
    // X11-specific features are available in GTK 3 but not GTK 4
  }

  WEBWINDOWED_IMPL void gtk_webkit_engine::set_window_size_fixed_impl(const bool value)
  {
    gtk_window_set_resizable(GTK_WINDOW(m_window), !value);
  }

  WEBWINDOWED_IMPL noresult gtk_webkit_engine::navigate_impl(const std::string& url)
  {
    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(m_webview), url.c_str());
    return {};
  }

  WEBWINDOWED_IMPL noresult gtk_webkit_engine::set_html_impl(const std::string& html)
  {
    webkit_web_view_load_html(WEBKIT_WEB_VIEW(m_webview), html.c_str(), nullptr);
    return {};
  }

  WEBWINDOWED_IMPL void gtk_webkit_engine::set_title_impl(const std::string& title)
  {
    gtk_window_set_title(GTK_WINDOW(m_window), title.c_str());
  }

  WEBWINDOWED_IMPL noresult gtk_webkit_engine::add_page_init_script(const std::string& js)
  {
    auto* wk_script = webkit_user_script_new(js.c_str(), WEBKIT_USER_CONTENT_INJECT_TOP_FRAME, WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START, nullptr, nullptr);

    webkit_user_content_manager_add_script(m_user_content_manager, wk_script);

    webkit_user_script_unref(wk_script);
    return {};
  }

  WEBWINDOWED_IMPL noresult gtk_webkit_engine::on_window_opened_impl()
  {
    auto result = window_init();
    if (!result.has_value())
      return std::move(result);

    result = window_settings();
    if (!result.has_value())
      return std::move(result);

    m_is_initialized = true;

    dispatch_size_default();

    window_show();

    return {};
  }

  WEBWINDOWED_IMPL window* gtk_webkit_engine::downcast_this()
  {
    return this;
  }

  WEBWINDOWED_IMPL char* gtk_webkit_engine::get_string_from_js_result(JSCValue* r)
  {
    return jsc_value_to_string(r);
  }

  WEBWINDOWED_IMPL noresult gtk_webkit_engine::window_init()
  {
    if (!gtk_init_check())
      return std::unexpected(error_info(webwindowed_error::UNSPECIFIED, "GTK init failed"));

    m_window = gtk_window_new();

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
                                                      "__webwindowed__",
                                                      [this](WebKitUserContentManager*, const std::string& r)
                                                      {
                                                        on_message(r);
                                                      });

    webkitgtk_compat::user_content_manager_register_script_message_handler(manager, "__webwindowed__");

    return {};
  }

  WEBWINDOWED_IMPL noresult gtk_webkit_engine::window_settings()
  {
    WebKitSettings* settings = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(m_webview));
    webkit_settings_set_javascript_can_access_clipboard(settings, true);

    if (m_debug)
    {
      webkit_settings_set_enable_write_console_messages_to_stdout(settings, true);
      webkit_settings_set_enable_developer_extras(settings, true);
    }

    auto res_expect = add_page_init_script(create_webwindowed_init_script("(message) => window.webkit.messageHandlers.__webwindowed__.postMessage(message)"));
    if (!res_expect.has_value())
      return res_expect;
    res_expect = add_page_init_script(create_bind_script());
    if (!res_expect.has_value())
      return res_expect;

    return call_plugin_setup_window(*this, plugin_window_context(m_window, m_webview, m_webview));
  }

  WEBWINDOWED_IMPL void gtk_webkit_engine::window_show()
  {
    if (!m_is_window_shown)
    {
      gtk_window_set_child(GTK_WINDOW(m_window), GTK_WIDGET(m_webview));
      gtk_widget_set_visible(GTK_WIDGET(m_webview), TRUE);

      gtk_widget_grab_focus(GTK_WIDGET(m_webview));
      gtk_widget_set_visible(GTK_WIDGET(m_window), TRUE);

      m_is_window_shown = true;
    }
  }

  WEBWINDOWED_IMPL void gtk_webkit_engine::run_event_loop_while(const std::function<bool()>& fn)
  {
    while (fn())
    {
      g_main_context_iteration(nullptr, TRUE);
    }
  }
} // namespace webwindowed::detail

#endif // defined(WEBWINDOWED_PLATFORM_LINUX) && defined(WEBWINDOWED_GTK)
#endif // WEBWINDOWED_BACKENDS_GTK_WEBKITGTK_HPP
