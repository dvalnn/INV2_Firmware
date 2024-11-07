int status_interval = 300; // ms
int packet_read_timeout = 250; // ms
int packet_loss_timeout = 250;
int heartbeat_timeout = 1000;
int doubt_timeout = 10000;

int chart_interval = 200; // ms

// GUI Positions and Sizes
float button_x1 = .8; // * displayWidth
float button_x2 = .89;
float button_height = .04; // * displayHeight
float button_height_big = .07;
float button_width = .13; // * displayWidth
float toggle_height = .04;
float toggle_width = .04;

int baudRate = 115200;

byte MyID = (byte) 0x00;

List<String> programs = Arrays.asList("Safety Pressure", "Purge Pressure", "Purge Liquid", "Fill He", "Fill N2O", "Purge Line");
byte[] prog_cmds = {(byte)0x00, (byte)0x01, (byte)0x02, (byte)0x03, (byte)0x04, (byte)0x05};

List<String> vars = Arrays.asList("Target Pressure", "Trigger Pressure", "Target Liquid");
List<String> IDs = Arrays.asList( "1 : Rocket", "2 : Filling Station", "3 : Ignition", "4 : Broadcast");

String logFileName = "log.txt";

String fill_img;
String fill_img_light = "diagrams/fillingBlack.png";
String fill_img_dark = "diagrams/fillingWhite.png";
String map_img = "diagrams/map.png";
String map_pimg = "diagrams/padock_map.png";
final float MIN_LAT = 39.335159, MAX_LAT = 39.397242, MIN_LONG = -8.331430, MAX_LONG = -8.176142;
final float pMIN_LAT = 39.204760, pMAX_LAT = 39.219381, pMIN_LONG = -8.073767, pMAX_LONG = -8.043232;
int history_capacity = 10;

int cmd_size = 14; // 13 comandos + 1

// colors
int bgColor;
int bgColorDark = color(0, 0, 0);
int bgColorLight = color(250, 250, 250);
int red = color(150, 55, 35);
int dark_red = color(150, 39, 23);
int blue = color(0, 100, 117);
int dark_blue = color(0, 47, 55);
int orange = color(92, 67, 0);
int light_orange = color(110, 93, 23);

CColor abortColor = new CColor();
CColor stopColor = new CColor();
CColor defaultColor = new CColor();
CColor unactiveColor = new CColor();
int labelColor;
int labelColor2;

CColor abortColorDark = new CColor();
CColor stopColorDark = new CColor();
CColor defaultColorDark = new CColor();
CColor unactiveColorDark = new CColor();
int labelColorDark = color(255, 255, 255);
int labelColor2Dark = color(180, 180, 180);

CColor abortColorLight = new CColor();
CColor stopColorLight = new CColor();
CColor defaultColorLight = new CColor();
CColor unactiveColorLight = new CColor();
int labelColorLight = color(0, 0, 0);
int labelColor2Light = color(70, 70, 70);

void setupColors() {
  // dark mode
  abortColorDark.setForeground(red)
    .setBackground(dark_red)
    .setActive(blue);
  defaultColorDark.setForeground(blue)
    .setBackground(dark_blue)
    .setActive(red);
  unactiveColorDark.setForeground(bgColor)
    .setBackground(dark_red)
    .setActive(red);
  stopColorDark.setForeground(light_orange)
    .setBackground(orange)
    .setActive(blue);

  // light mode
  abortColorLight.setForeground(red)
    .setBackground(dark_red)
    .setActive(blue);
  defaultColorLight.setForeground(blue)
    .setBackground(dark_blue)
    .setActive(red);
  unactiveColorLight.setForeground(bgColor)
    .setBackground(dark_red)
    .setActive(red);
  stopColorLight.setForeground(light_orange)
    .setBackground(orange)
    .setActive(blue);

  // choose mode
  abortColor = abortColorDark;
  stopColor = stopColorDark;
  unactiveColor = unactiveColorDark;
  defaultColor = defaultColorDark;
  bgColor = bgColorDark;
  fill_img = fill_img_dark;
  labelColor = labelColorDark;
  labelColor2 = labelColor2Dark;
}
