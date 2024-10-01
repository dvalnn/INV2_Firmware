#include "FlashLog.h"

#include "HardwareCfg.h"
#include "GlobalVars.h"

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
        buff[index++] = (Tank_Top_Module.temperature >> 8) & 0xff;
        buff[index++] = (Tank_Top_Module.temperature) & 0xff;
        buff[index++] = (Tank_Bot_Module.temperature >> 8) & 0xff;
        buff[index++] = (Tank_Bot_Module.temperature) & 0xff;

        int16_t ipressure = (int16_t)(Tank_Top_Module.pressure * 100);
        buff[index++] = (ipressure >> 8) & 0xff;
        buff[index++] = (ipressure) & 0xff;

        ipressure = (int16_t)(Tank_Bot_Module.pressure * 100);
        ipressure = Tank_Bot_Module.pressure * 100;
        buff[index++] = (ipressure >> 8) & 0xff;
        buff[index++] = (ipressure) & 0xff;

        buff[index++] = (tank_pressure >> 8) & 0xff;
        buff[index++] = (tank_pressure) & 0xff;

        int16_t itank_liquid = (int16_t)(tank_liquid * 10000);
        buff[index++] = (itank_liquid >> 8) & 0xff;
        buff[index++] = (itank_liquid) & 0xff;

        buff[index++] = (uint8_t)((log_running << 7) |
                                  (Tank_Top_Module.valve_state << 6) |
                                  (Tank_Bot_Module.valve_state << 5));

        ipressure = (int16_t)(Chamber_Module.pressure * 100);
        buff[index++] = (ipressure >> 8) & 0xff;
        buff[index++] = (ipressure) & 0xff;

        int16_t ialtura = (int16_t)(hL * 100);
        buff[index++] = (ialtura >> 8) & 0xff;
        buff[index++] = (ialtura) & 0xff;

        int16_t iVl = (int16_t)(Vl * 1000);
        buff[index++] = (iVl >> 8) & 0xff;
        buff[index++] = (iVl) & 0xff;

        int16_t iml = (int16_t)(ml * 100);
        buff[index++] = (iml >> 8) & 0xff;
        buff[index++] = (iml) & 0xff;

        int16_t he_moles_i = (int16_t)(he_mol * 10);
        buff[index++] = (he_moles_i >> 8) & 0xff;
        buff[index++] = (he_moles_i) & 0xff;

        int16_t tank_mol_lost_i = (int16_t)(tank_mol_lost * 10);
        buff[index++] = (tank_mol_lost_i >> 8) & 0xff;
        buff[index++] = (tank_mol_lost_i) & 0xff;
        
        int16_t hL_i = (int16_t)(he_mol * 10);
        buff[index++] = (hL_i >> 8) & 0xff;
        buff[index++] = (hL_i) & 0xff;

        int16_t ml_i = (int16_t)(tank_mol_lost * 10);
        buff[index++] = (ml_i >> 8) & 0xff;
        buff[index++] = (ml_i) & 0xff;
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