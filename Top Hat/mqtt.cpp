#include <Arduino.h>
#include <DNSServer.h>
#include <WiFiUdp.h>
#include <WiFiServerSecureBearSSL.h>
#include <WiFiServerSecureAxTLS.h>
#include <WiFiServerSecure.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <ESP8266WiFiType.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFi.h>

#include "utils.h"
#include "processes.h"
#include "settings.h"

#include "mqtt.h"

#include <PubSubClient.h>

unsigned long mqtt_timer_start;

Client * espClient = NULL;

PubSubClient * mqttPubSubClient = NULL;

#define MQTT_RECEIVE_BUFFER_SIZE 240
char mqtt_receive_buffer[MQTT_RECEIVE_BUFFER_SIZE];

#define MQTT_SEND_BUFFER_SIZE 240

char mqtt_send_buffer[MQTT_SEND_BUFFER_SIZE];

boolean first_mqtt_message = true;

int messagesSent;
int messagesReceived;

void callback(char* topic, byte* payload, unsigned int length)
{
	int i;
	for (i = 0; i < length; i++) {
		mqtt_receive_buffer[i] = (char)payload[i];
	}

	// Put the terminator on the string
	mqtt_receive_buffer[i] = 0;
	// Now do something with the string....
	Serial.printf("Received from MQTT: %s\n", mqtt_receive_buffer);

	messagesReceived++;
}

int mqttConnectErrorNumber;

struct process * mqttWiFiProcess = NULL;
struct process * activeMQTTProcess;

int startMQTT(struct process * mqttProcess)
{
	activeMQTTProcess = mqttProcess;

	if (mqttWiFiProcess == NULL)
	{
		mqttWiFiProcess = findProcessByName("WiFi");
	}

	mqttProcess->status = MQTT_STARTING;

	return mqttProcess->status;
}

int restartMQTT()
{
	messagesReceived = 0;
	messagesSent = 0;

	if (mqttWiFiProcess->status != WIFI_OK)
	{
		activeMQTTProcess->status = MQTT_ERROR_NO_WIFI;
		return MQTT_ERROR_NO_WIFI;
	}

	if (espClient == NULL)
	{
		if (settings.mqttSecureSockets)
		{
			espClient = new WiFiClientSecure();
		}
		else
		{
			espClient = new WiFiClient();
		}

		mqttPubSubClient = new PubSubClient(*espClient);

		mqttPubSubClient->setServer(settings.mqttServer, settings.mqttPort);
		mqttPubSubClient->setCallback(callback);
	}

	if (!mqttPubSubClient->connect(settings.deviceName, settings.mqttUser, settings.mqttPassword))
	{
		switch (mqttPubSubClient->state())
		{
		case MQTT_CONNECT_BAD_PROTOCOL:
			activeMQTTProcess->status = MQTT_ERROR_BAD_PROTOCOL;
			break;
		case MQTT_CONNECT_BAD_CLIENT_ID:
			activeMQTTProcess->status = MQTT_ERROR_BAD_CLIENT_ID;
			break;
		case MQTT_CONNECT_UNAVAILABLE:
			activeMQTTProcess->status = MQTT_ERROR_CONNECT_UNAVAILABLE;
			break;
		case MQTT_CONNECT_BAD_CREDENTIALS:
			activeMQTTProcess->status = MQTT_ERROR_BAD_CREDENTIALS;
			break;
		case MQTT_CONNECT_UNAUTHORIZED:
			activeMQTTProcess->status = MQTT_ERROR_CONNECT_UNAUTHORIZED;
			break;
		case MQTT_CONNECTION_TIMEOUT:
			activeMQTTProcess->status = MQTT_ERROR_BAD_PROTOCOL;
			break;
		case MQTT_CONNECT_FAILED:
			activeMQTTProcess->status = MQTT_ERROR_CONNECT_FAILED;
			break;
		default:
			mqttConnectErrorNumber = mqttPubSubClient->state();
			activeMQTTProcess->status = MQTT_ERROR_CONNECT_ERROR;
			break;
		}
		return activeMQTTProcess->status;
	}

	mqttPubSubClient->subscribe(settings.mqttSubscribeTopic);

	//snprintf(mqtt_send_buffer, MQTT_SEND_BUFFER_SIZE,
	//	"{\"dev\":\"%s\", \"status\":\"starting\"}",
	//	settings.deviceName);

	//if (!mqttPubSubClient->publish(settings.mqttReportTopic, mqtt_send_buffer))
	//{
	//	Serial.println("publish failed");
	//	mqttProcess->status = MQTT_ERROR_CONNECT_MESSAGE_FAILED;
	//	return MQTT_ERROR_CONNECT_MESSAGE_FAILED;
	//}

	activeMQTTProcess->status = MQTT_OK;
	return MQTT_OK;
}

boolean publishReadingsToMQTT(char * buffer)
{
	if (activeMQTTProcess->status == MQTT_OK)
	{
		messagesSent++;
		Serial.println("Publishing message");
		return mqttPubSubClient->publish(settings.mqttPublishTopic, buffer);
	}

	Serial.println("Not publishing message");

	return false;
}

int stopMQTT(struct process * mqttProcess)
{
	mqttProcess->status = MQTT_OFF;
	return MQTT_OFF;
}

unsigned long timeOfLastMQTTsuccess = 0;


int updateMQTT(struct process * mqttProcess)
{
	switch (mqttProcess->status)
	{

	case MQTT_OK:

		if (WiFi.status() != WL_CONNECTED)
		{
			mqttProcess->status = MQTT_ERROR_NO_WIFI;
			mqttPubSubClient->disconnect();
		}

		timeOfLastMQTTsuccess = millis();

		if (!mqttPubSubClient->loop())
		{
			mqttPubSubClient->disconnect();
			mqttProcess->status = MQTT_ERROR_LOOP_FAILED;
		}

		break;

	case MQTT_OFF:
		break;

	case MQTT_STARTING:
		mqttProcess->status = restartMQTT();
		break;

	case MQTT_ERROR_NO_WIFI:
		if (mqttWiFiProcess->status == WIFI_OK)
		{
			mqttProcess->status = restartMQTT();
		}
		break;

	case MQTT_ERROR_BAD_PROTOCOL:
	case MQTT_ERROR_BAD_CLIENT_ID:
	case MQTT_ERROR_CONNECT_UNAVAILABLE:
	case MQTT_ERROR_BAD_CREDENTIALS:
	case MQTT_ERROR_CONNECT_UNAUTHORIZED:
	case MQTT_ERROR_CONNECT_FAILED:
	case MQTT_ERROR_CONNECT_ERROR:
	case MQTT_ERROR_CONNECT_MESSAGE_FAILED:
	case MQTT_ERROR_LOOP_FAILED:
		if (ulongDiff(millis(), timeOfLastMQTTsuccess) > MQTT_CONNECT_RETRY_INTERVAL_MSECS)
		{
			mqttProcess->status = restartMQTT();
			timeOfLastMQTTsuccess = millis();
		}
		break;

	default:
		break;
	}

	return mqttProcess->status;
}

void mqttStatusMessage(struct process * mqttProcess, char * buffer, int bufferLength)
{
	switch (mqttProcess->status)
	{
	case MQTT_OK:
		snprintf(buffer, bufferLength, "MQTT OK sent: %d rec: %d", messagesSent, messagesReceived);
		break;
	case MQTT_STARTING:
		snprintf(buffer, bufferLength, "MQTT Starting");
		break;
	case MQTT_OFF:
		snprintf(buffer, bufferLength, "MQTT OFF");
		break;
	case MQTT_ERROR_NO_WIFI:
		snprintf(buffer, bufferLength, "MQTT error no WiFi");
		break;
	case MQTT_ERROR_BAD_PROTOCOL:
		snprintf(buffer, bufferLength, "MQTT error bad protocol");
		break;
	case MQTT_ERROR_BAD_CLIENT_ID:
		snprintf(buffer, bufferLength, "MQTT error bad client ID");
		break;
	case MQTT_ERROR_CONNECT_UNAVAILABLE:
		snprintf(buffer, bufferLength, "MQTT error connect unavailable");
		break;
	case MQTT_ERROR_BAD_CREDENTIALS:
		snprintf(buffer, bufferLength, "MQTT error bad credentials");
		break;
	case MQTT_ERROR_CONNECT_UNAUTHORIZED:
		snprintf(buffer, bufferLength, "MQTT error connect unauthorized");
		break;
	case MQTT_ERROR_CONNECT_FAILED:
		snprintf(buffer, bufferLength, "MQTT error connect failed");
		break;
	case MQTT_ERROR_CONNECT_ERROR:
		snprintf(buffer, bufferLength, "MQTT error connect error %d", mqttConnectErrorNumber);
		break;
	case MQTT_ERROR_CONNECT_MESSAGE_FAILED:
		snprintf(buffer, bufferLength, "MQTT error connect message failed");
		break;
	case MQTT_ERROR_LOOP_FAILED:
		snprintf(buffer, bufferLength, "MQTT error loop failed");
		break;
	default:
		snprintf(buffer, bufferLength, "MQTT failed but I'm not sure why");
		break;
	}
}

