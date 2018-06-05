
#ifndef _SENS_TSL2561_H_
#define _SENS_TSL2561_H_

#include <Wire.h>
#include <Sensors.h>
#include <TSL2561.h>

namespace as {

class Sens_Tsl2561 : public Sensor {

  uint16_t  _brightnessFull, _brightnessIR, _brightnessVis;
  uint32_t  _brightnessLux;
  bool      _x16;
  ::TSL2561 _tsl2561;
  
  void measureRaw (uint8_t sensitivity) {
    switch (sensitivity) {
      case 0: _tsl2561.setTiming(TSL2561_INTEGRATIONTIME_13MS);  break;
      case 1: _tsl2561.setTiming(TSL2561_INTEGRATIONTIME_101MS); break;
      case 2:
      default:
              _tsl2561.setTiming(TSL2561_INTEGRATIONTIME_402MS); break;
    }
    uint32_t lum = _tsl2561.getFullLuminosity();
    _brightnessFull = lum & 0xFFFF;
    _brightnessIR   = lum >> 16;
    _brightnessVis  = _brightnessFull - _brightnessIR;
  }

public:

  // constructor with parameter in header file -> Initalize with 'member initialiser' syntax in constructor
  Sens_Tsl2561 () : _tsl2561(TSL2561_ADDR_FLOAT), _x16(true), _brightnessFull(0), _brightnessIR(0), _brightnessVis(0), _brightnessLux(0) { }

  void init () {

    Wire.begin(); //ToDo sync with further I2C sensor classes if needed

    uint8_t i = 10;
    while( (!_tsl2561.begin()) && (i > 0) ) {
      delay(100);
      i--;
    }
    if (i > 0) {
      _present = true;
      _tsl2561.setGain(TSL2561_GAIN_0X);
      _tsl2561.setTiming(TSL2561_INTEGRATIONTIME_402MS);
      DPRINTLN("Success: found TSL2561 sensor");
    }
    else {
      DPRINTLN("Error: no TSL2561 sensor found");
    }
  }

  void measure () {
    if (_present == true) {
      uint8_t sensitivity = 2;
      uint8_t overflow = 0;
      do {
        measureRaw(sensitivity);
	// zu viel Licht
        if ( (_brightnessFull == _brightnessIR) || (_brightnessFull > 60000) ) {
          if (sensitivity > 0) { sensitivity--; }
	  else { overflow = 1; break; }	// ungültiger Wert, keine weitere Sensitivity Verringerung möglich
        }
	else { break; }			// brightness Wert ok
      } while (true);
      if (!overflow) {
        _brightnessLux  = _tsl2561.calculateLux(_brightnessFull, _brightnessIR);
      }
      else {
        _brightnessLux  = 130000;	// overflow, zu viel Licht, besser 130000 Lux (klarer Himmel und Sonne im Zenit) anstatt 0 Lux melden!
      }
      
      DPRINT("TSL2561 Sensitivity    : "); DDECLN(sensitivity);
      DPRINT("TSL2561 Brightness Full: "); DDECLN(_brightnessFull);
      DPRINT("TSL2561 Brightness IR  : "); DDECLN(_brightnessIR);
      DPRINT("TSL2561 Brightness Vis : "); DDECLN(_brightnessVis);
      DPRINT("TSL2561 Brightness Lux : "); DDECLN(_brightnessLux);
    }
  }

  uint16_t  brightnessFull () { return _brightnessFull; }
  uint16_t  brightnessIR ()   { return _brightnessIR; }
  uint16_t  brightnessVis ()  { return _brightnessVis; }
  uint32_t  brightnessLux ()  { return _brightnessLux; }
};

}

#endif

/*

Innen
-------------------------------------------------------------------------------
Time  13  Gain  0  Lum 00060015  Full    21  IR     6  Vis    15  Lux   192	1	0
Time 101  Gain  0  Lum 002F009C  Full   156  IR    47  Vis   109  Lux   186	7.8	1
Time 402  Gain  0  Lum 00BC026F  Full   623  IR   188  Vis   435  Lux   186	31	2
Time  13  Gain 16  Lum 0064014B  Full   331  IR   100  Vis   231  Lux   181	16	-
Time 101  Gain 16  Lum 02EF09BA  Full  2490  IR   751  Vis  1739  Lux   185	125	-
Time 402  Gain 16  Lum 0BB826D0  Full  9936  IR  3000  Vis  6936  Lux   186	500	-

Sonne, Juni 18 Uhr, indirekt
-------------------------------------------------------------------------------
Time  13  Gain  0  Lum 025305EF  Full  1519  IR   595  Vis   924  Lux  9713
Time 101  Gain  0  Lum 114D2C25  Full 11301  IR  4429  Vis  6872  Lux  9805
Time 402  Gain  0  Lum 4451AE93  Full 44691  IR 17489  Vis 27202  Lux  9779
Time  13  Gain 16  Lum 13441344  Full  4932  IR  4932  Vis     0  Lux    53	invalid
Time 101  Gain 16  Lum 90C690C6  Full 37062  IR 37062  Vis     0  Lux    54	invalid
Time 402  Gain 16  Lum FFFFFFFF  Full 65535  IR 65535  Vis     0  Lux    24	invalid

Sonne, Juni 18 Uhr, direkt
-------------------------------------------------------------------------------
Time  13  Gain  0  Lum 13441344  Full  4932  IR  4932  Vis     0  Lux   846	invalid
Time 101  Gain  0  Lum 90C690C6  Full 37062  IR 37062  Vis     0  Lux   863	invalid
Time 402  Gain  0  Lum FFFFFFFF  Full 65535  IR 65535  Vis     0  Lux   384	invalid
Time  13  Gain 16  Lum 13441344  Full  4932  IR  4932  Vis     0  Lux    53	invalid
Time 101  Gain 16  Lum 90C690C6  Full 37062  IR 37062  Vis     0  Lux    54	invalid
Time 402  Gain 16  Lum FFFFFFFF  Full 65535  IR 65535  Vis     0  Lux    24	invalid

*/
