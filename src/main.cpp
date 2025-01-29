#include <Arduino.h>

#include <DNSServer.h>
#include <wifi_provisioning.h>

#include <espcam_webserver.h>

#include <OV2640.h>

#include "soc/rtc_cntl_reg.h"

#include <SoftwareSerial.h>

const char app_name[] = "esp32cam";
const char ap_password[] = "esp32cam#";

String get_mac_address()
{
	auto mac = WiFi.macAddress();
	mac.replace(":", "");
	mac.toLowerCase();
	return mac;
}

auto instance_name = String(app_name) + "-" + get_mac_address();

OV2640 cam;
espcam_webserver espcam_web(cam, instance_name);

// Define software serial pins
const int RX_PIN = 14; 
const int TX_PIN = 15; 

// Initialize SoftwareSerial
SoftwareSerial altSerial(RX_PIN, TX_PIN);

// Define touch pins
const int tMoTorgle = T0; //gpio 4


// put your setup code here, to run once:
void setup()
{
	// Disable brownout
	WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

	Serial.begin(115200);
	Serial.setDebugOutput(true);
	esp_log_level_set("*", ESP_LOG_VERBOSE);

	altSerial.begin(115200);

	log_i("CPU Freq = %d Mhz", getCpuFrequencyMhz());
	log_i("Starting ESP32Cam...");

	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, false);

	log_i("Initialize the camera");
	esp32cam_aithinker_config.frame_size = FRAMESIZE_UXGA;
	if (cam.init(esp32cam_aithinker_config) != ESP_OK)
		log_e("Initializing the camera failed");

	log_i("Instance_name: %s", instance_name.c_str());

	log_i("Connecting...");
	wifi_provisioning provisioning(instance_name);
	auto const portal_timeout_milliseconds = (uint)3 * 60 * 1000;
	if (provisioning.connect() != WL_CONNECTED)
	{
		log_i("Provisioning...");
		provisioning.start_portal(ap_password);
		auto start = millis();
		while (WiFi.softAPgetStationNum() > 0 || millis() - start < portal_timeout_milliseconds)
			provisioning.doLoop();

		log_i("Provisioning timeout. Restarting...");
		ESP.restart();
	}

	log_i("Connected! IP address: %s", WiFi.localIP().toString().c_str());

	log_i("Starting servers...");

	espcam_web.begin();
}

void loop()
{
	espcam_web.doLoop();

	// Read the touch sensor value
    int touchValue = touchRead(tMoTorgle);
	Serial.print("Touch value: ");
    Serial.println(touchValue);
	if (touchValue < 30) {
		altSerial.println("t");
	}




}
