#pragma once

#ifndef WEBWINDOWED_PLUGIN_DETAIL_ASSET_HANDLER_WINDOWS_HPP
#define WEBWINDOWED_PLUGIN_DETAIL_ASSET_HANDLER_WINDOWS_HPP

#include "../../../plugin/asset_handler.hpp"
#include "../../macros.hpp"
#include "asset_handler_impl.hpp"

#if defined(WEBWINDOWED_PLATFORM_WINDOWS) && defined(WEBWINDOWED_EDGE)
#ifdef WEBWINDOWED_INCLUDE_IMPL

#include "../../platform/windows/webview2/loader.hpp"

#include <Windows.h>
#include <sstream>
#include <wrl/event.h>

namespace webwindowed
{
  WEBWINDOWED_IMPL std::wstring headers_for_asset_name(const std::string& asset_name, const size_t content_length, const std::string& user_content_type)
  {
    std::wstringstream wss;

    wss << std::format(L"Content-Length: {}\n", content_length);
    wss << std::format(L"Content-Type: {}", detail::widen_string(user_content_type.empty() ? get_mime_type_for_file_name(asset_name) : user_content_type));

    return wss.str();
  }

  WEBWINDOWED_IMPL HRESULT handle_resource_requested(ICoreWebView2_22* core22,
                                                     IUnknown* args,
                                                     const std::string& protocol_name,
                                                     std::unordered_map<std::string, static_asset> static_asset_lookup,
                                                     std::unordered_map<std::string, dynamic_asset> dynamic_asset_lookup)
  {
    Microsoft::WRL::ComPtr<ICoreWebView2WebResourceRequestedEventArgs2> web_resource_request_args;
    if (SUCCEEDED(args->QueryInterface(IID_PPV_ARGS(&web_resource_request_args))))
    {
      COREWEBVIEW2_WEB_RESOURCE_REQUEST_SOURCE_KINDS request_source_kind = COREWEBVIEW2_WEB_RESOURCE_REQUEST_SOURCE_KINDS_ALL;
      if (FAILED(web_resource_request_args->get_RequestedSourceKind(&request_source_kind)))
      {
        return S_FALSE;
      }

      Microsoft::WRL::ComPtr<ICoreWebView2WebResourceRequest> request;
      if (FAILED(web_resource_request_args->get_Request(&request)))
      {
        return S_FALSE;
      }

      LPWSTR wide_uri;
      if (FAILED(request->get_Uri(&wide_uri)))
      {
        return S_FALSE;
      }

      Microsoft::WRL::ComPtr<ICoreWebView2Environment> environment;
      if (FAILED(core22->get_Environment(&environment)))
      {
        return S_FALSE;
      }

      Microsoft::WRL::ComPtr<ICoreWebView2WebResourceResponse> response;

      const auto uri = detail::narrow_string(wide_uri);
      bool file_found = false;

      std::string hostname;
      std::string asset;
      std::string query;
      split_asset_request_uri(uri, protocol_name, hostname, asset, query);

      if (hostname == "localhost")
      {
        const auto found_static_file = static_asset_lookup.find(asset);
        if (found_static_file != static_asset_lookup.end())
        {
          const void* data;
          size_t data_size;
          found_static_file->second.get_data(data, data_size);

          const Microsoft::WRL::ComPtr<IStream> response_stream = SHCreateMemStream(static_cast<const BYTE*>(data), static_cast<UINT>(data_size));

          const auto headers = headers_for_asset_name(asset, data_size, std::string());
          if (FAILED(environment->CreateWebResourceResponse(response_stream.Get(), 200, L"OK", headers.data(), &response)))
            return S_FALSE;

          file_found = true;
        }

        if (!file_found)
        {
          const auto found_dynamic_file = dynamic_asset_lookup.find(asset);
          if (found_dynamic_file != dynamic_asset_lookup.end())
          {
            auto failed = false;
            dynamic_asset_request dyn_request(std::move(query));
            dynamic_asset_response dyn_response(
                [&](const int code, const void* data, const size_t data_size, const std::string& content_type)
                {
                  const Microsoft::WRL::ComPtr<IStream> response_stream = SHCreateMemStream(static_cast<const BYTE*>(data), static_cast<UINT>(data_size));

                  const auto headers = headers_for_asset_name(asset, data_size, content_type);
                  if (FAILED(environment->CreateWebResourceResponse(response_stream.Get(), code, L"OK", headers.data(), &response)))
                    failed = true;
                  else
                    file_found = true;
                });

            found_dynamic_file->second.call(dyn_request, dyn_response);

            if (failed)
              return S_FALSE;
          }
        }
      }

      if (!file_found)
      {
        if (FAILED(environment->CreateWebResourceResponse(nullptr, 404, L"Not found", L"", &response)))
        {
          return S_FALSE;
        }
      }

      if (FAILED(web_resource_request_args->put_Response(response.Get())))
      {
        return S_FALSE;
      }

      return S_OK;
    }

    return S_FALSE;
  }

  WEBWINDOWED_IMPL std::expected<void, std::string> asset_handler_plugin::on_setup_window(window& window, const plugin_window_context& context)
  {
    const auto maybe_browser_controller = context.browser_controller();
    if (!maybe_browser_controller.has_value())
      return std::unexpected("Failed to get browser controller");
    const auto controller = static_cast<ICoreWebView2Controller*>(maybe_browser_controller.value());

    Microsoft::WRL::ComPtr<ICoreWebView2> core;
    if (FAILED(controller->get_CoreWebView2(&core)))
      return std::unexpected("Failed to get webview");

    Microsoft::WRL::ComPtr<ICoreWebView2_22> core22;
    if (FAILED(core->QueryInterface(IID_PPV_ARGS(&core22))))
      return std::unexpected("Failed to get core22");

    const auto filter = std::format(L"{}://*", detail::widen_string(m_protocol_name));
    if (FAILED(core22->AddWebResourceRequestedFilterWithRequestSourceKinds(
            filter.c_str(), COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL, COREWEBVIEW2_WEB_RESOURCE_REQUEST_SOURCE_KINDS_ALL)))
    {
      return std::unexpected("Failed to install request filter");
    }

    EventRegistrationToken token;
    if (FAILED(core->add_WebResourceRequested(Microsoft::WRL::Callback<ICoreWebView2WebResourceRequestedEventHandler>(
                                                  [core22, this](ICoreWebView2* sender, IUnknown* args) -> HRESULT
                                                  {
                                                    return handle_resource_requested(
                                                        core22.Get(), args, m_protocol_name, m_static_asset_lookup, m_dynamic_asset_lookup);
                                                  })
                                                  .Get(),
                                              &token)))
    {
      return std::unexpected("Failed to add resource requested filter");
    }

    return {};
  }

  WEBWINDOWED_IMPL std::expected<void, std::string> asset_handler_plugin::on_setup_environment_options(window& window, void* environment_options)
  {
    Microsoft::WRL::ComPtr<ICoreWebView2EnvironmentOptions4> options4;
    const auto options = static_cast<ICoreWebView2EnvironmentOptions*>(environment_options);
    if (FAILED(options->QueryInterface(IID_PPV_ARGS(&options4))))
      return std::unexpected("Failed to get options4");

    const std::wstring wide_protocol = detail::widen_string(m_protocol_name);
    const auto custom_scheme_registration = Microsoft::WRL::Make<CoreWebView2CustomSchemeRegistration>(wide_protocol.c_str());
    if (FAILED(custom_scheme_registration->put_TreatAsSecure(TRUE)))
      return std::unexpected("Failed to set treatAsSecure");

    if (FAILED(custom_scheme_registration->put_HasAuthorityComponent(TRUE)))
      return std::unexpected("Failed to set hasAuthorityComponent");

    ICoreWebView2CustomSchemeRegistration* registrations[]{custom_scheme_registration.Get()};
    if (FAILED(options4->SetCustomSchemeRegistrations(std::extent_v<decltype(registrations)>, registrations)))
      return std::unexpected("Failed to set custom scheme registrations");

    return {};
  }
} // namespace webwindowed

#endif
#endif
#endif
