#include "webwindowed/webwindowed.hpp"

#include <iostream>

#ifdef WEBWINDOWED_PLATFORM_WINDOWS
#include <windows.h>
#endif

#ifdef WEBWINDOWED_PLATFORM_WINDOWS
int WINAPI WinMain(HINSTANCE /*hInst*/, HINSTANCE /*hPrevInst*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
#else
int main()
{
#endif
  auto w = std::make_shared<webwindowed::window>();
  w->set_title("Basic Example");
  w->set_window_size(480, 320);
  auto result = w->set_html("Thanks for using webwindowed!");
  if (!result.has_value())
  {
    std::cerr << "Failed to set html: " << result.error().message() << std::endl;
    return 1;
  }

  webwindowed::app app;

  result = app.run(std::move(w));
  if (!result.has_value())
  {
    std::cerr << "Failed to run app: " << result.error().message() << std::endl;
    return 1;
  }

  return 0;
}
