#include "webwindowed/plugin/favicon_handler.hpp"
#include "webwindowed/webwindowed.hpp"

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

  auto w = std::make_unique<webwindowed::window>();
  w->set_debug(true);
  w->set_title("Favicon handler example");
  w->set_window_min(200, 200);
  w->set_window_max(600, 600);
  w->set_window_size(480, 320);

  auto result = w->navigate("https://github.com");
  if (!result.has_value())
  {
    std::cerr << "Failed to set navigate: " << result.error().message() << std::endl;
    return 1;
  }

  webwindowed::app app;

  app.register_plugin(std::make_shared<webwindowed::favicon_handler_plugin>());

  result = app.run(std::move(w));
  if (!result.has_value())
  {
    std::cerr << "Failed to run app: " << result.error().message() << std::endl;
    return 1;
  }

  return 0;
}
