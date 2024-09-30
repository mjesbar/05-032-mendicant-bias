#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include <thread>
#include <cstdlib>

using json = nlohmann::json;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
  size_t total_size = size * nmemb;
  output->append((char*)contents, total_size);
  return total_size;
}

std::string readFileContent(const std::string& filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Unable to open file: " + filename);
  }
  return std::string((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
}

void speakText(const std::string& text) {
  std::string command = "say \'" + text + "\'";
  system(command.c_str());
}

int main() {
  CURL* curl;
  CURLcode res;
  std::string url = "https://api.anthropic.com/v1/messages";

  // Read API key from file
  std::string api_key;
  try {
    api_key = readFileContent("/home/mjesbar/Desktop/anthropic_apikey");
    api_key.erase(std::remove(api_key.begin(), api_key.end(), '\n'), api_key.end());
  } catch (const std::exception& e) {
    std::cerr << "Error reading API key: " << e.what() << std::endl;
    return 1;
  }

  // Read system prompt from file
  std::string system_prompt;
  try {
    system_prompt = readFileContent("system.md");
  } catch (const std::exception& e) {
    std::cerr << "Error reading system prompt: " << e.what() << std::endl;
    return 1;
  }

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  std::vector<json> conversation_history;

  if (curl) {
    while (true) {
      std::string prompt;
      std::cout << "['quit' to exit]\nPrompt: ";
      std::getline(std::cin, prompt);

      if (prompt == "quit") {
        break;
      }

      conversation_history.push_back({{"role", "user"}, {"content", prompt}});

      json request_data = {
        {"model", "claude-3-haiku-20240307"},
        {"system", system_prompt},
        {"messages", conversation_history},
        {"max_tokens", 4096},
        {"temperature", 0.5}
      };

      std::string request_body = request_data.dump();
      std::string response;

      struct curl_slist* headers = NULL;
      headers = curl_slist_append(headers, "Content-Type: application/json");
      headers = curl_slist_append(headers, ("x-api-key: " + api_key).c_str());
      headers = curl_slist_append(headers, "anthropic-version: 2023-06-01");

      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body.c_str());
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request_body.length());
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

      res = curl_easy_perform(curl);

      if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() fail: " << curl_easy_strerror(res) << std::endl;
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        std::cerr << "HTTP response code: " << response_code << std::endl;
      } else {
        try {
          json response_json = json::parse(response);
          
          if (response_json.contains("error")) {
            std::cerr << "API Error: " << response_json["error"]["message"] << std::endl;
            // remove the lass "user" message from the conversation history
            conversation_history.pop_back();
          } else if (response_json.contains("content")) {
            std::string ai_response = response_json["content"][0]["text"];

            std::cout << "Claude's response:" << std::endl;
            for (char c : ai_response) {
              std::cout << c << std::flush;
              std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
            std::cout << std::endl << std::endl;

            conversation_history.push_back(
                {{"role", "assistant"}, {"content", ai_response}}
            );

            // Speak the response
            speakText(ai_response);
          } else {
            std::cerr << "Unexpected response format. Raw response:" << std::endl;
            std::cerr << response << std::endl;
          }
        } catch (const json::exception& e) {
          std::cerr << "JSON parsing error: " << e.what() << std::endl;
          std::cerr << "Raw response:" << std::endl << response << std::endl;
        }
      }

      curl_slist_free_all(headers);
    }

    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();
  return 0;
}
