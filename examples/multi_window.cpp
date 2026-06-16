#include "webwindowed/webwindowed.hpp"

#include <format>
#include <iostream>

#ifdef WEBWINDOWED_PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace
{
  std::shared_ptr<webwindowed::window> create_sample_window(const int index)
  {
    auto w = std::make_shared<webwindowed::window>();
    w->set_title(std::format("Multi window {}", index));
    w->set_window_size(480, 320);
    w->set_debug(true);
    auto result = w->set_html(std::format("This is window {}", index));
    if (!result.has_value())
    {
      std::cerr << "Failed to set html: " << result.error().message() << std::endl;
      exit(1);
    }

    return w;
  }
} // namespace

#ifdef WEBWINDOWED_PLATFORM_WINDOWS
int WINAPI WinMain(HINSTANCE /*hInst*/, HINSTANCE /*hPrevInst*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
#else
int main()
{
#endif

  webwindowed::app app;
  app.set_shutdown_behaviour(webwindowed::app_shutdown_behaviour::on_all_windows_closed);

  auto w0 = create_sample_window(0);

  auto w1 = create_sample_window(1);
  auto result = app.open_window(w1);
  if (!result.has_value())
  {
    std::cerr << "Failed to run app: " << result.error().message() << std::endl;
    return 1;
  }

  auto w2 = create_sample_window(2);
  result = app.open_window(w2);
  if (!result.has_value())
  {
    std::cerr << "Failed to run app: " << result.error().message() << std::endl;
    return 1;
  }

  result = app.run(std::move(w0));
  if (!result.has_value())
  {
    std::cerr << "Failed to run app: " << result.error().message() << std::endl;
    return 1;
  }

  return 0;
}
