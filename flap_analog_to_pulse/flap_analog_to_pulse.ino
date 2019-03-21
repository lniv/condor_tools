/*
convert an analog read of a flap handle to pulses, as condor needs.
Niv Levy March 2019
GPL v2

read a pot and send pulses to two wires (pulling them to ground).
TODO: must be willing to accept override on the wires, and adjust state accordingly


1 meant to be run on an esp8266, as it's my digital logic platform of choice these days; it sucks for general use, but it's dirt cheap
2 acually using a pro mini 5V, 16M since i've had lying around forever, and this is a one of.

*/
#define ARRLEN(a) (sizeof(a)/sizeof((a)[0]))

// lines to be switched in response to flap handle position.
#define FLAPS_UP 4
#define FLAPS_DOWN 5

//location of upper and lowe limit in which the flap is considered to be in a flap position.
//TBD -= adjust as needed.
int16_t flap_upper_values[] = {90, 135, 180, 225, 270, 320, 365};
int16_t flap_lower_values[] = {85, 125, 170, 215, 260, 310, 355};
uint8_t flap_state, closest_flap = 0;

uint32_t t0;

/*figure out which flap state we're closest to
 * do nothing - reaction happens elsewhere (and when booting, we just set the flap state to wherever we found it)
 */
void determine_closest_flap_state(uint16_t sensor_value) {
	uint8_t i;
	int16_t flap_nominal_value, distance_to_tested_flap, closest_distance = 32767;
	
	/*
	Serial.print("checking for value of ");
	Serial.println(sensor_value);
	*/
	
	for (i = 0; i < ARRLEN(flap_lower_values); i++) {
		flap_nominal_value = (flap_lower_values[i] + flap_upper_values[i]) / 2;
		distance_to_tested_flap = sensor_value - flap_nominal_value;
		distance_to_tested_flap = abs(distance_to_tested_flap);
		
		/*
		Serial.print(i);
		Serial.print(", nominal value ");
		Serial.print(flap_nominal_value);
		Serial.print(", distance to it "); 
		Serial.print(distance_to_tested_flap);
		Serial.print(", closest up to now ");
		Serial.println(closest_distance);
		*/
		
		if (distance_to_tested_flap < closest_distance) {
			closest_flap = i;
			closest_distance = distance_to_tested_flap;
		}
	}
	/*
	Serial.print("best flap = ");
	Serial.println(closest_flap);
	*/
}

/* check if we're within some fraction of the range near the nominal value
 * if so, do nothing
 * else, figure out which flap we're closests to, how many clicks it is one way or another, and change the flap setting.
 */
void update_flap_state(uint16_t sensor_value) {
	int8_t flap_delta;
	uint8_t i;
	uint8_t flap_switch;
	
	determine_closest_flap_state(sensor_value);
	// if we're within range of a flap position different than current one, switch.
	if (closest_flap != flap_state && sensor_value > flap_lower_values[closest_flap] && sensor_value < flap_upper_values[closest_flap]) {
		// figure out how far are we from it
		flap_delta = closest_flap - flap_state;
		
		if (flap_delta > 0) {
			flap_switch = FLAPS_UP;
		}
		else {
			flap_switch = FLAPS_DOWN;
			flap_delta = -flap_delta;
		}
		
		Serial.print("sensor value = ");
		Serial.print(sensor_value);
		Serial.print(" : flap change to ");
		Serial.print(closest_flap);
		Serial.print(", switch = ");
		Serial.print(flap_switch);
		Serial.print(" delta ");
		Serial.println(flap_delta);
		
		for (i =0 ; i < flap_delta; i++) {
			digitalWrite(flap_switch, LOW);
			delay(50);
			digitalWrite(flap_switch, HIGH);
		}
		flap_state = closest_flap;
	}
}

void setup() {
	uint16_t sensor_value;
	Serial.begin(9600);

	delay(1000); // just to get the serial monitor time to wake up
	Serial.println("Begin setup");
	pinMode(A0, INPUT);
	pinMode(FLAPS_DOWN, OUTPUT);
	pinMode(FLAPS_UP, OUTPUT);
	
	digitalWrite(FLAPS_DOWN, LOW);
	digitalWrite(FLAPS_UP, LOW);
	delay(500);
	digitalWrite(FLAPS_DOWN, HIGH);
	digitalWrite(FLAPS_UP, HIGH);
	
	// to start, just set it to whichever is closest, even if it's not in range.
	sensor_value = analogRead(A0);
	determine_closest_flap_state(sensor_value);
	flap_state = closest_flap;
	Serial.print("initial sensor value = ");
	Serial.print(sensor_value);
	Serial.print(", therfore initial flap state = ");
	Serial.println(flap_state);
	
	t0 = (uint32_t) millis();
	Serial.println("End setup");
}


void loop() {
	uint16_t sensor_value;
	// read the value from the sensor:
	sensor_value = analogRead(A0);
	update_flap_state(sensor_value);
	/*
	Serial.print(millis() - t0);
	Serial.print(", ");
	Serial.println(sensor_value);
	*/
	//delay(100);
}