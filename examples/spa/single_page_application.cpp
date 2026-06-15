#include "webview/webview.hpp"

// Plugins
#include "webview/plugin/asset_handler.hpp"
#include "webview/plugin/favicon_handler.hpp"
#include "webview/plugin/title_handler.hpp"

// Include assets from vite build
#include "dist/ViteAssets.h"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE /*hInst*/, HINSTANCE /*hPrevInst*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
#else
int main()
{
#endif
  long count = 0;

  auto w = std::make_unique<webview::window>();
  w->set_debug(true);
  w->set_title("Title handler example");
  w->set_window_min(200, 200);
  w->set_window_max(600, 600);
  w->set_window_size(480, 320);

  webview::app app;

  const auto asset_handler_plugin = std::make_shared<webview::asset_handler_plugin>(VITE_ASSETS, std::extent_v<decltype(VITE_ASSETS)>);
  asset_handler_plugin->set_protocol_name("sample-protocol");
  app.register_plugin(asset_handler_plugin);
  app.register_plugin(std::make_shared<webview::favicon_handler_plugin>());
  app.register_plugin(std::make_shared<webview::title_handler_plugin>());

  auto result = w->navigate(asset_handler_plugin->get_url_for_asset("index.html"));
  if (!result.has_value())
  {
    std::cerr << "Failed to set html: " << result.error().message() << std::endl;
    return 1;
  }

  result = app.run(std::move(w));
  if (!result.has_value())
  {
    std::cerr << "Failed to run app: " << result.error().message() << std::endl;
    return 1;
  }

  return 0;
}
