#include "Peripherals/Loadcells.h"
#include "Peripherals/IO_Map.h"

// TODO: Calibrate these values
float calib[4] = {420.0983, 421.365, 419.200, 418.500};

HX711 loadcell1;
HX711 loadcell2;
HX711 loadcell3;

int loadcells_setup(void)
{
#if DEFAULT_ID == LIFT_R_ID
    loadcell3.begin(HX711_DOUT4_PIN, HX711_SCK4_PIN);
    loadcell3.set_scale(calib[0]);
    loadcell3.tare();
#elif DEFAULT_ID == LIFT_FS_ID
    loadcell1.begin(HX711_DOUT1_PIN, HX711_SCK1_PIN);
    loadcell1.set_scale(calib[1]);
    loadcell1.tare();
    loadcell2.begin(HX711_DOUT2_PIN, HX711_SCK2_PIN);
    loadcell2.set_scale(calib[2]);
    loadcell2.tare();
    loadcell3.begin(HX711_DOUT3_PIN, HX711_SCK3_PIN);
    loadcell3.set_scale(calib[3]);
    loadcell3.tare();
#endif
    return 0;
}

int read_loadcells(data_t *data)
{
#if DEFAULT_ID == LIFT_FS_ID
    data->loadcells.n2o_bottle_weight = loadcell3.get_units(10);
#elif DEFAULT_ID == LIFT_R_ID
    data->loadcells.thrust_loadcell1 = loadcell1.get_units(10);
    data->loadcells.thrust_loadcell2 = loadcell2.get_units(10);
    data->loadcells.thrust_loadcell3 = loadcell3.get_units(10);
#endif
    return 0;
}

void calibrate(HX711 *loadcell)
{
  Serial.println("\n\nCALIBRATION\n===========");
  Serial.println("remove all weight from the loadcell");
  //  flush Serial input
  while (Serial.available()) Serial.read();

  Serial.println("and press enter\n");
  while (Serial.available() == 0);

  Serial.println("Determine zero weight offset");
  //  average 20 measurements.
  loadcell->tare(20);
  int32_t offset = loadcell->get_offset();

  Serial.print("OFFSET: ");
  Serial.println(offset);
  Serial.println();


  Serial.println("place a weight on the loadcell");
  //  flush Serial input
  while (Serial.available()) Serial.read();

  Serial.println("enter the weight in (whole) grams and press enter");
  uint32_t weight = 0;
  while (Serial.peek() != '\n')
  {
    if (Serial.available())
    {
      char ch = Serial.read();
      if (isdigit(ch))
      {
        weight *= 10;
        weight = weight + (ch - '0');
      }
    }
  }
  Serial.print("WEIGHT: ");
  Serial.println(weight);
  loadcell->calibrate_scale(weight, 20);
  float scale = loadcell->get_scale();

  Serial.print("SCALE:  ");
  Serial.println(scale, 6);

  Serial.print("\nuse scale.set_offset(");
  Serial.print(offset);
  Serial.print("); and scale.set_scale(");
  Serial.print(scale, 6);
  Serial.print(");\n");
  Serial.println("in the setup of your project");

  Serial.println("\n\n");
}

void calibrate_loadcells() {
    calibrate(&loadcell3);
}