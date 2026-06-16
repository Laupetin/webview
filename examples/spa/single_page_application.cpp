#include "webwindowed/webwindowed.hpp"

// Plugins
#include "webwindowed/plugin/asset_handler.hpp"
#include "webwindowed/plugin/favicon_handler.hpp"
#include "webwindowed/plugin/title_handler.hpp"

// Include assets from vite build
#include "dist/ViteAssets.h"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE /*hInst*/, HINSTANCE /*hPrevInst*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
#else
int main()
{
#endif
  auto w = std::make_unique<webwindowed::window>();
  w->set_debug(true);
  w->set_title("Single page application example");
  w->set_window_min(200, 200);
  w->set_window_max(600, 600);
  w->set_window_size(480, 400);

  webwindowed::app app;

  const auto asset_handler_plugin = std::make_shared<webwindowed::asset_handler_plugin>(VITE_ASSETS, std::extent_v<decltype(VITE_ASSETS)>);
  asset_handler_plugin->set_protocol_name("sample-protocol");
  app.register_plugin(asset_handler_plugin);
  app.register_plugin(std::make_shared<webwindowed::favicon_handler_plugin>());
  app.register_plugin(std::make_shared<webwindowed::title_handler_plugin>());

  webwindowed::commands_builder commands_builder;
  commands_builder.add_command_sync("path",
                                    [](webwindowed::window& calling_window, std::string message_json_str)
                                    {
                                      auto current_path = std::filesystem::current_path().string();
                                      std::ranges::replace(current_path, '\\', '/');
                                      std::ranges::replace(current_path, '"', '\'');

                                      return std::format("\"{}\"", current_path);
                                    });
  w->set_commands(commands_builder.build());

#ifdef _DEBUG
  auto result = w->navigate(VITE_DEV_SERVER ? std::format("http://localhost:{}", VITE_DEV_SERVER_PORT) : asset_handler_plugin->get_url_for_asset("index.html"));
#else
  auto result = w->navigate(asset_handler_plugin->get_url_for_asset("index.html"));
#endif
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
