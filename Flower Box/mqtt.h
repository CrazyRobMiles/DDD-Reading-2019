#define MQTT_DISPLAY_TIMEOUT 3000

#define MQTT_RECONNECT_TIMEOUT 20000

unsigned long mqtt_timer_start;

Client * espClient = NULL;

PubSubClient * mqttPubSubClient = NULL;

#define MQTT_RECEIVE_BUFFER_SIZE 240
char mqtt_receive_buffer[MQTT_RECEIVE_BUFFER_SIZE];

#define MQTT_SEND_BUFFER_SIZE 240

char mqtt_send_buffer[MQTT_SEND_BUFFER_SIZE];

boolean send_buffer_to_mqtt( char * buffer)
{
	TRACELN("Sending to MQTT");
	if (mqttState != ConnectedToMQTTServer)
	{
		TRACELN("MQTT not connected yet");
		return false;
	}

	TRACELN(mqtt_send_buffer);

	Serial.print("MQTT Sending: ");
	Serial.println(buffer);

	if (mqttPubSubClient->publish(settings.mqttPublishTopic, buffer))
	{
		mqtt_message_count++;
		TRACELN("MQTT sent");
		Serial.println("MQTT Sent");
	}
	else {
		TRACELN("MQTT send failed");
		Serial.println("Send failed");
	}

	return true;
}

boolean send_to_mqtt()
{
	if (pub_got_gps_fix)
	{
		// got a GPS fix - how old is it?
		ulong fix_age = ulongDiff(millis(), pub_ticks_at_last_gps_update);
		if (fix_age < MILLIS_LIFETIME_OF_GPS_FIX)
		{
			double latitude = pub_latitude_mdeg / 1000000.0;
			double longitude = pub_longitude_mdeg / 1000000.0;

			// still regard the fix as valid - add lat and long to the message
			sprintf(mqtt_send_buffer,
				"{\"dev\":\"%s\", \"temp\":%.2f, \"humidity\" : %.2f, \"pressure\" : %.2f, \"PM10\" : %.2f, \"PM25\" : %.2f, \"Lat\" : %.6f, \"Long\" : %.6f, \"timestamp\" : \"%s %s %d %d %02d:%02d:%02d GMT+0000\"}",
				settings.mqttName,
				pub_temp,
				pub_humidity,
				pub_pressure,
				pub_ppm_10,
				pub_ppm_25,
				latitude,
				longitude,
				dayNames[pub_day_of_week],
				monthNames[pub_month],
				pub_day,
				pub_year,
				pub_hour,
				pub_minute,
				pub_second);
			return send_buffer_to_mqtt(mqtt_send_buffer);
		}
	}

	// otherwise send the message with no location information

	sprintf(mqtt_send_buffer,
		"{\"dev\":\"%s\",\"temp\":%.2f, \"humidity\" : %.2f, \"pressure\" : %.2f, \"PM10\" : %.2f, \"PM25\" : %.2f, \"timestamp\" : \"%s %s %d %d %02d:%02d:%02d GMT+0000\"}",
		settings.mqttName, 
		pub_temp,
		pub_humidity,
		pub_pressure,
		pub_ppm_10,
		pub_ppm_25,
		dayNames[pub_day_of_week],
		monthNames[pub_month],
		pub_day,
		pub_year,
		pub_hour,
		pub_minute,
		pub_second);

	return send_buffer_to_mqtt(mqtt_send_buffer);
}

void mqtt_deliver_command_result(char * result)
{
	send_buffer_to_mqtt(result);
}

void callback(char* topic, byte* payload, unsigned int length)
{
	TRACE("Message arrived in topic: ");
	TRACELN(topic);

	TRACE("Message:");
	int i;
	for (i = 0; i < length; i++) {
		TRACE((char)payload[i]);
		mqtt_receive_buffer[i] = (char)payload[i];
	}

	// Put the terminator on the string

	mqtt_receive_buffer[i] = 0;

	TRACELN();
	TRACELN("-----------------------");

	act_onJson_command(mqtt_receive_buffer, mqtt_deliver_command_result);
}

void setup_mqtt()
{
	if (espClient == NULL)
	{
		if (settings.mqttPort == 1883)
		{
			Serial.println("Using insecure sockets");
			espClient = new WiFiClient();
		}
		else
		{
			Serial.println("Using secure sockets");
			espClient = new WiFiClientSecure();
		}

		mqttPubSubClient = new PubSubClient(*espClient);
		mqttPubSubClient->setServer(settings.mqttServer, settings.mqttPort);
		mqttPubSubClient->setCallback(callback);
	}

	mqttState = AwaitingWiFi;
}

void mqtt_connect_failed()
{
	TRACE("MQTT failed with state: ");

	TRACELN(mqttPubSubClient->state());
	mqtt_timer_start = millis();

	sprintf(mqtt_send_buffer,
		"Connect Failed: %d",
		mqttPubSubClient->state());

	update_action(settings.mqttName, mqtt_send_buffer);
	mqttState = ShowingConnectToMQTTServerFailed;
}

void mqtt_connected()
{
	TRACELN("MQTT connected");
	mqtt_timer_start = millis();
	update_action(settings.mqttName, "Connected OK");
	mqttPubSubClient->subscribe(settings.mqttSubscribeTopic);
	mqttState = ShowingConnectedToMQTTServer;
}

void attempt_mqtt_connect()
{
	Serial.print("mqttName: ");
	Serial.println(settings.mqttName);
	Serial.print("mqttUser: ");
	Serial.println(settings.mqttUser);
	Serial.print("mqttPassword: ");
	Serial.println(settings.mqttPassword);
	if (mqttPubSubClient->connect(settings.mqttName, settings.mqttUser, settings.mqttPassword))
	{
		mqtt_connected();
	}
	else
	{
		mqtt_connect_failed();
	}
}

void loop_mqtt()
{
	unsigned long elapsed_time;

	switch (mqttState)
	{
	case AwaitingWiFi:
		if (wifiState == WiFiConnected)
		{
			// Start the MQTT connection running
			start_action(settings.mqttName, "Connecting to MQTT");
			attempt_mqtt_connect();
		}
		break;

	case ConnectingToMQTTServer:
		attempt_mqtt_connect();
		break;

	case ShowingConnectedToMQTTServer:
		elapsed_time = millis() - mqtt_timer_start;

		if (elapsed_time > MQTT_DISPLAY_TIMEOUT)
		{
			end_action();
			mqttState = ConnectedToMQTTServer;
		}
		mqttPubSubClient->loop();
		break;

	case ShowingConnectToMQTTServerFailed:
		elapsed_time = millis() - mqtt_timer_start;

		if (elapsed_time > MQTT_DISPLAY_TIMEOUT)
		{
			end_action();
			mqtt_timer_start = millis();
			mqttState = ConnectToMQTTServerFailed;
		}
		break;

	case ConnectedToMQTTServer:

		if (wifiState != WiFiConnected)
		{
			mqttState = AwaitingWiFi;
		}

		if (!mqttPubSubClient->loop())
		{
			mqttPubSubClient->disconnect();
			mqtt_timer_start = millis();
			mqtt_disconnect_count++;
			mqttState = ConnectToMQTTServerFailed;
		}

		break;

	case ConnectToMQTTServerFailed:

		elapsed_time = millis() - mqtt_timer_start;

		if (elapsed_time > MQTT_RECONNECT_TIMEOUT)
		{
			mqttState = AwaitingWiFi;
		}
		break;
	}
}

