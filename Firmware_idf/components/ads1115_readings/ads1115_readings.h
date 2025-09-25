// ads1115_readings.h

#ifndef ADS1115_READINGS_H
#define ADS1115_READINGS_H

#include "ads111x.h"


// --- North American style (240 Ω → 33 Ω) ---
#define ADC_EMPTY_NA   32767
#define ADC_FULL_NA     0

#define FUEL_PERCENT_NA_RAW(raw) ( \
    ((ADC_EMPTY_NA - (raw)) / (ADC_EMPTY_NA - ADC_FULL_NA)) * 100 \
)

// --- European/VDO style (180 Ω → 10 Ω) ---
// Replace with actual ADC_EMPTY_EU and ADC_FULL_EU once calculated
#define ADC_EMPTY_EU   18000  // placeholder
#define ADC_FULL_EU     6000  // placeholder

#define FUEL_PERCENT_EU_RAW(raw) ( \
    ((ADC_EMPTY_EU - (raw)) / (ADC_EMPTY_EU - ADC_FULL_EU)) * 100 \
)


// ADC range from sender (calculated with 100Ω fixed resistor)
#define ADC_PSI_MAX   32767  // at 0 PSI (240Ω sender)
#define ADC_PSI_MIN    0   // at 100 PSI (33Ω sender)

// Macro to map ADC value to PSI
#define ADC_TO_PSI(adc)  ( ((adc) - ADC_PSI_MIN) * 100L / (ADC_PSI_MAX - ADC_PSI_MIN) )



extern int16_t VOLT_SIG;
extern int16_t SWITCH;
extern int16_t tach_value;
extern int16_t tilt_trim_value;
extern int16_t oil_press_value;
extern int16_t coolant_value;
extern int16_t warning;
extern int16_t fuel_1;
extern int16_t fuel_2;
extern int16_t starter;

void init_ads1115();
void readSCL();
void readGND();
void readVCC();
int  get_sensor_data_json(char *buffer, size_t size);

#endif // ADS1115_READINGS_H
