#include "webview/webview.hpp"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

constexpr auto html =
    R"html(
<div>
  <button id="increment">+</button>
  <button id="decrement">−</button>
  <span>Counter: <span id="counterResult">0</span></span>
</div>
<hr />
<div>
  <button id="compute">Compute</button>
  <span>Result: <span id="computeResult">(not started)</span></span>
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
    "increment", "decrement", "counterResult", "compute",
    "computeResult"
  ]);
  ui.increment.addEventListener("click", async () => {
    ui.counterResult.textContent = await window.webviewBinds.count(1);
  });
  ui.decrement.addEventListener("click", async () => {
    ui.counterResult.textContent = await window.webviewBinds.count(-1);
  });
  ui.compute.addEventListener("click", async () => {
    ui.compute.disabled = true;
    ui.computeResult.textContent = "(pending)";
    ui.computeResult.textContent = await window.webviewBinds.compute(6, 7);
    ui.compute.disabled = false;
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

  auto w = std::make_unique<webview::window>();
  w->set_debug(true);
  w->set_title("Bind Example");
  w->set_window_size(480, 320);

  webview::commands c;

  // A binding that counts up or down and immediately returns the new value.
  c.add_command_sync("count",
                     [&](webview::window& calling_window, std::string message_json_str) -> std::string
                     {
                       // Imagine that req is properly parsed or use your own JSON parser.
                       const auto direction = std::stol(message_json_str.substr(1, message_json_str.size() - 1));
                       return std::to_string(count += direction);
                     });

  // A binding that creates a new thread and returns the result at a later time.
  c.add_command_async("compute",
                      [&](std::string promise_id, webview::window& calling_window, std::string message_json_str)
                      {
                        // Create a thread and forget about it for the sake of simplicity.
                        std::thread(
                            [&, id = std::move(promise_id), req = std::move(message_json_str)]
                            {
                              // Simulate load.
                              std::this_thread::sleep_for(std::chrono::seconds(1));
                              // Imagine that req is properly parsed or use your own JSON parser.
                              const auto* result = "42";
                              calling_window.promise_resolve(id, result);
                            })
                            .detach();
                      });

  w->set_commands(c);
  auto result = w->set_html(html);
  if (!result.has_value())
  {
    std::cerr << "Failed to set html: " << result.error().message() << std::endl;
    return 1;
  }

  webview::app app;
  result = app.run(std::move(w));
  if (!result.has_value())
  {
    std::cerr << "Failed to run app: " << result.error().message() << std::endl;
    return 1;
  }

  return 0;
}
