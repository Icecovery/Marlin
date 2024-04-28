#include "../gcode.h"
#include "../../MarlinCore.h"

/**
 * M10026 - Perform Electroplating Process
 *  A<int> - Anode Pin Number
 *  S<int> - Sensor Pin Number
 *  I<int> - Target Current in mA
 *  D<float> - Interval Size in V
 *  F<int> - PWM Frequency in Hz
 *  T<int> - Time in milliseconds
*/
void GcodeSuite::M10026()
{
  int anode_pin = PARSED_PIN_INDEX('A', PA0); // default to PA0, must be a pwm
  if (anode_pin < 0)
  {
    SERIAL_ECHO("Invalid Anode Pin\n");
    return;
  }
  if (!PWM_PIN(anode_pin))
  {
    SERIAL_ECHO("Anode Pin must be a PWM Pin\n");
    return;
  }

  int sensor_pin = PARSED_PIN_INDEX('S', PB0); // default to PB0, must be a analog pin
  if (sensor_pin < 0)
  {
    SERIAL_ECHO("Invalid Sensor Pin\n");
    return;
  }

  int target_current = parser.intval('I', 20); // default to 20 mA
  float interval_size = parser.floatval('D', 0.02); // default to 0.02 V
  int pwm_frequency = parser.intval('F', 1000); // default to 1 kHz
  uint32_t time = parser.ulongval('T', 1000); // default to 1 second

  // Set the anode pin to output
  pinMode(anode_pin, OUTPUT);
  // Set the PWM frequency
  hal.set_pwm_frequency(anode_pin, pwm_frequency);

  // Set the sensor pin to input
  hal.adc_enable(sensor_pin);

  SERIAL_ECHO("Electroplating Settings:\n");
  SERIAL_ECHO("Anode Pin: ");
  SERIAL_ECHO(anode_pin);
  SERIAL_ECHO("\n");
  SERIAL_ECHO("Sensor Pin: ");
  SERIAL_ECHO(sensor_pin);
  SERIAL_ECHO("\n");
  SERIAL_ECHO("Target Current: ");
  SERIAL_ECHO(target_current);
  SERIAL_ECHO(" mA\n");
  SERIAL_ECHO("Interval Size: ");
  SERIAL_ECHO(interval_size);
  SERIAL_ECHO(" V\n");
  SERIAL_ECHO("PWM Frequency: ");
  SERIAL_ECHO(pwm_frequency);
  SERIAL_ECHO(" Hz\n");
  SERIAL_ECHO("Time: ");
  SERIAL_ECHO(time);
  SERIAL_ECHO(" ms\n");

  float outputV = 0.5; // start at 0.5 V
  uint32_t start_time = millis();
  int iteration = 0;
  do 
  {
    int output = (outputV / 5) * 255;
    hal.set_pwm_duty(anode_pin, output);

    dwell(50); // wait for the voltage to stabilize

    // read the current
    int sensorValue = analogRead(sensor_pin);
    float current = 1000 * (sensorValue / 1023.0) * 0.5;

    // adjust the output voltage based on the current
    if (current < target_current)
    {
      outputV += interval_size;
    }
    else if (current > target_current)
    {
      outputV -= interval_size;
    }
    outputV = constrain(outputV, 0, 5);

    SERIAL_ECHO("Iteration ");
    SERIAL_ECHO(iteration);
    SERIAL_ECHO(" (");
    SERIAL_ECHO(millis() - start_time);
    SERIAL_ECHO(" ms): ");
    SERIAL_ECHO(current);
    SERIAL_ECHO(" mA at ");
    SERIAL_ECHO(outputV);
    SERIAL_ECHO(" V (PWM ");
    SERIAL_ECHO(output);
    SERIAL_ECHO(")\n");

    iteration++;
  } while (millis() - start_time < time);

  // turn off the anode
  hal.set_pwm_duty(anode_pin, 0);

  SERIAL_ECHOLN(STR_DONE);
}