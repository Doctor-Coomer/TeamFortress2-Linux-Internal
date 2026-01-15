#ifndef PISHOCK_HPP
#define PISHOCK_HPP


#include <curl/curl.h>
#include <thread>

#include "../../gui/config.hpp"

#include "../../print.hpp"

static CURL* curl = nullptr;
static bool curl_init = false;

static std::string response_string = "";
static std::string header_string = "";


size_t curl_write_function(void* ptr, size_t size, size_t nmemb, std::string* data) {
  data->append((char*)ptr, size * nmemb);
  return size * nmemb;
}



static void init_curl(void) {
  static struct curl_slist* list = NULL;

  if (curl != nullptr) {
    return;
  }

  curl = curl_easy_init();
	  
  if(curl != nullptr) {
    curl_easy_setopt(curl, CURLOPT_URL, "https://do.pishock.com/api/apioperate");
    curl_easy_setopt(curl, CURLOPT_POST, true);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, true);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, false);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, true);
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

    char body[256];
    sprintf(body, "{\"Username\":\"%s\",\"Name\":\"TG_Bot_Script\",\"Code\":\"%s\",\"Intensity\":\"%d\",\"Duration\":\"%d\",\"Apikey\":\"%s\",\"Op\":\"%d\"}", config.debug.username.c_str(), config.debug.sharecode.c_str(), config.debug.intensity, config.debug.duration, config.debug.apikey.c_str(), config.debug.operation);
    curl_easy_setopt(curl, CURLOPT_POST, true);
    curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, body);

    curl_easy_setopt(curl, CURLOPT_HEADER, true);
    if (list == nullptr) {
      list = curl_slist_append(list, "Content-Type: application/json");
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

    // Read the body and header
    /*
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_function);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);

    char* url;
    long response_code;
    double elapsed;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &elapsed);
    curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url);
    */
  }

}

static void pishock(int intensity = config.debug.intensity, int duration = config.debug.duration, Debug::PiShockOperation operation = config.debug.operation) {
  if (curl == nullptr) {
    init_curl();
  }

  // Clamp
  if (intensity > 100)
    intensity = 100;
  if (duration > 15)
    duration = 15;
  
  char body[256];
  sprintf(body, "{\"Username\":\"%s\",\"Name\":\"TG_Bot_Script\",\"Code\":\"%s\",\"Intensity\":\"%d\",\"Duration\":\"%d\",\"Apikey\":\"%s\",\"Op\":\"%d\"}", config.debug.username.c_str(), config.debug.sharecode.c_str(), intensity, duration, config.debug.apikey.c_str(), operation);
  curl_easy_setopt(curl, CURLOPT_POST, true);
  curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, body);
  
  std::thread([&] {
    CURLcode res;
    if (curl != nullptr) {
      res = curl_easy_perform(curl);
    }
  }).detach();
}


#endif
