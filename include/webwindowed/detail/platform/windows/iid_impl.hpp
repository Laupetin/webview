#pragma once

#ifndef WEBWINDOWED_DETAIL_PLATFORM_WINDOWS_IID_IMPL_HPP
#define WEBWINDOWED_DETAIL_PLATFORM_WINDOWS_IID_IMPL_HPP

#include "../../macros.hpp"

#if defined(WEBWINDOWED_PLATFORM_WINDOWS)

#ifdef _MSC_VER
#pragma comment(lib, "ole32.lib")
#endif

#endif // defined(WEBWINDOWED_PLATFORM_WINDOWS)
#endif // WEBWINDOWED_PLATFORM_WINDOWS_IID_HPP
