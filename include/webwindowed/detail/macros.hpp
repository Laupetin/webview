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

#ifndef WEBWINDOWED_NATIVE_MACROS_HPP
#define WEBWINDOWED_NATIVE_MACROS_HPP

#if defined(__unix__)
#define WEBWINDOWED_PLATFORM_LINUX
#elif defined(_WIN32)
#define WEBWINDOWED_PLATFORM_WINDOWS
#else
#error "Unable to detect current platform"
#endif

#if !defined(WEBWINDOWED_GTK) && !defined(WEBWINDOWED_EDGE)
#if defined(WEBWINDOWED_PLATFORM_LINUX)
#define WEBWINDOWED_GTK
#elif defined(WEBWINDOWED_PLATFORM_WINDOWS)
#define WEBWINDOWED_EDGE
#else
#error "please, specify webwindowed backend"
#endif
#endif

#ifdef WEBWINDOWED_STATIC_LIB
#define WEBWINDOWED_IMPL
#else
#define WEBWINDOWED_IMPL inline
#define WEBWINDOWED_INCLUDE_IMPL
#endif

#endif // WEBWINDOWED_MACROS_HPP
