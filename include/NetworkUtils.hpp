#pragma once

#include <WiFi.h>

#include <string>

namespace NetworkUtils {
std::string wifiStatusToString(wl_status_t status);
}