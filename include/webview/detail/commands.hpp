#ifndef WEBVIEW_DETAIL_COMMANDS_HPP
#define WEBVIEW_DETAIL_COMMANDS_HPP

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace webview
{
  namespace detail
  {
    class engine_base;
    class command;
    class window_base;

    using response_id_t = std::uint64_t;
    using command_handler_t = std::function<void(std::string payload)>;

    class command_handler_wrapper
    {
  public:
      virtual ~command_handler_wrapper() = default;

      [[nodiscard]] const std::string& name() const;

      [[nodiscard]] virtual bool returns_value() const = 0;
      virtual void call_handler(std::string promise_id, window_base& calling_window, std::string message_json_str) = 0;

  protected:
      explicit command_handler_wrapper(std::string name);

  private:
      std::string m_name;
    };

    class command_handler_wrapper_void : public command_handler_wrapper
    {
  public:
      using cb_t = std::function<void(window_base& calling_window, std::string message_json_str)>;

      command_handler_wrapper_void(std::string name, cb_t cb);

      [[nodiscard]] bool returns_value() const override;

      void call_handler(std::string promise_id, window_base& calling_window, std::string message_json_str) override;

  private:
      cb_t m_cb;
    };

    class command_handler_wrapper_sync : public command_handler_wrapper
    {
  public:
      using cb_t = std::function<std::string(window_base& calling_window, std::string message_json_str)>;

      command_handler_wrapper_sync(std::string name, cb_t cb);

      [[nodiscard]] bool returns_value() const override;

      void call_handler(std::string promise_id, window_base& calling_window, std::string message_json_str) override;

  private:
      cb_t m_cb;
    };

    class command_handler_wrapper_async : public command_handler_wrapper
    {
  public:
      using cb_t = std::function<void(std::string promise_id, window_base& calling_window, std::string message_json_str)>;

      command_handler_wrapper_async(std::string name, cb_t cb);

      [[nodiscard]] bool returns_value() const override;

      void call_handler(std::string promise_id, window_base& calling_window, std::string message_json_str) override;

  private:
      cb_t m_cb;
    };
  } // namespace detail

  class command_collection
  {
public:
    explicit command_collection(std::vector<std::unique_ptr<detail::command_handler_wrapper>> commands);

    [[nodiscard]] detail::command_handler_wrapper* find_by_name(const std::string& name) const;

private:
    std::vector<std::unique_ptr<detail::command_handler_wrapper>> m_commands;
    std::unordered_map<std::string, detail::command_handler_wrapper*> m_command_lookup;

    friend class detail::window_base;
  };

  class commands
  {
public:
    commands() = default;

    commands& add_command_void(std::string command_name, detail::command_handler_wrapper_void::cb_t handler);
    commands& add_command_sync(std::string command_name, detail::command_handler_wrapper_sync::cb_t handler);
    commands& add_command_async(std::string command_name, detail::command_handler_wrapper_async::cb_t handler);

    std::shared_ptr<command_collection> build();
    operator std::shared_ptr<command_collection>();

protected:
    std::vector<std::unique_ptr<detail::command_handler_wrapper>> m_commands;
  };
} // namespace webview

#endif
