#pragma once

#include <Arduino.h>
#include <WiFi.h>

namespace NetworkUtils {
  String wifiStatusToString(wl_status_t status);
}