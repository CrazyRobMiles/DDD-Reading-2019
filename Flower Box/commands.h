#pragma once

// used from timing.h
void setup_timing();

#define EEPROM_SIZE 2000
#define SETTINGS_EEPROM_OFFSET 0

#define WORKED_OK 0
#define NUMERIC_VALUE_NOT_AN_INTEGER 1
#define NUMERIC_VALUE_BELOW_MINIMUM 2
#define NUMERIC_VALUE_ABOVE_MAXIMUM 3
#define INVALID_HEX_DIGIT_IN_VALUE 4
#define INCOMING_HEX_VALUE_TOO_BIG_FOR_BUFFER 5
#define VALUE_MISSING_OR_NOT_A_STRING 6
#define INCOMING_HEX_VALUE_IS_THE_WRONG_LENGTH 7
#define INVALID_OR_MISSING_TARGET_IN_RECEIVED_COMMAND 8
#define COMMAND_FOR_DIFFERENT_TARGET 9
#define INVALID_OR_MISSING_DEVICE_NAME 10
#define INVALID_DEVICE_NAME_FOR_MQTT_ON 11
#define STRING_VALUE_MISSING_OR_NOT_STRING 13
#define STRING_VALUE_TOO_LONG 14
#define INVALID_LORA_ACCESS_SETTING 15
#define INVALID_COMMAND_NAME 16
#define JSON_COMMAND_COULD_NOT_BE_PARSED 17
#define JSON_COMMAND_MISSING_VERSION 18
#define JSON_COMMAND_MISSING_COMMAND 19
#define JSON_COMMAND_MISSING_OPTION 20
#define INVALID_MQTT_STATUS_SETTING 21
#define INVALID_LORA_STATUS_SETTING 22
#define MISSING_WIFI_SETTING_NUMBER 23
#define INVALID_WIFI_SETTING_NUMBER 24

#define REPLY_ELEMENT_SIZE 100
#define COMMAND_REPLY_BUFFER_SIZE 240

char command_reply_buffer[COMMAND_REPLY_BUFFER_SIZE];

void build_command_reply(int errorNo, JsonObject& root, char * resultBuffer)
{
	char replyBuffer[REPLY_ELEMENT_SIZE];

	const char * sequence = root["seq"];

	if (sequence)
	{
		// Got a sequence number in the command - must return the same number
		// so that the sender can identify the command that was sent
		int sequenceNo = root["seq"];
		sprintf(replyBuffer, "\"error\":%d,\"seq\":%d", errorNo, sequenceNo);
	}
	else
	{
		sprintf(replyBuffer, "\"error\":%d", errorNo);
	}
	strcat(resultBuffer, replyBuffer);
}

void build_text_value_command_reply(int errorNo, char * result, JsonObject& root, char * resultBuffer)
{
	char replyBuffer[REPLY_ELEMENT_SIZE];

	const char * sequence = root["seq"];

	if (sequence)
	{
		// Got a sequence number in the command - must return the same number
		// so that the sender can identify the command that was sent
		int sequenceNo = root["seq"];
		sprintf(replyBuffer, "\"val\":\"%s\",\"error\":%d,\"seq\":%d", result, errorNo, sequenceNo);
	}
	else
	{
		sprintf(replyBuffer, "\"val\":\"%s\",\"error\":%d", result, errorNo);
	}

	strcat(resultBuffer, replyBuffer);
}

void build_number_value_command_reply(int errorNo, int result, JsonObject& root, char * resultBuffer)
{
	char replyBuffer[REPLY_ELEMENT_SIZE];

	const char * sequence = root["seq"];

	if (sequence)
	{
		// Got a sequence number in the command - must return the same number
		// so that the sender can identify the command that was sent
		int sequenceNo = root["seq"];
		sprintf(replyBuffer, "\"val\":%d,\"error\":%d,\"seq\":%d", result, errorNo, sequenceNo);
	}
	else
	{
		sprintf(replyBuffer, "\"val\":%d,\"error\":%d", result, errorNo);
	}

	strcat(resultBuffer, replyBuffer);
}

void dump_hex(u1_t * pos, int length)
{
	while (length > 0)
	{
		// handle leading zeroes
		if (*pos < 0x10) {
			TRACE("0");
		}
		TRACE_HEX(*pos);
		pos++;
		length--;
	}
	TRACELN();
}

char hex_digit(int val)
{
	if (val < 10)
	{
		return '0' + val;
	}
	else
	{
		return 'A' + (val - 10);
	}
}

void dump_hex_string(char * dest, u1_t * pos, int length)
{
	while (length > 0)
	{
		// handle leading zeroes

		*dest = hex_digit(*pos / 16);
		dest++;
		*dest = hex_digit(*pos % 16);
		dest++;
		pos++;
		length--;
	}
	*dest = 0;
}

void dump_unsigned_long(char * dest, u4_t value)
{
	// Write backwards to put least significant values in
	// the right place

	// move to the end of the string
	int pos = 8;
	// put the terminator in position
	dest[pos] = 0;
	pos--;

	while (pos > 0)
	{
		byte b = value & 0xff;
		dest[pos] = hex_digit(b % 16);
		pos--;
		dest[pos] = hex_digit(b / 16);
		pos--;
		value = value >> 8;
	}
}

void dump_settings()
{
	TRACE("Version: "); TRACELN(settings.version);
	TRACE("Device name: "); TRACELN(settings.deviceNane);
	TRACE("MQTT server: "); TRACELN(settings.mqttServer);
	TRACE("MQTT port: "); TRACELN(settings.mqttPort);
	TRACE("MQTT user: "); TRACELN(settings.mqttUser);
	TRACE("MQTT password: "); TRACELN(settings.mqttPassword);
	TRACE("MQTT device name: "); TRACELN(settings.mqttName);
	TRACE("MQTT publish topic: "); TRACELN(settings.mqttPublishTopic);
	TRACE("MQTT subscribe topic: "); TRACELN(settings.mqttSubscribeTopic);
	TRACE("MQTT secs per update: "); TRACELN(settings.seconds_per_mqtt_update);
	TRACE("MQTT secs per retry: "); TRACELN(settings.seconds_per_mqtt_retry);
	TRACE("MQTT enabled: "); TRACELN(settings.mqtt_enabled);
	TRACE("LoRa enabled: "); TRACELN(settings.lora_enabled);
	TRACE("LoRa Abp Device address: "); TRACE_HEXLN(settings.lora_abp_DEVADDR);
	TRACE("LoRa Abp Network Key: "); dump_hex(settings.lora_abp_NWKSKEY, LORA_KEY_LENGTH);
	TRACE("LoRa Abp App Key: "); dump_hex(settings.lora_abp_APPSKEY, LORA_KEY_LENGTH);

	TRACE("LoRa Otaa App Key: "); dump_hex(settings.lora_otaa_APPKEY, LORA_KEY_LENGTH);
	TRACE("LoRa Otaa Device EUI: "); dump_hex(settings.lora_otaa_DEVEUI, LORA_EUI_LENGTH);
	TRACE("LoRa Otaa App EUI: "); dump_hex(settings.lora_otaa_APPEUI, LORA_EUI_LENGTH);

	TRACE("LoRa ticks per update: "); TRACELN(settings.seconds_per_lora_update);
}

void save_settings()
{
	int addr = SETTINGS_EEPROM_OFFSET;

	void * settingPtr = (void *)&settings;

	EEPROM.writeBytes(addr, settingPtr, sizeof(Device_Settings));
	EEPROM.commit();
}

void load_settings()
{
	int addr = SETTINGS_EEPROM_OFFSET;

	void * settingPtr = (void *)&settings;
	EEPROM.readBytes(addr, settingPtr, sizeof(Device_Settings));
}

int decodeStringValue(char * dest, JsonObject& root, const char * valName, int maxLength)
{
	TRACE("Decoding string value: ");
	TRACELN(valName);

	if (!root[valName].is<char*>())
	{
		TRACELN("Value is missing or not a string");
		return STRING_VALUE_MISSING_OR_NOT_STRING;
	}
	else
	{
		String newVal = root[valName];
		if (newVal.length() > maxLength)
		{
			TRACELN("Value is too long");
			return STRING_VALUE_TOO_LONG;
		}
		newVal.toCharArray(dest, maxLength);
		return WORKED_OK;
	}
}

int decodeNumericValue(int * dest, JsonObject& root, const char * valName, int min, int max)
{
	TRACE("Decoding numeric value: ");
	TRACELN(valName);

	if (!root[valName].is<int>())
	{
		TRACELN("Value is missing or not an integer");
		return NUMERIC_VALUE_NOT_AN_INTEGER;
	}
	else
	{
		int newGapVal = root[valName];
		if (newGapVal < min)
		{
			TRACELN("Value below minimum");
			return NUMERIC_VALUE_BELOW_MINIMUM;
		}
		if (newGapVal > max)
		{
			TRACELN("Value above maximum");
			return NUMERIC_VALUE_ABOVE_MAXIMUM;
		}
		*dest = newGapVal;
		return WORKED_OK;
	}
}

int hexFromChar(char c, int * dest)
{
	if (c >= '0' && c <= '9')
	{
		*dest = (int)(c - '0');
		return WORKED_OK;
	}
	else
	{
		if (c >= 'A' && c <= 'F')
		{
			*dest = (int)(c - 'A' + 10);
			return WORKED_OK;
		}
		else
		{
			if (c >= 'a' && c <= 'f')
			{
				*dest = (int)(c - 'a' + 10);
				return WORKED_OK;
			}
		}
	}
	return INVALID_HEX_DIGIT_IN_VALUE;
}

#define MAX_DECODE_BUFFER_LENGTH 20

int decodeHexValueIntoBytes(u1_t * dest, JsonObject& root, const char * valName, int length)
{
	TRACE("Decoding array of bytes value: ");
	TRACELN(valName);

	if (!root[valName].is<char*>())
	{
		TRACELN("Value is missing or not a string");
		return VALUE_MISSING_OR_NOT_A_STRING;
	}

	if(length> MAX_DECODE_BUFFER_LENGTH)
	{
		TRACELN("Incoming hex value will not fit in the buffer");
		return INCOMING_HEX_VALUE_TOO_BIG_FOR_BUFFER;
	}

	String newVal = root[valName];
	// Each hex value is in two bytes - make sure the incoming text is the right length

	if (newVal.length() != length*2)
	{
		TRACELN("Incoming hex value is the wrong length");
		return INCOMING_HEX_VALUE_IS_THE_WRONG_LENGTH;
	}

	int pos = 0;

	u1_t buffer[MAX_DECODE_BUFFER_LENGTH];
	u1_t * bpos = buffer;

	while (pos < newVal.length())
	{
		int d1, d2, reply;

		reply = hexFromChar(newVal[pos], &d1);
		if (reply != WORKED_OK)
			return reply;
		pos++;
		reply = hexFromChar(newVal[pos], &d2);
		if (reply != WORKED_OK)
			return reply;
		pos++;

		*bpos = (u1_t)(d1 * 16 + d2);
		bpos++;
	}

	// If we get here the buffer has been filled OK

	memcpy_P(dest, buffer, length);
	return WORKED_OK;
}

int decodeHexValueIntoUnsignedLong(u4_t * dest, JsonObject& root, const char * valName)
{
	TRACE("Decoding unsigned long value: ");
	TRACELN(valName);

	if (!root[valName].is<char*>())
	{
		TRACELN("Value is missing or not a string");
		return VALUE_MISSING_OR_NOT_A_STRING;
	}
	// Each hex value is in two bytes - make sure the incoming text is the right length

	String newVal = root[valName];
	if (newVal.length() != 8)
	{
		TRACELN("Incoming hex value is the wrong length");
		return INCOMING_HEX_VALUE_IS_THE_WRONG_LENGTH;
	}

	int pos = 0;

	u4_t result = 0;

	while (pos < newVal.length())
	{
		int d1, d2, reply;

		reply = hexFromChar(newVal[pos], &d1);
		if (reply != WORKED_OK)
			return reply;
		pos++;
		reply = hexFromChar(newVal[pos], &d2);
		if (reply != WORKED_OK)
			return reply;
		pos++;

		u4_t v = d1 * 16 + d2;
		result = result * 256 + v;
	}

	// If we get here the value has been received OK

	*dest = result;
	return WORKED_OK;
}

int checkTargetDeviceName(JsonObject& root)
{
	const char * target = root["t"];

	if (!target)
	{
		TRACELN("Invalid or missing target in received command");
		return INVALID_OR_MISSING_TARGET_IN_RECEIVED_COMMAND;
	}

	if (strcasecmp(target, settings.deviceNane))
	{
		TRACE("Command for different target: ");
		TRACELN(target);
		return COMMAND_FOR_DIFFERENT_TARGET;
	}

	TRACELN("Doing a command for this device");
	return WORKED_OK;
}

// request: {"v":1, "t" : "Sensor01", "c" : "mqtt", "o" : "send"}
void do_send_mqtt(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		pub_mqtt_force_send = true;
	}
	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "Sensor01", "c" : "mqtt", "o" : "state", "v" : "on"}
// {"v":1, "t" : "Sensor01", "c" : "mqtt", "o" : "state", "v" : "off"}
void do_mqtt_state(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];
		if (!option)
		{
			// no option - just a status request
			if (settings.mqtt_enabled)
			{
				build_text_value_command_reply(WORKED_OK, "on", root, resultBuffer);
			}
			else
			{
				build_text_value_command_reply(WORKED_OK, "off", root, resultBuffer);
			}
			return;
		}

		if (!root["val"].is<char*>())
		{
			TRACELN("Value is missing or not a string");
			reply = INVALID_MQTT_STATUS_SETTING;
		}
		else
		{
			const char * option = root["val"];
			if (strcasecmp(option, "on") == 0)
			{
				settings.mqtt_enabled = true;
			}
			else
			{
				if (strcasecmp(option, "off") == 0)
				{
					settings.mqtt_enabled = false;
				}
				else
				{
					reply = INVALID_MQTT_STATUS_SETTING;
				}
			}
		}
	}

	if (reply == WORKED_OK)
		save_settings();

	build_command_reply(reply, root, resultBuffer);
}

#define MIN_MQTT_GAP 1
#define MAX_MQTT_GAP 30000

// {"v":1, "t" : "sensor01", "c" : "mqtt", "o" : "gap", "val":20}
void do_mqtt_gap(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_number_value_command_reply(WORKED_OK, settings.seconds_per_mqtt_update, root, resultBuffer);
			return;
		}

		reply = decodeNumericValue(&settings.seconds_per_mqtt_update, root, "val", MIN_MQTT_GAP, MAX_MQTT_GAP);

		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	setup_timing();

	build_command_reply(reply, root, resultBuffer);
}

#define MIN_MQTT_RETRY 1
#define MAX_MQTT_RETRY 30000

// {"v":1, "t" : "sensor01", "c" : "mqtt", "o" : "retry", "val":20}
void do_mqtt_retry(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_number_value_command_reply(WORKED_OK, settings.seconds_per_mqtt_retry, root, resultBuffer);
			return;
		}

		reply = decodeNumericValue(&settings.seconds_per_mqtt_retry, root, "val", MIN_MQTT_RETRY, MAX_MQTT_RETRY);

		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "sensor01", "c" : "mqtt", "o" : "publish", "val":"sensor01/data" }
void do_mqtt_publish_location(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_text_value_command_reply (WORKED_OK, settings.mqttPublishTopic, root, resultBuffer);
			return;
		}

		reply = decodeStringValue(settings.mqttPublishTopic, root, "val", MQTT_PUBLISH_TOPIC_LENGTH - 1);

		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "sensor01", "c" : "mqtt", "o" : "subscribe", "val":"sensor01/commands" }
void do_mqtt_subscribe_location(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_text_value_command_reply(WORKED_OK, settings.mqttSubscribeTopic, root, resultBuffer);
			return;
		}

		reply = decodeStringValue(settings.mqttSubscribeTopic, root, "val", MQTT_SUBSCRIBE_TOPIC_LENGTH - 1);
		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "sensor01", "c" : "mqtt", "o" : "id", "val":"sensor01" }
void do_mqtt_device_id(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_text_value_command_reply(WORKED_OK, settings.mqttName, root, resultBuffer);
			return;
		}

		reply = decodeStringValue(settings.mqttName, root, "val", MQTT_DEVICE_NAME_LENGTH - 1);
		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}


// {"v":1, "t" : "sensor01", "c" : "mqtt", "o" : "host", "val":"mqtt.connectedhumber.org" }
void do_mqtt_host(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_text_value_command_reply(WORKED_OK, settings.mqttServer, root, resultBuffer);
			return;
		}

		reply = decodeStringValue(settings.mqttServer, root, "val", MQTT_SERVER_NAME_LENGTH - 1);
		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}


// {"v":1, "t" : "sensor01", "c" : "mqtt", "o" : "user", "val":"username" }
void do_mqtt_user(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_text_value_command_reply(WORKED_OK, settings.mqttUser, root, resultBuffer);
			return;
		}

		reply = decodeStringValue(settings.mqttUser, root, "val", MQTT_USER_NAME_LENGTH - 1);
		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "sensor01", "c" : "mqtt", "o" : "pwd", "val":"123456" }
void do_mqtt_password(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_text_value_command_reply(WORKED_OK, settings.mqttPassword, root, resultBuffer);
			return;
		}

		reply = decodeStringValue(settings.mqttPassword, root, "val", MQTT_PASSWORD_LENGTH - 1);
		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "sensor01", "c" : "mqtt", "o" : "port", "val":1883 }
void do_mqtt_port(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_number_value_command_reply(WORKED_OK, settings.mqttPort, root, resultBuffer);
			return;
		}

		reply = decodeNumericValue(&settings.mqttPort, root, "val", 0, 9999);
		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

StaticJsonBuffer<300> jsonBuffer;

struct OptionDecodeItems
{
	const char * optionName;
	void (*optionAction) (JsonObject& root, char * resultBuffer);
};

OptionDecodeItems mqttOptionDecodeItems[] = {
	{"state", do_mqtt_state},
	{"send", do_send_mqtt},
	{"gap", do_mqtt_gap},
	{"retry", do_mqtt_retry},
	{"publish", do_mqtt_publish_location},
	{"subscribe", do_mqtt_subscribe_location},
	{"id", do_mqtt_device_id},
	{"user", do_mqtt_user},
	{"pwd", do_mqtt_password},
	{"host", do_mqtt_host},
	{"port", do_mqtt_port}
};

// {"v":1, "t" : "Sensor01", "c" : "lora", "o" : "send"}
void do_send_lora(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		pub_lora_force_send = true;
	}
	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "Sensor01", "c" : "lora", "o" : "test"}
void do_test_lora(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		pub_lora_force_test = true;
	}
	build_command_reply(reply, root, resultBuffer);
}


// {"v":1, "t" : "Sensor01", "c" : "lora", "o" : "state", "val" : "on"}
// {"v":1, "t" : "Sensor01", "c" : "lora", "o" : "state", "val" : "off"}
void do_lora_state(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];
		if (!option)
		{
			// no option - just a status request
			if (settings.lora_enabled)
			{
				build_text_value_command_reply(WORKED_OK, "on", root, resultBuffer);
			}
			else
			{
				build_text_value_command_reply(WORKED_OK, "off", root, resultBuffer);
			}
			return;
		}

		if (!root["val"].is<char*>())
		{
			TRACELN("Value is missing or not a string");
			reply = INVALID_LORA_STATUS_SETTING;
		}
		else
		{
			const char * option = root["val"];
			if (strcasecmp(option, "on") == 0)
			{
				settings.lora_enabled = true;
			}
			else
			{
				if (strcasecmp(option, "off") == 0)
				{
					settings.lora_enabled = false;
				}
				else
				{
					reply = INVALID_LORA_STATUS_SETTING;
				}
			}
		}
	}

	if (reply == WORKED_OK)
		save_settings();

	build_command_reply(reply, root, resultBuffer);
}


#define MIN_LORA_GAP 30
#define MAX_LORA_GAP 30000

// {"v":1, "t" : "sensor01", "c" : "lora", "o" : "gap", "val":20}
void do_lora_gap(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_number_value_command_reply(WORKED_OK, settings.seconds_per_lora_update, root, resultBuffer);
			return;
		}

		reply = decodeNumericValue(&settings.seconds_per_lora_update, root, "val", MIN_LORA_GAP, MAX_LORA_GAP);
		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	setup_timing();
	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "sensor01", "c" : "lora", "o" : "abpapp", "val":"AD77768C296F2CA5C4C03E85862AE688"}
void do_lora_abp_app_key(JsonObject& root,char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			char hex_buffer[100];
			dump_hex_string(hex_buffer, settings.lora_abp_APPSKEY, LORA_KEY_LENGTH);

			build_text_value_command_reply(WORKED_OK, hex_buffer, root, resultBuffer);
			return;
		}

		reply = decodeHexValueIntoBytes(settings.lora_abp_APPSKEY, root, "val", LORA_KEY_LENGTH);
		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "sensor01", "c" : "lora", "o" : "abpnwk", "val":"117480D907331B077C5DAE9777FFE43C"}
void do_lora_abp_nwk_key(JsonObject& root, char* resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			char hex_buffer[100];
			dump_hex_string(hex_buffer, settings.lora_abp_NWKSKEY, LORA_KEY_LENGTH);

			build_text_value_command_reply(WORKED_OK, hex_buffer, root, resultBuffer);
			return;
		}

		reply = decodeHexValueIntoBytes(settings.lora_abp_NWKSKEY, root, "val", LORA_KEY_LENGTH);
		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "sensor01", "c" : "lora", "o" : "abpdev", "val":"26011BEE"}
void do_lora_abp_device_id(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			char hex_buffer[100];
			dump_unsigned_long(hex_buffer, settings.lora_abp_DEVADDR);

			build_text_value_command_reply(WORKED_OK, hex_buffer, root, resultBuffer);
			return;
		}

		reply = decodeHexValueIntoUnsignedLong(&settings.lora_abp_DEVADDR, root, "val");
		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "Sensor01", "c" : "lora", "o" : "access", "val":"abp"}
// {"v":1, "t" : "Sensor01", "c" : "lora", "o" : "access", "val":"otaa"}
void do_lora_access(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];
		if (!option)
		{
			// no option - just a status request
			if (settings.lora_abp_active)
			{
				build_text_value_command_reply(WORKED_OK, "abp", root, resultBuffer);
			}
			else
			{
				build_text_value_command_reply(WORKED_OK, "otaa", root, resultBuffer);
			}
			return;
		}

		if (!root["val"].is<char*>())
		{
			TRACELN("Value is missing or not a string");
			reply = INVALID_LORA_ACCESS_SETTING;
		}
		else
		{
			const char * option = root["val"];
			if (strcasecmp(option, "abp") == 0)
			{
				settings.lora_abp_active = true;
			}
			else
			{
				if (strcasecmp(option, "otaa") == 0)
				{
					settings.lora_abp_active = false;
				}
				else
				{
					reply = INVALID_LORA_ACCESS_SETTING;
				}
			}
		}
	}

	if (reply == WORKED_OK)
		save_settings();

	build_command_reply(reply, root, resultBuffer);
}

OptionDecodeItems loraOptionDecodeItems[] = {
	{"state", do_lora_state},
	{"send", do_send_lora},
	{"test", do_test_lora},
	{"gap", do_lora_gap},
	{"abpapp", do_lora_abp_app_key},
	{"abpnwk", do_lora_abp_nwk_key},
	{"abpdev", do_lora_abp_device_id},
	{"access", do_lora_access}
};

// forward declaration - function in WiFiConnection.h

void start_wifi();

// {"v":1, "t" : "Sensor01", "c" : "wifi", "o" : "on"}
void do_wifi_on(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		TRACELN("Starting WiFi");
		start_wifi();
	}
	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "Sensor01", "c" : "wifi", "o" : "off"}
void do_wifi_off(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		TRACELN("Stopping WiFi");
		// not sure how to do this just yet...
	}
	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "sensor01", "c" : "wifi",  "o" : "ssid", "set":0, "val":"ssid"}
void do_wifi_ssid(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		if (!root["set"].is<int>())
		{
			TRACELN("Missing WiFi setting number");
			reply = MISSING_WIFI_SETTING_NUMBER;
		}
		else
		{
			int settingNo = root["set"];
			if (settingNo<0 || settingNo >= NO_OF_WIFI_SETTINGS)
			{
				reply = INVALID_WIFI_SETTING_NUMBER;
			}
			else
			{
				const char * option = root["val"];

				if (!option)
				{
					build_text_value_command_reply(WORKED_OK, settings.wifiSettings[settingNo].wifiSsid, 
						root, resultBuffer);
					return;
				}

				reply = decodeStringValue(settings.wifiSettings[settingNo].wifiSsid, root,
					"val", WIFI_PASSWORD_LENGTH - 1);
			}

			if (reply == WORKED_OK)
			{
				save_settings();
			}
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "sensor01", "c" : "wifi", "o" : "pwd", "set":0, "val":"password"}
void do_wifi_password(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		if (!root["set"].is<int>())
		{
			TRACELN("Missing WiFi setting number");
			reply = MISSING_WIFI_SETTING_NUMBER;
		}
		else 
		{
			int settingNo = root["set"];
			if (settingNo<0 || settingNo>=NO_OF_WIFI_SETTINGS)
			{
				reply = INVALID_WIFI_SETTING_NUMBER;
			}
			else
			{
				reply = decodeStringValue(settings.wifiSettings[settingNo].wifiPassword, root, "val", WIFI_PASSWORD_LENGTH - 1);
			}

			if (reply == WORKED_OK)
			{
				save_settings();
			}
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

OptionDecodeItems WiFiOptionDecodeItems[] = {
	{"on", do_wifi_on},
	{"off", do_wifi_off},
	{"ssid", do_wifi_ssid},
	{"pwd", do_wifi_password}
};

// {"v":1, "c" : "node", "o" : "ver"}
void do_node_version(JsonObject& root, char * resultBuffer)
{
	TRACELN("Getting version");
	int length = sprintf(resultBuffer, "{\"version\":%d,", settings.version);


	build_command_reply(WORKED_OK, root, resultBuffer + length);
}

// {"v":1, "c" : "node", "o" : "getdevname"}
void do_get_device_name(JsonObject& root, char * resultBuffer)
{
	TRACELN("Getting device name");

	int length = sprintf(resultBuffer, "{\"nodename\":\"%s\",", settings.deviceNane);
	build_command_reply(WORKED_OK, root, resultBuffer + length);
}

// {"v":1, "t" : "sensor01", "c" : "node", "o" : "reset"}
void do_reset(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		ESP.restart();
	}
}

// {"v":1, "t" : "sensor01", "c" : "node", "o" : "devname", "val":"sensor01"}
void do_device_name(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_text_value_command_reply(WORKED_OK, settings.deviceNane, root, resultBuffer);
			return;
		}

		reply = decodeStringValue(settings.deviceNane, root, "val", DEVICE_NAME_LENGTH - 1);
		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

// declared in menu.h and used to refresh the menu display
// Allows for changes in the splash screen to be reflected instantly 
// on arrival. 

void refresh_menu();

// {"v":1, "t" : "sensor01", "c" : "node", "o" : "spltop", "val":"Connected"}
void do_splash_screen_top_line(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_text_value_command_reply(WORKED_OK, settings.splash_screen_top_line, root, resultBuffer);
			return;
		}

		reply = decodeStringValue(settings.splash_screen_top_line, root, "val", SPLASH_LINE_LENGTH - 1);
		if (reply == WORKED_OK)
		{
			save_settings();
			refresh_menu();		
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "sensor01", "c" : "node", "o" : "splbtm", "val":"Humber"}
void do_splash_screen_bottom_line(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_text_value_command_reply(WORKED_OK, settings.splash_screen_top_line, root, resultBuffer);
			return;
		}

		reply = decodeStringValue(settings.splash_screen_bottom_line, root, "val", SPLASH_LINE_LENGTH - 1);
		if (reply == WORKED_OK)
		{
			save_settings();
			refresh_menu();		
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

#define COLOUR_NAME_LENGTH 10

// {"v":1, "t" : "sensor01", "c" : "node", "o" : "pixel", "val":"red"}
void do_pixel_colour(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);
	char colour_name[COLOUR_NAME_LENGTH];


	if (reply == WORKED_OK)
	{
		reply = decodeStringValue(colour_name, root, "val", COLOUR_NAME_LENGTH - 1);
		if (reply == WORKED_OK)
		{
			save_settings();
			refresh_menu();
		}
	}

	if (strcasecmp(colour_name, "red") == 0)
	{
		safe_r = 255;
		safe_g = 0;
		safe_b = 0;
	}

	if (strcasecmp(colour_name, "green") == 0)
	{
		safe_r = 0;
		safe_g = 255;
		safe_b = 0;
	}

	if (strcasecmp(colour_name, "blue") == 0)
	{
		safe_r = 0;
		safe_g = 0;
		safe_b = 255;
	}

	if (strcasecmp(colour_name, "black") == 0)
	{
		safe_r = 0;
		safe_g = 0;
		safe_b = 0;
	}

	build_command_reply(reply, root, resultBuffer);
}

#define MIN_POWER_UP 5
#define MAX_POWER_UP 60000

// {"v":1, "t" : "sensor01", "c" : "node", "o" : "warmup", "val":30}
void do_sensor_warm_up_time(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_number_value_command_reply(WORKED_OK, settings.seconds_sensor_warmup, root, resultBuffer);
			return;
		}

		reply = decodeNumericValue(&settings.seconds_sensor_warmup, root, "val", MIN_POWER_UP, MAX_POWER_UP);

		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	setup_timing();

	build_command_reply(reply, root, resultBuffer);
}

void do_sensor_status(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_number_value_command_reply(WORKED_OK, settings.seconds_sensor_warmup, root, resultBuffer);
			return;
		}

		reply = decodeNumericValue(&settings.seconds_sensor_warmup, root, "val", MIN_POWER_UP, MAX_POWER_UP);

		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	setup_timing();

	build_command_reply(reply, root, resultBuffer);
}



OptionDecodeItems nodeOptionDecodeItems[] = {
	{"ver", do_node_version},
	{"getdevname", do_get_device_name},
	{"reset", do_reset },
	{"devname", do_device_name},
	{"spltop", do_splash_screen_top_line },
	{"splbtm", do_splash_screen_bottom_line },
	{"pixel", do_pixel_colour },
    {"warmup", do_sensor_warm_up_time}
};

struct CommandDecoder
{
	const char * commandName;
	OptionDecodeItems * items;
	int noOfOptions;
};


struct CommandDecoder commandDecoder[] =
{
	{"node", nodeOptionDecodeItems, sizeof(nodeOptionDecodeItems) / sizeof(OptionDecodeItems)},
	{"mqtt", mqttOptionDecodeItems, sizeof(mqttOptionDecodeItems) / sizeof(OptionDecodeItems)},
	{"lora", loraOptionDecodeItems, sizeof(loraOptionDecodeItems) / sizeof(OptionDecodeItems)},
	{"wifi", WiFiOptionDecodeItems, sizeof(WiFiOptionDecodeItems) / sizeof(OptionDecodeItems)}
};

void actOnCommand(const char * command, const char * option, JsonObject& root, char * resultBuffer)
{
	bool foundCommand = false;

	for (int commandNo = 0; commandNo < sizeof(commandDecoder) / sizeof(CommandDecoder); commandNo++)
	{
		if (strcasecmp(command, commandDecoder[commandNo].commandName) == 0)
		{
			TRACE("Performing command: ");
			TRACELN(commandDecoder[commandNo].commandName);

			OptionDecodeItems * items = commandDecoder[commandNo].items;

			for (int optionNo = 0; optionNo < commandDecoder[commandNo].noOfOptions; optionNo++)
			{
				if (strcasecmp(option, items[optionNo].optionName) == 0)
				{
					TRACE("    Performing option: ");
					TRACELN(items[optionNo].optionName);
					items[optionNo].optionAction(root, command_reply_buffer);
					foundCommand = true;
				}
			}
		}
	}

	if (foundCommand)
	{
		TRACELN("Command found and performed.");
	}
	else
	{
		build_command_reply(INVALID_COMMAND_NAME, root, resultBuffer);
		TRACELN("Command not found");
	}
}


void abort_json_command(int error, JsonObject& root, void(*deliverResult) (char * resultText))
{
	build_command_reply(error, root, command_reply_buffer);
	// append the version number to the invalid command message
	strcat(command_reply_buffer, ",\"v\":1}");
	deliverResult(command_reply_buffer);
}

void act_onJson_command(char * json, void(*deliverResult) (char * resultText))
{
	command_reply_buffer[0] = 0;

	strcat(command_reply_buffer, "{");

	TRACE("Received command:");
	TRACELN(json);

	// Clear any previous elements from the buffer

	jsonBuffer.clear();

	JsonObject& root = jsonBuffer.parseObject(json);

	if(!root.success())
	{
		TRACELN("JSON could not be parsed");
		abort_json_command(JSON_COMMAND_COULD_NOT_BE_PARSED, root, deliverResult);
		return;
	}
	
	int v = root["v"];

	if (v != settings.version)
	{
		TRACELN("Invalid or missing version in received command");
		TRACELN(v);
		abort_json_command(JSON_COMMAND_MISSING_VERSION, root, deliverResult);
		return;
	}

	const char * command = root["c"];

	if (!command)
	{
		TRACELN("Missing command");
		abort_json_command(JSON_COMMAND_MISSING_COMMAND, root, deliverResult);
		return;
	}

	TRACE("Received command: ");
	TRACELN(command);

	const char * option = root["o"];

	if (!option)
	{
		TRACELN("Missing option");
		TRACELN(v);
		abort_json_command(JSON_COMMAND_MISSING_OPTION, root, deliverResult);
		return;
	}

	TRACE("Received option: ");
	TRACELN(option);

	actOnCommand(command, option, root, command_reply_buffer);
	strcat(command_reply_buffer, "}");
	deliverResult(command_reply_buffer);
}

#define CHECK_BYTE_O1 0xAA
#define CHECK_BYTE_O2 0x55

boolean valid_stored_settings()
{
	return (settings.checkByte1 == CHECK_BYTE_O1 && settings.checkByte2 == CHECK_BYTE_O2);
}

static const u4_t abp_DEVADDR = 0x26011BEE; // <-- Change this address for every node!

										// LoRaWAN NwkSKey, network session key
										// Copy this from the Network Session Key entry on TTN properties for the 
										// node. Use the msb ordering
static const PROGMEM u1_t abp_NWKSKEY[16] = { 0x11, 0x74, 0x80, 0xD9, 0x07, 0x33, 0x1B, 0x07, 0x7C, 0x5D, 0xAE, 0x97, 0x77, 0xFF, 0xE4, 0x3C };

// LoRaWAN AppSKey, application session key
// Copy this from the App Session Key on TTN properties for the 
// node. Use the msb ordering
static const u1_t PROGMEM abp_APPSKEY[16] = { 0xAD, 0x77, 0x76, 0x8C, 0x29, 0x6F, 0x2C, 0xA5, 0xC4, 0xC0, 0x3E, 0x85, 0x86, 0x2A, 0xE6, 0x88 };


static const u1_t otaa_DEVEUI[8] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const u1_t otaa_APPEUI[8] = { 0x03, 0x1D, 0x01, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
// The key shown here is the semtech default key.
static const u1_t otaa_APPKEY[16] =
{ 0x59, 0x1A, 0x3B, 0xD4, 0x40, 0x74, 0x69, 0xB5, 0xDA, 0xC1, 0xEF, 0xAD, 0x9D, 0x82, 0x8F, 0x7D };

void reset_settings()
{
	settings.version = 1;
	settings.checkByte1 = CHECK_BYTE_O1;
	settings.checkByte2 = CHECK_BYTE_O2;

	strcpy(settings.deviceNane, "sensor01");

	settings.wiFiOn = true;

	// enter some pre-set WiFi locations
	// enter some pre-set WiFi locations
	strcpy(settings.wifiSettings[0].wifiSsid, "sdfsdf");
	strcpy(settings.wifiSettings[0].wifiPassword, "sdfsd");

	strcpy(settings.wifiSettings[1].wifiSsid, "sdfdfdf");
	strcpy(settings.wifiSettings[1].wifiPassword, "fdfsddf");

	strcpy(settings.wifiSettings[2].wifiSsid, "dsdsdffd");
	strcpy(settings.wifiSettings[2].wifiPassword, "fdsffdsf");

	// clear the rest of the settings
	for (int i = 3; i < NO_OF_WIFI_SETTINGS; i++)
	{
		settings.wifiSettings[i].wifiSsid[0] = 0;
		settings.wifiSettings[i].wifiPassword[0] = 0;
	}

	strcpy(settings.splash_screen_top_line, "Connected");
	strcpy(settings.splash_screen_bottom_line, "Humber");

#ifdef SECURE_SOCKETS

	strcpy(settings.mqttServer, "mydevice.azure-devices.net");
	settings.mqttPort = 8883;
	strcpy(settings.mqttName, "MQTTMini01");
	strcpy(settings.mqttUser, "mydevice.azure-devices.net/MQTTMini01");
	strcpy(settings.mqttPassword, "SharedAccessSignature sr=mydevice.azure-devices.net%2Fdevices%2FMQTTMini01&sig=sdfsddsdsffsdfsdfssfSFcRG%2BaeE%3D&se=1568130534");

	strcpy(settings.mqttPublishTopic, "devices/MQTTMini01/messages/events/");
	strcpy(settings.mqttSubscribeTopic, "devices/MQTTMini01/messages/devicebound/#");

#else

	strcpy(settings.mqttServer, "mqtt.connectedhumber.org");
	settings.mqttPort = 1883;
	strcpy(settings.mqttUser, "sdfsdf");
	strcpy(settings.mqttPassword, "sdfsdsdf");
	strcpy(settings.mqttName, "Sensor01");
	strcpy(settings.mqttPublishTopic, "sensor01/datazump");
	strcpy(settings.mqttSubscribeTopic, "sensor01/commands");

#endif

	settings.lora_enabled = true;
	settings.mqtt_enabled = true;
	settings.seconds_per_lora_update = 60;
	settings.seconds_per_mqtt_update = 10;
	settings.seconds_per_mqtt_retry = 1;
	settings.seconds_sensor_warmup = 30;

	settings.lora_abp_active = false;

	// Copy these constants from lora_apb.h into the settings array

	settings.lora_abp_DEVADDR = 0x26011BEE;

	memcpy_P(settings.lora_abp_APPSKEY, abp_APPSKEY, sizeof(abp_APPSKEY));
	memcpy_P(settings.lora_abp_NWKSKEY, abp_NWKSKEY, sizeof(abp_NWKSKEY));

	memcpy_P(settings.lora_otaa_APPKEY, otaa_APPKEY, sizeof(otaa_APPKEY));
	memcpy_P(settings.lora_otaa_DEVEUI, otaa_DEVEUI, sizeof(otaa_DEVEUI));
	memcpy_P(settings.lora_otaa_APPEUI, otaa_APPEUI, sizeof(otaa_APPEUI));

}

void serial_deliver_command_result(char * result)
{
	Serial.println(result);
}

#define SERIAL_BUFFER_SIZE 240
#define SERIAL_BUFFER_LIMIT SERIAL_BUFFER_SIZE-1

char serial_receive_buffer[SERIAL_BUFFER_SIZE];

int serial_receive_buffer_pos = 0;

void reset_serial_buffer()
{
	serial_receive_buffer_pos = 0;
}

void act_on_serial_command()
{
	act_onJson_command(serial_receive_buffer, serial_deliver_command_result);
}

void buffer_char(char ch)
{
	if ((serial_receive_buffer_pos > 0) &&
		(ch == '\n' || ch == '\r' || ch == 0))
	{
		// terminate the received string
		serial_receive_buffer[serial_receive_buffer_pos] = 0;
		act_on_serial_command();
		reset_serial_buffer();
	}
	else
	{
		if (serial_receive_buffer_pos < SERIAL_BUFFER_SIZE)
		{
			serial_receive_buffer[serial_receive_buffer_pos] = ch;
			serial_receive_buffer_pos++;
		}
	}
}


#define CHAR_FLUSH_START_TIMEOUT 1000

unsigned long character_timer_start;

void check_serial_buffer()
{
	if (Serial.available())
	{
		character_timer_start = millis();
		while (Serial.available())
		{
			buffer_char(Serial.read());
		}
	}
	else
	{
		if (serial_receive_buffer_pos > 0)
		{
			// have got some characters - if they've been here a while - discard them
			unsigned long elapsed_time = millis() - character_timer_start;
			if (elapsed_time > CHAR_FLUSH_START_TIMEOUT)
			{
				reset_serial_buffer();
			}
		}
	}
}

void error_stop(String title, String text);

void setup_commands()
{
	if (!EEPROM.begin(EEPROM_SIZE))
	{
		error_stop("Failure", "EEPROM faulty");
	}

	load_settings();

	if (!valid_stored_settings())
	{
		TRACELN("Stored settings reset");
		reset_settings();
		save_settings();
	}
	else
	{
		TRACELN("Settings loaded OK");
	}
	dump_settings();
}



// Checks the serial port for any commands and acts on them

void loop_commands()
{
	check_serial_buffer();
}
