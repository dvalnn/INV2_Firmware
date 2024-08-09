int status_interval = 1000; // ms
int packet_read_timeout = 250; // ms
int packet_loss_timeout = 250;

// GUI Positions and Sizes
float button_x1 = .7; // * displayWidth
float button_x2 = .85;
float button_height = .04; // * displayHeight
float button_width = .13; // * displayWidth

int baudRate = 115200;

byte MyID = (byte) 0x00;

List<String> programs = Arrays.asList("Safety Pressure", "Purge Pressure", "Purge Liquid", "Fill He", "Fill N2O", "Purge Line");
byte[] prog_cmds = {(byte)0x00, (byte)0x01, (byte)0x02, (byte)0x03, (byte)0x04, (byte)0x05};

List<String> vars = Arrays.asList("Target Pressure", "Trigger Pressure", "Target Liquid");
List<String> IDs = Arrays.asList( "1 : Rocket", "2 : Filling Station", "3 : Broadcast");

String logFileName = "log.txt";

int cmd_size = 14; // 13 comandos + 1
