#include <Arduino.h>

#include <DNSServer.h>
#include <wifi_provisioning.h>

#include <espcam_webserver.h>

#include <OV2640.h>

#include "soc/rtc_cntl_reg.h"

#include <SoftwareSerial.h>
#include <PS4Controller.h>

//#include <input_controls

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

#define RXD2 14
#define TXD2 15


// put your setup code here, to run once:
void setup()
{
	// Disable brownout
	WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

	Serial.begin(115200);
	Serial.setDebugOutput(true);
	esp_log_level_set("*", ESP_LOG_VERBOSE);

	Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

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


//--NOTE: at the moment you will need the sixaxispair tool to either change your controllers mac address, or copy yours here
	PS4.begin("a0:d7:95:6b:6b:cd");

	//indicate successful setup
	digitalWrite(LED_BUILTIN, true);
	delay(250);
	digitalWrite(LED_BUILTIN, false);
	delay(250);
	digitalWrite(LED_BUILTIN, true);
	delay(250);
	digitalWrite(LED_BUILTIN, false);
}


void moveControl(){
	if(PS4.event.button_down.options){
		Serial2.println("t");
		Serial.println("options down, sending t to toggle motors");
		delay(10);
	}
	
	if(PS4.event.button_down.circle){
		Serial2.println("3");
		Serial.println("circle down, sending 3");
	}
	if(PS4.event.button_up.circle){
		Serial2.println("0");
		Serial.println("stopping");
	}
	if(PS4.event.button_down.cross){
		Serial2.println("1");
		Serial.println("cross down, sending 1");
	}
	if(PS4.event.button_up.cross){
		Serial2.println("0");
		Serial.println("stopping");
	}
	if(PS4.event.button_down.square){
		Serial2.println("7");
		Serial.println("square down, sending 7");
	}
	if(PS4.event.button_up.square){
		Serial2.println("0");
		Serial.println("stopping");
	}
	if(PS4.event.button_down.triangle){
		Serial2.println("9");
		Serial.println("triangle down, sending 9");
	}
	if(PS4.event.button_up.triangle){
		Serial2.println("0");
		Serial.println("stopping");
	}
	if(PS4.event.button_down.up){
		Serial2.println("8");
		Serial.println("up, sending 8");
	}
	if(PS4.event.button_up.up){
		Serial2.println("0");
		Serial.println("stopping");
	}
	if(PS4.event.button_down.down){
		Serial2.println("2");
		Serial.println("down, sending 2");
	}
	if(PS4.event.button_up.down){
		Serial2.println("0");
		Serial.println("stopping");
	}
	if(PS4.event.button_down.left){
		Serial2.println("4");
		Serial.println("left, sending 4");
	}
	if(PS4.event.button_up.left){
		Serial2.println("0");
		Serial.println("stopping");
	}
	if(PS4.event.button_down.right){
		Serial2.println("6");
		Serial.println("right, sending 6");
	}
	if(PS4.event.button_up.right){
		Serial2.println("0");
		Serial.println("stopping");
	}
	if(PS4.event.button_down.l1){
		Serial2.println("s");
		Serial.println("l1 down, sending s to spin");
	}
	if(PS4.event.button_up.l1){
		Serial2.println("0");
		Serial.println("stopping");
	}
	if(PS4.event.button_down.r1){
		Serial2.println("5");
		Serial.println("r1 down, sending 5 to spin");
	}
	if(PS4.event.button_up.r1){
		Serial2.println("0");
		Serial.println("stopping");
	}
}

void loop()
{
	espcam_web.doLoop();

	moveControl();




}
