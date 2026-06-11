#pragma once

#ifndef WEBVIEW_DETAIL_PLATFORM_WINDOWS_IID_IMPL_HPP
#define WEBVIEW_DETAIL_PLATFORM_WINDOWS_IID_IMPL_HPP

#include "../../macros.hpp"

#if defined(WEBVIEW_PLATFORM_WINDOWS)

#ifdef _MSC_VER
#pragma comment(lib, "ole32.lib")
#endif

#endif // defined(WEBVIEW_PLATFORM_WINDOWS)
#endif // WEBVIEW_PLATFORM_WINDOWS_IID_HPP
