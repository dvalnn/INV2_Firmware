#include "FlashLog.h"

#include "HardwareCfg.h"
#include "GlobalVars.h"
#include "StateMachine.h"

bool log_running = false;
File file;

uint16_t current_id;
uint32_t data_used;

void start_log()
{
    log_running = true;

    /*
     * TODO
     *   before opening verify that the file does not exist,
     *   256 diferent reboots before we are out of files
     */
    char filename[100] = "";
    int count = sprintf(filename, LOG_NAME_PATTERN_WRITE, current_id);
    current_id++;
    Serial.print("new file \n");
    Serial.print(filename);
    Serial.print("\n");

    file = SD.open(filename, FILE_WRITE, true);

    if (!file)
    {
        log_running = false;
        // assert(false);
    }
}
void stop_log()
{
    log_running = false;
    file.close();
}

uint16_t get_last_id()
{

    //uint16_t last_log_id = preferences.getUInt("last_log_id", 0);
    uint16_t last_log_id = 0;
    
    File root = SD.open("/");
    while (1)
    {
        File entry = root.openNextFile();

        if (!entry)
            break; // no more files

        if (!entry.isDirectory())
        {
            uint16_t temp;
            sscanf(entry.name(), "%u", &temp);
            //Serial.print(temp);
            last_log_id = max(last_log_id, temp);
        }
        //Serial.print(" Name in sd: ");
        //Serial.print(entry.name());
        //Serial.print(" ");
        //Serial.print(entry.size());
        //Serial.print(" \n");

        entry.close();
    }

    root.close();

    preferences.putUInt("last_log_id", last_log_id);

    return last_log_id;
}

void log(void *data, uint16_t size, log_event_t event)
{
    if (log_running == false)
        return;

    static uint8_t buff[256];
    uint16_t index = 0;

    uint32_t time = millis();

    buff[index++] = event;

    buff[index++] = (time >> 24) & 0xff;
    buff[index++] = (time >> 16) & 0xff;
    buff[index++] = (time >> 8) & 0xff;
    buff[index++] = (time) & 0xff;

    switch (event)
    {
    case SENSOR_READING:
    {

        buff[index++] = state;
        buff[index++] = (uint8_t)((log_running << 7) |
                                        (Tank_Top_Module.valve_state << 6) |
                                        (Tank_Bot_Module.valve_state << 5) |
                                        (Chamber_Module.valve_state << 4) |
                                        (DragDeployed << 3) |
                                        (MainDeployed << 2));
 
        uint16_t gps_altitude = (uint16_t)(gps.altitude.meters());
        buff[index++] = (uint8_t)((gps_altitude >> 8) & 0xff);
        buff[index++] = (uint8_t)((gps_altitude) & 0xff);

        uint16_t ualtitude = altitude;
        buff[index++] = (uint8_t)((ualtitude >> 8) & 0xff);
        buff[index++] = (uint8_t)((ualtitude) & 0xff);

        uint16_t u_ax = imu_ax * 10;
        buff[index++] = (uint8_t)((u_ax >> 8) & 0xff);
        buff[index++] = (uint8_t)((u_ax) & 0xff);

        uint16_t u_ay = imu_ay * 10;
        buff[index++] = (uint8_t)((u_ay >> 8) & 0xff);
        buff[index++] = (uint8_t)((u_ay) & 0xff);

        uint16_t u_az = imu_az * 10;
        buff[index++] = (uint8_t)((u_az >> 8) & 0xff);
        buff[index++] = (uint8_t)((u_az) & 0xff);
    }
    break;

    case MSG_RECEIVED:
    {
        command_t *cmd = (command_t *)data;
        buff[index++] = cmd->cmd;
        buff[index++] = cmd->id;
        buff[index++] = cmd->size;

        for (int i = 0; i < cmd->size; i++)
            buff[index++] = cmd->data[i];
    }
    break;

    case MSG_SENT:
        break;

    case SYSTEM_ERROR:
        break;

    case STATE_CHANGE:
        buff[index++] = *(uint8_t *)(data);
        break;

    case EVENT_REACTION:
        break;

    default:
        break;
    }

    file.write(buff, index);
}

void dump_log(uint16_t id)
{
    if (log_running)
    {
        char arr[] = {0,0,0,0};
        Serial.write(arr, 4);
        printf("error, cannot dump flash while logging\n");
        return;
    }

    char filename[100];
    sprintf(filename, LOG_NAME_PATTERN_READ, id);

    File flashDump = SD.open(filename);
    if (!flashDump)
    {
        char arr[] = {0,0,0,0};
        Serial.write(arr, 4);
        printf("error openening log file\n");
        return;
    }

    delay(50);
    // first send the size of the file
    Serial.write((flashDump.size() >> 24) & 0xff);
    Serial.write((flashDump.size() >> 16) & 0xff);
    Serial.write((flashDump.size() >> 8) & 0xff);
    Serial.write((flashDump.size() & 0xff));

    while (flashDump.available())
    {
        Serial.write(flashDump.read());
    }

    // Serial.printf("Flash dump size: %d\n", flashDump.size());
    flashDump.close();
}

void get_log_ids(uint16_t *files, uint16_t *files_index)
{
    *files_index = 0;

    File root = SD.open("/");
    //Serial.printf("Get files from fs %x\n", root);
    while (1)
    {
        File entry = root.openNextFile();
        if (!entry)
            break; // no more files

        if (!entry.isDirectory())
        {
            uint16_t temp = 0;
            sscanf(entry.name(), LOG_NAME_PATTERN_READ, &temp);
            files[*files_index] = temp;
            Serial.printf("file: %u %u\n", files[*files_index], temp);
            (*files_index)+=1;
        }

        Serial.print("Name in sd: ");
        Serial.print(entry.name());
        Serial.print(" ");
        Serial.print(*files_index);
        Serial.print(" size: ");
        Serial.print(entry.size());
        Serial.print(" \n");

        entry.close();
    }

    root.close();
}