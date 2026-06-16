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
  WEBWINDOWED_IMPL std::wstring headers_for_asset_name(const std::string& assetName, const size_t contentLength)
  {
    std::wstringstream wss;

    wss << std::format(L"Content-Length: {}\n", contentLength);
    wss << std::format(L"Content-Type: {}", detail::widen_string(get_mime_type_for_file_name(assetName)));

    return wss.str();
  }

  WEBWINDOWED_IMPL HRESULT HandleResourceRequested(ICoreWebView2_22* core22,
                                                   IUnknown* args,
                                                   const std::string& protocol_name,
                                                   const std::unordered_map<std::string, asset>& asset_lookup)
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
      split_asset_request_uri(uri, protocol_name, hostname, asset);

      if (hostname == "localhost")
      {
        const auto found_ui_file = asset_lookup.find(asset);
        if (found_ui_file != asset_lookup.end())
        {
          const Microsoft::WRL::ComPtr<IStream> response_stream =
              SHCreateMemStream(static_cast<const BYTE*>(found_ui_file->second.data), static_cast<UINT>(found_ui_file->second.data_size));

          const auto headers = headers_for_asset_name(asset, found_ui_file->second.data_size);
          if (FAILED(environment->CreateWebResourceResponse(response_stream.Get(), 200, L"OK", headers.data(), &response)))
          {
            return S_FALSE;
          }

          file_found = true;
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
                                                    return HandleResourceRequested(core22.Get(), args, m_protocol_name, m_asset_lookup);
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

    std::wstring wProtocol = detail::widen_string(m_protocol_name);
    auto customSchemeRegistration = Microsoft::WRL::Make<CoreWebView2CustomSchemeRegistration>(wProtocol.c_str());
    customSchemeRegistration->put_TreatAsSecure(TRUE);
    customSchemeRegistration->put_HasAuthorityComponent(TRUE);

    ICoreWebView2CustomSchemeRegistration* registrations[]{customSchemeRegistration.Get()};
    options4->SetCustomSchemeRegistrations(std::extent_v<decltype(registrations)>, registrations);

    return {};
  }
} // namespace webwindowed

#endif
#endif
#endif
