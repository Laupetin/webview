#include "webwindowed/plugin/title_handler.hpp"
#include "webwindowed/webwindowed.hpp"

#include <chrono>
#include <iostream>
#include <string>

constexpr auto html =
    R"html(
<div>
  <input id="titleInput" type="text" placeholder="Enter new title" />
</div>
<hr />
<div>
  <button id="action">Set title</button>
</div>

<style>
body {
  background: #e3e3e3;
  color: #000000;
}
@media (prefers-color-scheme: dark) {
  body {
    background: #1c1c1c;
    color: #ffffff;
  }
}
</style>

<script type="module">
  const getElements = ids => Object.assign({}, ...ids.map(
    id => ({ [id]: document.getElementById(id) })));
  const ui = getElements([
    "action", "titleInput"
  ]);
  ui.action.addEventListener("click", async () => {
    document.title = ui.titleInput.value;
  });
</script>
)html";

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE /*hInst*/, HINSTANCE /*hPrevInst*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
#else
int main()
{
#endif
  long count = 0;

  auto w = std::make_unique<webwindowed::window>();
  w->set_debug(true);
  w->set_title("Title handler example");
  w->set_window_min(200, 200);
  w->set_window_max(600, 600);
  w->set_window_size(480, 320);

  auto result = w->set_html(html);
  if (!result.has_value())
  {
    std::cerr << "Failed to set html: " << result.error().message() << std::endl;
    return 1;
  }

  webwindowed::app app;

  app.register_plugin(std::make_shared<webwindowed::title_handler_plugin>());

  result = app.run(std::move(w));
  if (!result.has_value())
  {
    std::cerr << "Failed to run app: " << result.error().message() << std::endl;
    return 1;
  }

  return 0;
}
