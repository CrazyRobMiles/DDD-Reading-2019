#pragma once

#include "utils.h"
#include "commands.h"

unsigned long mqtt_reading_retry_interval_in_millis;
unsigned long mqtt_reading_interval_in_millis;
unsigned long milliseconds_at_last_mqtt_update;

unsigned long lora_reading_retry_interval_in_millis;
unsigned long lora_reading_interval_in_millis;
unsigned long milliseconds_at_last_lora_update;

// return true if it is time for an update
bool mqtt_interval_has_expired()
{
	unsigned long time_in_millis = millis();
	unsigned long millis_since_last_mqtt_update = ulongDiff(time_in_millis, milliseconds_at_last_mqtt_update);


	if (millis_since_last_mqtt_update > mqtt_reading_interval_in_millis)
	{
		return true; 
	}
	return false;
}


bool lora_interval_has_expired()
{
	unsigned long time_in_millis = millis();
	unsigned long millis_since_last_lora_update = ulongDiff(time_in_millis, milliseconds_at_last_lora_update);


	if (millis_since_last_lora_update > lora_reading_interval_in_millis)
	{
		return true;  
	}
	return false;
}


unsigned long time_to_next_mqtt_update()
{
	unsigned long millis_since_last_mqtt_update = ulongDiff(millis(), milliseconds_at_last_mqtt_update);

	if (millis_since_last_mqtt_update > mqtt_reading_interval_in_millis)
		return 0;
	else
		return mqtt_reading_interval_in_millis - millis_since_last_mqtt_update;
}

unsigned long time_to_next_lora_update()
{
	unsigned long millis_since_last_lora_update = ulongDiff(millis(), milliseconds_at_last_lora_update);

	if (millis_since_last_lora_update > lora_reading_interval_in_millis)
		return 0;
	else
		return lora_reading_interval_in_millis - millis_since_last_lora_update;
}

void readings_ready()
{
	unsigned long time_in_millis = millis();

	if (settings.mqtt_enabled)
	{
		unsigned long millis_since_last_mqtt_update = ulongDiff(time_in_millis, milliseconds_at_last_mqtt_update);


		if (millis_since_last_mqtt_update > mqtt_reading_interval_in_millis)
		{
			milliseconds_at_last_mqtt_update = time_in_millis;

			if (send_to_mqtt())
			{
				mqtt_reading_interval_in_millis = settings.seconds_per_mqtt_update * 1000;
			}
			else
			{
				mqtt_reading_interval_in_millis = settings.seconds_per_mqtt_retry * 1000;
			}
		}
	}

	if (pub_mqtt_force_send)
	{
		if (send_to_mqtt())
		{
			pub_mqtt_force_send = false;
		}
	}

	if (settings.lora_enabled)
	{
		unsigned long millis_since_last_lora_update = ulongDiff(time_in_millis, milliseconds_at_last_lora_update);

		if (millis_since_last_lora_update > lora_reading_interval_in_millis)
		{
			milliseconds_at_last_lora_update = time_in_millis ;
			send_to_lora();
		}
	}

	if (pub_lora_force_send || pub_lora_force_test)
	{
		if (send_to_lora())
		{
			pub_lora_force_send = false;
			pub_lora_force_test = false;
		}
	}
}

void turn_sensor_on()
{
	if (timing_state != sensorOn)
	{
		Serial.println("Turning sensor on");
		set_sensor_working(true);
		timing_state = sensorOn;
	}
}

void turn_sensor_off()
{
	if (timing_state != sensorOff)
	{
		Serial.println("Turning sensor off");
		set_sensor_working(false);
		timing_state = sensorOff;
	}
}


void start_getting_readings()
{
	Serial.println("Starting to fetch readings");
	// clear the averages
	pub_pm25Average.restartAverage();
	pub_pm10Average.restartAverage();
	timing_state = gettingReading;
}

bool can_power_off_sensor()
{
	long milliseconds_for_sensor_warmup = settings.seconds_sensor_warmup * 1000;

	if (time_to_next_mqtt_update() > milliseconds_for_sensor_warmup &&
		time_to_next_lora_update() > milliseconds_for_sensor_warmup) {
		return true;
	}
	else
		return false;
}

bool can_start_reading()
{
	// take two readings a second
	long milliseconds_for_averaging = (NO_OF_AVERAGES / 2) * 1000;

	if (time_to_next_mqtt_update() > milliseconds_for_averaging &&
		time_to_next_lora_update() > milliseconds_for_averaging) {
		return false;
	}
	else
		return true;
}

void check_sensor_power()
{
	if (can_power_off_sensor())
	{
		turn_sensor_off();
	}
	else
	{
		turn_sensor_on();
	}
}

void timing_sensor_off()
{
	check_sensor_power();
}

void timing_sensor_on()
{
	if (can_start_reading())
		start_getting_readings();
}

void timing_sensor_getting_reading()
{
#ifdef TEST_LORA
	if (1)
#else
	if (pub_air_values_ready && pub_bme_values_ready) 
#endif
	{
		Serial.println("Readings ready");
		readings_ready();
		pub_air_values_ready = false;
		pub_bme_values_ready = false;
		check_sensor_power();
	}
}

void setup_timing()
{
	unsigned long time_in_millis = millis();
	mqtt_reading_interval_in_millis = settings.seconds_per_mqtt_update * 1000;
	milliseconds_at_last_mqtt_update = time_in_millis - mqtt_reading_interval_in_millis;
	lora_reading_interval_in_millis = settings.seconds_per_lora_update * 1000;
	milliseconds_at_last_lora_update = time_in_millis - lora_reading_interval_in_millis;
}

void loop_timing()
{
	switch (timing_state)
	{
	case sensorOff:
		timing_sensor_off();
		break;

	case sensorOn:
		timing_sensor_on();
		break;

	case gettingReading:
		timing_sensor_getting_reading();
		break;

	}

	if (pub_lora_force_send)
	{
		unsigned long time_in_millis = millis();
		milliseconds_at_last_lora_update = time_in_millis - 
			((settings.seconds_per_lora_update * 1000) - (settings.seconds_sensor_warmup * 1000));
		pub_lora_force_send = false;
	}

	if (pub_mqtt_force_send)
	{
		long milliseconds_for_sensor_warmup = settings.seconds_sensor_warmup * 1000;
		unsigned long time_in_millis = millis();
		milliseconds_at_last_mqtt_update = time_in_millis -
			((settings.seconds_per_mqtt_update * 1000) - (settings.seconds_sensor_warmup * 1000));
		pub_mqtt_force_send = false;
	}

	if (pub_lora_force_test)
	{
		if (send_to_lora())
		{
			pub_lora_force_test = false;
		}
	}
}
