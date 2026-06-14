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
//
// ====================================================================
//

#include "../../errors.hpp"
#include "../../types.hpp"
#include "../../window_base.hpp"

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

      virtual ~gtk_webkit_engine();

      void set_window_min(const unsigned width, const unsigned height) override;
      void set_window_max(const unsigned width, const unsigned height) override;
      void set_window_size_fixed(const bool value) override;

      noresult eval(const std::string& js) override;

  protected:
      void dispatch_impl(std::function<void()> f) override;

      void set_window_size_impl(int width, int height) override;
      noresult navigate_impl(const std::string& url) override;
      noresult set_html_impl(const std::string& html) override;
      void set_title_impl(const std::string& title) override;

      noresult add_page_init_script(const std::string& js) override;

      noresult on_window_opened_impl() override;

      webview::window* downcast_this() override;

  private:
      static char* get_string_from_js_result(JSCValue* r);

      noresult window_init();
      noresult window_settings();
      void window_show();
      void run_event_loop_while(const std::function<bool()>& fn) override;

      GtkWidget* m_window{};
      GtkWidget* m_webview{};
      WebKitUserContentManager* m_user_content_manager{};
      static inline bool m_stop_run_loop{};
      bool m_is_window_shown{};
    };
  } // namespace detail
} // namespace webview

#endif // defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)
#endif // WEBVIEW_BACKENDS_GTK_WEBKITGTK_HPP
