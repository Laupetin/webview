#pragma once

#ifndef WEBVIEW_DETAIL_PLATFORM_WINDOWS_IMPL_WIN32_HPP
#define WEBVIEW_DETAIL_PLATFORM_WINDOWS_IMPL_WIN32_HPP

#include "../../macros.hpp"

#if defined(WEBVIEW_PLATFORM_WINDOWS) && defined(WEBVIEW_EDGE)

#include "app_win32_impl.hpp"
#include "backend_edge_impl.hpp"
#include "com_init_wrapper_impl.hpp"
#include "dpi_impl.hpp"
#include "iid_impl.hpp"
#include "reg_key_impl.hpp"
#include "theme_impl.hpp"
#include "version_impl.hpp"
#include "webview2/loader_impl.hpp"

#endif

#endif
