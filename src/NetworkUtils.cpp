#include "NetworkUtils.hpp"

namespace NetworkUtils {
String wifiStatusToString(wl_status_t status) {
	switch (status) {
		case WL_NO_SHIELD:
			return "No WiFi shield is present";
		case WL_IDLE_STATUS:
			return "Idle Status";
		case WL_NO_SSID_AVAIL:
			return "No SSID available";
		case WL_SCAN_COMPLETED:
			return "Scan Completed";
		case WL_CONNECTED:
			return "Connected";
		case WL_CONNECT_FAILED:
			return "Connection Failed";
		case WL_CONNECTION_LOST:
			return "Connection Lost";
		case WL_DISCONNECTED:
			return "Disconnected";
		default:
			return "Unknown Status";
	}
}
}  // namespace NetworkUtils