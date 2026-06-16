#pragma once

#ifndef WEBWINDOWED_DETAIL_COMMANDS_IMPL_HPP
#define WEBWINDOWED_DETAIL_COMMANDS_IMPL_HPP

#include "commands.hpp"
#include "macros.hpp"

namespace webwindowed
{
  namespace detail
  {
    WEBWINDOWED_IMPL const std::string& command_handler_wrapper::name() const
    {
      return m_name;
    }

    WEBWINDOWED_IMPL command_handler_wrapper::command_handler_wrapper(std::string name)
        : m_name(std::move(name))
    {
    }

    WEBWINDOWED_IMPL command_handler_wrapper_void::command_handler_wrapper_void(std::string name, cb_t cb)
        : command_handler_wrapper(std::move(name)),
          m_cb(std::move(cb))
    {
    }

    WEBWINDOWED_IMPL bool command_handler_wrapper_void::returns_value() const
    {
      return false;
    }

    WEBWINDOWED_IMPL void command_handler_wrapper_void::call_handler(std::string promise_id, window& calling_window, std::string message_json_str)
    {
      m_cb(calling_window, std::move(message_json_str));
    }

    WEBWINDOWED_IMPL command_handler_wrapper_sync::command_handler_wrapper_sync(std::string name, cb_t cb)
        : command_handler_wrapper(std::move(name)),
          m_cb(std::move(cb))
    {
    }

    WEBWINDOWED_IMPL bool command_handler_wrapper_sync::returns_value() const
    {
      return true;
    }

    WEBWINDOWED_IMPL void command_handler_wrapper_sync::call_handler(std::string promise_id, window& calling_window, std::string message_json_str)
    {
      const auto result = m_cb(calling_window, std::move(message_json_str));
      calling_window.promise_resolve(promise_id, result);
    }

    WEBWINDOWED_IMPL command_handler_wrapper_async::command_handler_wrapper_async(std::string name, cb_t cb)
        : command_handler_wrapper(std::move(name)),
          m_cb(std::move(cb))
    {
    }

    WEBWINDOWED_IMPL bool command_handler_wrapper_async::returns_value() const
    {
      return true;
    }

    WEBWINDOWED_IMPL void command_handler_wrapper_async::call_handler(std::string promise_id, window& calling_window, std::string message_json_str)
    {
      m_cb(std::move(promise_id), calling_window, std::move(message_json_str));
    }
  } // namespace detail

  WEBWINDOWED_IMPL command_collection::command_collection(std::vector<std::unique_ptr<detail::command_handler_wrapper>> commands)
      : m_commands(std::move(commands))
  {
    m_commands.reserve(m_commands.size());
    for (const auto& command : m_commands)
    {
      m_command_lookup.emplace(command->name(), command.get());
    }
  }

  WEBWINDOWED_IMPL detail::command_handler_wrapper* command_collection::find_by_name(const std::string& name) const
  {
    const auto found_command = m_command_lookup.find(name);
    if (found_command != m_command_lookup.end())
      return found_command->second;

    return nullptr;
  }

  WEBWINDOWED_IMPL commands_builder& commands_builder::add_command_void(std::string command_name, detail::command_handler_wrapper_void::cb_t handler)
  {
    m_commands.emplace_back(std::make_unique<detail::command_handler_wrapper_void>(std::move(command_name), std::move(handler)));
    return *this;
  }

  WEBWINDOWED_IMPL commands_builder& commands_builder::add_command_sync(std::string command_name, detail::command_handler_wrapper_sync::cb_t handler)
  {
    m_commands.emplace_back(std::make_unique<detail::command_handler_wrapper_sync>(std::move(command_name), std::move(handler)));
    return *this;
  }

  WEBWINDOWED_IMPL commands_builder& commands_builder::add_command_async(std::string command_name, detail::command_handler_wrapper_async::cb_t handler)
  {
    m_commands.emplace_back(std::make_unique<detail::command_handler_wrapper_async>(std::move(command_name), std::move(handler)));
    return *this;
  }

  WEBWINDOWED_IMPL std::shared_ptr<command_collection> commands_builder::build()
  {
    auto result = std::make_shared<command_collection>(std::move(m_commands));
    m_commands = std::vector<std::unique_ptr<detail::command_handler_wrapper>>();
    return result;
  }
} // namespace webwindowed

#endif
