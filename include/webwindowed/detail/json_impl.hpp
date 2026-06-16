#pragma once

#ifndef WEBWINDOWED_DETAIL_JSON_IMPL_HPP
#define WEBWINDOWED_DETAIL_JSON_IMPL_HPP

#include <cassert>
#include <cstring>
#include <string>

namespace webwindowed::detail
{
  WEBWINDOWED_IMPL int json_parse_c(const char* str, size_t size, const char* key, size_t key_size, const char** value, size_t* value_size)
  {
    enum
    {
      JSON_STATE_VALUE,
      JSON_STATE_LITERAL,
      JSON_STATE_STRING,
      JSON_STATE_ESCAPE,
      JSON_STATE_UTF8
    } state = JSON_STATE_VALUE;

    const char* k = nullptr;
    int index = 1;
    int depth = 0;
    int utf8_bytes = 0;

    *value = nullptr;
    *value_size = 0;

    if (key == nullptr)
    {
      index = static_cast<decltype(index)>(key_size);
      if (index < 0)
        return -1;

      key_size = 0;
    }

    for (; size > 0; str++, size--)
    {
      enum
      {
        JSON_ACTION_NONE,
        JSON_ACTION_START,
        JSON_ACTION_END,
        JSON_ACTION_START_STRUCT,
        JSON_ACTION_END_STRUCT
      } action = JSON_ACTION_NONE;

      const auto c = static_cast<unsigned char>(*str);
      switch (state)
      {
      case JSON_STATE_VALUE:
      {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ',' || c == ':')
          continue;

        if (c == '"')
        {
          action = JSON_ACTION_START;
          state = JSON_STATE_STRING;
        }
        else if (c == '{' || c == '[')
        {
          action = JSON_ACTION_START_STRUCT;
        }
        else if (c == '}' || c == ']')
        {
          action = JSON_ACTION_END_STRUCT;
        }
        else if (c == 't' || c == 'f' || c == 'n' || c == '-' || (c >= '0' && c <= '9'))
        {
          action = JSON_ACTION_START;
          state = JSON_STATE_LITERAL;
        }
        else
        {
          return -1;
        }
      }
      break;

      case JSON_STATE_LITERAL:
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ',' || c == ']' || c == '}' || c == ':')
        {
          state = JSON_STATE_VALUE;
          str--;
          size++;
          action = JSON_ACTION_END;
        }
        else if (c < 32 || c > 126)
        {
          return -1;
        } // fallthrough

      case JSON_STATE_STRING:
      {
        if (c < 32 || (c > 126 && c < 192))
          return -1;

        if (c == '"')
        {
          action = JSON_ACTION_END;
          state = JSON_STATE_VALUE;
        }
        else if (c == '\\')
        {
          state = JSON_STATE_ESCAPE;
        }
        else if (c >= 192 && c < 224)
        {
          utf8_bytes = 1;
          state = JSON_STATE_UTF8;
        }
        else if (c >= 224 && c < 240)
        {
          utf8_bytes = 2;
          state = JSON_STATE_UTF8;
        }
        else if (c >= 240 && c < 247)
        {
          utf8_bytes = 3;
          state = JSON_STATE_UTF8;
        }
        else if (c >= 128 && c < 192)
        {
          return -1;
        }
      }
      break;

      case JSON_STATE_ESCAPE:
        if (c == '"' || c == '\\' || c == '/' || c == 'b' || c == 'f' || c == 'n' || c == 'r' || c == 't' || c == 'u')
        {
          state = JSON_STATE_STRING;
        }
        else
        {
          return -1;
        }

        break;
      case JSON_STATE_UTF8:
        if (c < 128 || c > 191)
          return -1;

        utf8_bytes--;
        if (utf8_bytes == 0)
          state = JSON_STATE_STRING;

        break;
      default:
        return -1;
      }

      if (action == JSON_ACTION_END_STRUCT)
        depth--;

      if (depth == 1)
      {
        if (action == JSON_ACTION_START || action == JSON_ACTION_START_STRUCT)
        {
          if (index == 0)
            *value = str;
          else if (key_size > 0 && index == 1)
            k = str;
          else
            index--;
        }
        else if (action == JSON_ACTION_END || action == JSON_ACTION_END_STRUCT)
        {
          if (*value != nullptr && index == 0)
          {
            *value_size = static_cast<size_t>(str + 1 - *value);
            return 0;
          }

          if (key_size > 0 && k != nullptr)
          {
            if (key_size == static_cast<size_t>(str - k - 1) && memcmp(key, k + 1, key_size) == 0)
              index = 0;
            else
              index = 2;

            k = nullptr;
          }
        }
      }

      if (action == JSON_ACTION_START_STRUCT)
        depth++;
    }

    return -1;
  }

  constexpr bool is_json_special_char(const char c)
  {
    return c == '"' || c == '\\' || c == '\b' || c == '\f' || c == '\n' || c == '\r' || c == '\t';
  }

  constexpr bool is_ascii_control_char(const char c)
  {
    return c >= 0 && c <= 0x1f;
  }

  WEBWINDOWED_IMPL std::string json_escape(const std::string& s, const bool add_quotes)
  {
    // Calculate the size of the resulting string.
    // Add space for the double quotes.
    size_t required_length = add_quotes ? 2 : 0;
    for (const auto c : s)
    {
      if (is_json_special_char(c))
      {
        // '\' and a single following character
        required_length += 2;
        continue;
      }

      if (is_ascii_control_char(c))
      {
        // '\', 'u', 4 digits
        required_length += 6;
        continue;
      }

      ++required_length;
    }

    // Allocate memory for resulting string only once.
    std::string result;
    result.reserve(required_length);
    if (add_quotes)
      result += '"';

    // Copy string while escaping characters.
    for (const auto c : s)
    {
      if (is_json_special_char(c))
      {
        static constexpr char SPECIAL_ESCAPE_TABLE[256] = "\0\0\0\0\0\0\0\0btn\0fr\0\0"
                                                          "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                          "\0\0\"\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                          "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                          "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                          "\0\0\0\0\0\0\0\0\0\0\0\0\\";

        result += '\\';
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        result += SPECIAL_ESCAPE_TABLE[static_cast<unsigned char>(c)];
        continue;
      }

      if (is_ascii_control_char(c))
      {
        // Escape as \u00xx
        static constexpr char HEX_ALPHABET[]{"0123456789abcdef"};
        const auto uc = static_cast<unsigned char>(c);
        const auto h = (uc >> 4) & 0x0f;
        const auto l = uc & 0x0f;
        result += "\\u00";

        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-constant-array-index)
        result += HEX_ALPHABET[h];
        result += HEX_ALPHABET[l];
        // NOLINTEND(cppcoreguidelines-pro-bounds-constant-array-index)

        continue;
      }

      result += c;
    }

    if (add_quotes)
      result += '"';

    // Should have calculated the exact amount of memory needed
    assert(required_length == result.size());
    return result;
  }

  WEBWINDOWED_IMPL int json_unescape(const char* s, size_t n, char* out)
  {
    int r = 0;
    if (*s++ != '"')
      return -1;

    while (n > 2)
    {
      char c = *s;
      if (c == '\\')
      {
        s++;
        n--;
        switch (*s)
        {
        case 'b':
          c = '\b';
          break;
        case 'f':
          c = '\f';
          break;
        case 'n':
          c = '\n';
          break;
        case 'r':
          c = '\r';
          break;
        case 't':
          c = '\t';
          break;
        case '\\':
          c = '\\';
          break;
        case '/':
          c = '/';
          break;
        case '\"':
          c = '\"';
          break;
        default: // TODO: support unicode decoding
          return -1;
        }
      }

      if (out != nullptr)
        *out++ = c;

      s++;
      n--;
      r++;
    }

    if (*s != '"')
      return -1;

    if (out != nullptr)
      *out = '\0';

    return r;
  }

  WEBWINDOWED_IMPL std::string json_parse(const std::string& s, const std::string& key, const int index)
  {
    const char* value;
    size_t value_size;

    if (key.empty())
      json_parse_c(s.c_str(), s.length(), nullptr, index, &value, &value_size);
    else
      json_parse_c(s.c_str(), s.length(), key.c_str(), key.length(), &value, &value_size);

    if (value != nullptr)
    {
      if (value[0] != '"')
        return {value, value_size};

      const int n = json_unescape(value, value_size, nullptr);
      if (n > 0)
      {
        auto* decoded = new char[n + 1];
        json_unescape(value, value_size, decoded);
        std::string result(decoded, n);
        delete[] decoded;

        return result;
      }
    }

    return "";
  }
} // namespace webwindowed::detail

#endif // WEBWINDOWED_DETAIL_JSON_HPP
