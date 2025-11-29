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

#ifndef WEBVIEW_PLATFORM_LINUX_GTK_COMPAT_HPP
#define WEBVIEW_PLATFORM_LINUX_GTK_COMPAT_HPP

#include "../../../macros.hpp"

#if defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)

#include <gtk/gtk.h>

#ifdef GDK_WINDOWING_X11
#include <gdk/x11/gdkx.h>
#endif

namespace webview
{
  namespace detail
  {
    /**
     * GTK compatibility helper class.
     */
    class gtk_compat
    {
  public:
      static gboolean init_check()
      {
        return gtk_init_check();
      }

      static GtkWidget* window_new()
      {
        return gtk_window_new();
      }

      static void window_set_child(GtkWindow* window, GtkWidget* widget)
      {
        gtk_window_set_child(window, widget);
      }

      static void window_remove_child(GtkWindow* window, GtkWidget* widget)
      {
        if (gtk_window_get_child(window) == widget)
        {
          gtk_window_set_child(window, nullptr);
        }
      }

      static void widget_set_visible(GtkWidget* widget, bool visible)
      {
        gtk_widget_set_visible(widget, visible ? TRUE : FALSE);
      }

      static void window_set_size(GtkWindow* window, int width, int height)
      {
        // GTK 4 can set a default window size, but unlike GTK 3 it can't resize
        // the window after it has been set up.
        gtk_window_set_default_size(window, width, height);
      }

      static void window_set_max_size(GtkWindow* window, int width, int height)
      {
        // X11-specific features are available in GTK 3 but not GTK 4
        // Avoid "unused parameter" warnings
        (void)window;
        (void)width;
        (void)height;
      }
    };

  } // namespace detail
} // namespace webview

#endif // defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)
#endif // WEBVIEW_PLATFORM_LINUX_GTK_COMPAT_HPP
