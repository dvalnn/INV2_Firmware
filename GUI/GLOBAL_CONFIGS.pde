int status_interval = 1000; // ms
int packet_read_timeout = 250; // ms
int packet_loss_timeout = 250;
int heartbeat_timeout = 1000;

int chart_interval = 200; // ms

// GUI Positions and Sizes
float button_x1 = .8; // * displayWidth
float button_x2 = .89;
float button_height = .04; // * displayHeight
float button_width = .13; // * displayWidth

int baudRate = 115200;

byte MyID = (byte) 0x00;

List<String> programs = Arrays.asList("Safety Pressure", "Purge Pressure", "Purge Liquid", "Fill He", "Fill N2O", "Purge Line");
byte[] prog_cmds = {(byte)0x00, (byte)0x01, (byte)0x02, (byte)0x03, (byte)0x04, (byte)0x05};

List<String> vars = Arrays.asList("Target Pressure", "Trigger Pressure", "Target Liquid");
List<String> IDs = Arrays.asList( "1 : Rocket", "2 : Filling Station", "3 : Broadcast");

String logFileName = "log.txt";

String fill_img = "diagrams/filling.png";

int history_capacity = 10;

int cmd_size = 14; // 13 comandos + 1

// colors
int bgColor = color(0, 0, 0);
int red = color(150, 55, 35);
int dark_red = color(150, 39, 23);
int blue = color(0, 100, 117);
int dark_blue = color(0, 47, 55);
int orange = color(92, 67, 0);
int light_orange = color(110, 93, 23);

CColor abortColor = new CColor();
CColor stopColor = new CColor();
CColor colors2 = new CColor();
CColor unactiveColor = new CColor();

void setupColors() {
  abortColor.setForeground(red)
    .setBackground(dark_red)
    .setActive(blue);
  colors2.setForeground(blue)
    .setBackground(dark_blue)
    .setActive(red);
  unactiveColor.setForeground(bgColor)
    .setBackground(dark_red)
    .setActive(red);
  stopColor.setForeground(light_orange)
    .setBackground(orange)
    .setActive(blue);
}
