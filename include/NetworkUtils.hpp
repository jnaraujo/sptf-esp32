#pragma once

#include <Arduino.h>
#include <WiFi.h>

namespace NetworkUtils {
auto wifiStatusToString(wl_status_t status) -> String;
}