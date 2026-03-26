// ads1115_readings.h

#ifndef ADS1115_READINGS_H
#define ADS1115_READINGS_H

#include "ads111x.h"

extern float voltage_by_ads;
extern int16_t VOLT_SIG;
extern int16_t SWITCH;
extern int16_t tach_value;
extern int16_t rpm_display;
extern int16_t tilt_trim_value;
extern int16_t oil_press_value;
extern int16_t coolant_value;
extern int16_t warning;

extern int16_t fuel_1;
extern int16_t fuel_2;
extern int16_t starter;
extern float starter_voltage;
extern char fuel_sensor_type; // 'A' for American, 'E' for European


void init_ads1115();
void readSCL();
void readGND();
void readVCC();
int  get_sensor_data_json(char *buffer, size_t size);

#endif // ADS1115_READINGS_H
