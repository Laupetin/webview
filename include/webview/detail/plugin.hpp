#pragma once

#ifndef WEBVIEW_DETAIL_PLUGIN_HPP
#define WEBVIEW_DETAIL_PLUGIN_HPP

#include "window.hpp"

namespace webview
{
  class plugin
  {
public:
    plugin() = default;
    virtual ~plugin() = default;

    virtual noresult on_setup_window(window& window)
    {
      return {};
    };
  };
} // namespace webview

#endif
