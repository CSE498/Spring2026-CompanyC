#pragma once

#ifndef __EMSCRIPTEN__

#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>

namespace cse498 {
namespace smart_agent_bridge_detail {

constexpr std::string_view llama_url = "http://127.0.0.1:8080/v1/chat/completions"; // Keeping this local for now, will pivot to WebLLM when web GUI is working
constexpr std::string_view llama_user_prompt_prefix = "";
constexpr std::string_view llama_user_prompt_suffix = "";
constexpr int llama_max_tokens = 24;
constexpr double llama_temperature = 0;
constexpr int llama_top_k = 40;
constexpr double llama_top_p = 0.95;
constexpr int llama_seed = -1;

[[nodiscard]] inline std::string Trim(std::string text)
{
  size_t begin = 0;
  while (begin < text.size() && std::isspace(static_cast<unsigned char>(text[begin]))) {
    ++begin;
  }

  size_t end = text.size();
  while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
    --end;
  }

  return text.substr(begin, end - begin);
}

[[nodiscard]] inline std::string JsonEscape(std::string_view text)
{
  std::string escaped;
  escaped.reserve(text.size() + 16);

  for (unsigned char ch : text) {
    switch (ch) {
      case '\\': escaped += "\\\\"; break;
      case '"': escaped += "\\\""; break;
      case '\n': escaped += "\\n"; break;
      case '\r': escaped += "\\r"; break;
      case '\t': escaped += "\\t"; break;
      default:
        if (ch < 0x20) escaped += ' ';
        else escaped += static_cast<char>(ch);
        break;
    }
  }

  return escaped;
}

[[nodiscard]] inline std::string QuoteArg(std::string_view text)
{
  return "\"" + std::string(text) + "\"";
}

[[nodiscard]] inline std::string ReadCommandOutput(const std::string & command)
{
#ifdef _WIN32
  FILE * pipe = _popen(command.c_str(), "r");
#else
  FILE * pipe = popen(command.c_str(), "r");
#endif
  if (pipe == nullptr) return {};

  std::string output;
  char buffer[4096];
  while (std::fgets(buffer, static_cast<int>(sizeof(buffer)), pipe) != nullptr) {
    output += buffer;
  }

#ifdef _WIN32
  _pclose(pipe);
#else
  pclose(pipe);
#endif
  return output;
}

[[nodiscard]] inline std::string RequestPath()
{
  namespace fs = std::filesystem;
  return (fs::temp_directory_path() / "smart_agent_request.json").string();
}

inline bool WriteTextFile(const std::string & path, const std::string & contents)
{
  std::ofstream out(path, std::ios::binary);
  if (!out) return false;
  out << contents;
  return static_cast<bool>(out);
}

[[nodiscard]] inline std::string ParseJsonStringAt(const std::string & json, size_t quote_pos)
{
  if (quote_pos >= json.size() || json[quote_pos] != '"') return {};

  std::string value;
  for (size_t i = quote_pos + 1; i < json.size(); ++i) {
    const char ch = json[i];
    if (ch == '"') return value;
    if (ch != '\\') {
      value += ch;
      continue;
    }

    if (++i >= json.size()) return {};

    switch (json[i]) {
      case '"': value += '"'; break;
      case '\\': value += '\\'; break;
      case '/': value += '/'; break;
      case 'b': value += '\b'; break;
      case 'f': value += '\f'; break;
      case 'n': value += '\n'; break;
      case 'r': value += '\r'; break;
      case 't': value += '\t'; break;
      default: value += json[i]; break;
    }
  }

  return {};
}

[[nodiscard]] inline std::string ExtractChatReply(const std::string & json)
{
  // This is intentionally tiny and a little naive. We're only pulling the
  // assistant content field out of the local llama-server response shape.
  const size_t choices_pos = json.find("\"choices\"");
  if (choices_pos == std::string::npos) return {};

  const size_t message_pos = json.find("\"message\"", choices_pos);
  if (message_pos == std::string::npos) return {};

  const size_t content_pos = json.find("\"content\"", message_pos);
  if (content_pos == std::string::npos) return {};

  const size_t colon_pos = json.find(':', content_pos + 9);
  if (colon_pos == std::string::npos) return {};

  size_t value_pos = colon_pos + 1;
  while (value_pos < json.size() && std::isspace(static_cast<unsigned char>(json[value_pos]))) {
    ++value_pos;
  }

  return ParseJsonStringAt(json, value_pos);
}

[[nodiscard]] inline std::string RequestSmartAgentMoveViaLlamaServerSync(std::string system_prompt,
                                                                         std::string sent_prompt,
                                                                         int request_max_tokens)
{
  const std::string request_body =
    std::string("{")
    + "\"messages\":["
    + "{\"role\":\"system\",\"content\":\"" + JsonEscape(system_prompt) + "\"},"
    + "{\"role\":\"user\",\"content\":\"" + JsonEscape(sent_prompt) + "\"}"
    + "],"
    + "\"max_tokens\":" + std::to_string(request_max_tokens) + ","
    + "\"temperature\":" + std::to_string(llama_temperature) + ","
    + "\"top_k\":" + std::to_string(llama_top_k) + ","
    + "\"top_p\":" + std::to_string(llama_top_p) + ","
    + "\"seed\":" + std::to_string(llama_seed) + ","
    + "\"stream\":false"
    + "}";

  const std::string request_path = RequestPath();
  if (!WriteTextFile(request_path, request_body)) {
    std::cerr
      << "[SmartAgent] Could not create a temporary request file for llama-server.\n";
    return {};
  }

  const std::string response = ReadCommandOutput(
    "curl -sS --fail --max-time 15 "
    + QuoteArg(llama_url)
    + " -H " + QuoteArg("Content-Type: application/json")
    + " --data-binary " + QuoteArg("@" + request_path));

  std::remove(request_path.c_str());

  if (response.empty()) {
    std::cerr << "[SmartAgent] llama-server request failed. "
              << "Expected a local server endpoint at " << llama_url << ".\n";
    return {};
  }

  const std::string decoded_response = Trim(ExtractChatReply(response));
  std::cout << "[SmartAgent] raw response:\n" << response << "\n";
  std::cout << "[SmartAgent] decoded response:\n" << decoded_response << "\n";
  return decoded_response;
}

[[nodiscard]] inline std::future<std::string> RequestSmartAgentMoveViaLlamaServer(const SmartAgent & agent,
                                                                                   const std::string & user_prompt)
{
  const std::string system_prompt{SmartAgent::GetSystemPrompt()};
  (void) agent;
  const int request_max_tokens = llama_max_tokens;
  const std::string sent_prompt =
    std::string(llama_user_prompt_prefix) + user_prompt + std::string(llama_user_prompt_suffix);

  std::cout << "[SmartAgent] prompt sent to model:\n"
            << "[system]\n" << system_prompt << "\n"
            << "[user]\n" << sent_prompt << "\n";
  std::cout << "[SmartAgent] completion token budget: " << request_max_tokens << "\n";
  std::cout << "[SmartAgent] sending async stateless request with only current messages.\n";

  std::promise<std::string> promise;
  std::future<std::string> future = promise.get_future();
  // Kick curl work to a background thread so the world loop does not block on IO.
  std::thread([promise = std::move(promise),
               system_prompt,
               sent_prompt,
               request_max_tokens]() mutable {
    try {
      promise.set_value(RequestSmartAgentMoveViaLlamaServerSync(system_prompt,
                                                                sent_prompt,
                                                                request_max_tokens));
    }
    catch (...) {
      try {
        promise.set_value("");
      } catch (...) { }
    }
  }).detach();

  return future;
}

} // namespace smart_agent_bridge_detail

inline void EnsureSmartAgentDefaultCallback()
{
  SmartAgent::SetNpcRequestCallback(&smart_agent_bridge_detail::RequestSmartAgentMoveViaLlamaServer);
}

} // namespace cse498

#endif
