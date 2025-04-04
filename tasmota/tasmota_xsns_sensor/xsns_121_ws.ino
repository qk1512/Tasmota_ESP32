#ifdef USE_RS485
#ifdef USE_WDS

#define XSNS_121 121

struct WDSt
{
    bool valid = false;
    int wind_direction;
    char name[15] = "WIND DIRECTION";
}WDS;

bool WDSisConnected()
{
    if(!RS485.active) return false;

    RS485.Rs485Modbus -> Send(0x01, 0x03, (0x00 << 8) | 0x01, (0x00 << 8) | 0x01);

    uint32_t start_time = millis();
    /* while(!RS485.Rs485Modbus -> ReceiveReady())
    {
        if(millis() - start_time > 200) return false;
        yield();
    } */

    delay(200);
    if (!RS485.Rs485Modbus->ReceiveReady())
        return false;

    uint8_t buffer[8];
    uint8_t error = RS485.Rs485Modbus -> ReceiveBuffer(buffer,8);
    if(error)
    {
        AddLog(LOG_LEVEL_INFO,PSTR("error %d"),error);
        return false;
    }
    else
    {
        if(buffer[0] == 0x01) return true;
        return false;
    }
    return false;
    //return(RS485.Rs485Modbus -> ReceiveBuffer(buffer, 8) == 0 && buffer[0] == 0x01);
}

void WDSInit(void)
{
    if(!RS485.active)
    {
        //AddLog(LOG_LEVEL_INFO, PSTR(""))
        return;
    }

    WDS.valid = WDSisConnected();
    AddLog(LOG_LEVEL_INFO, PSTR(WDS.valid ? "Wind Direction is connected" : "Wind Direction is not detected"));
} 

void WDSReadData(void)
{

    if(WDS.valid == false) return;

    //static uint32_t lastRequestTime_WDS = 0;
    //static bool requestSent_WDS = false;

    for (int i = 0; i < 3; i++)
    {
        if (RS485.requestSent[0] == 1)
            continue;
        else if (RS485.requestSent[i] == 1)
            return;
    }

    if (RS485.requestSent[0] == 0 && RS485.lastRequestTime == 0)
    {
        RS485.Rs485Modbus->Send(0x01, 0x03, 0x0001, 0x0001);
        //lastRequestTime_WDS = millis();
        //requestSent_WDS = true;
        RS485.requestSent[0] = 1;
        RS485.lastRequestTime = millis();
    }

    if ((RS485.requestSent[0] == 1) && millis() - RS485.lastRequestTime >= 200)
    {
        if (RS485.Rs485Modbus->ReceiveReady())
        {
            uint8_t buffer[8];
            uint8_t error = RS485.Rs485Modbus->ReceiveBuffer(buffer, 8);

            if (error)
            {
                AddLog(LOG_LEVEL_INFO, PSTR("Modbus WDS Error: %u"), error);
            }
            else if (buffer[0] == 0x01) // Ensure response is from the correct slave ID
            {
                uint16_t wind_directionRaw = (buffer[3] << 8) | buffer[4];
                WDS.wind_direction = wind_directionRaw / 10.0;
                //current_sensor = (current_sensor + 1) % MAX_SENSORS; // Switch to the next sensor
            }
            //requestSent_WDS = false; // Reset for the next cycle
            RS485.requestSent[0] = 0;
            RS485.lastRequestTime = 0;
        }
        else
        {
            // AddLog(LOG_LEVEL_INFO,PSTR("Can not read Data"));
        }
    }
}

/* void WDSReadData(void)
{
    if(WDS.valid == false) return;

    //if(current_sensor != 0) return;

    RS485.Rs485Modbus -> Send(0x01, 0x03, 0x0001, 0x0001);
    delay(100);
    if(RS485.Rs485Modbus -> ReceiveReady())
    {
        uint8_t buffer[8];
        uint8_t error = RS485.Rs485Modbus -> ReceiveBuffer(buffer, 8);

        if(error)
        {
            AddLog(LOG_LEVEL_INFO, PSTR("Modbus WDS Error: %u"), error);
        }
        else if(buffer[0] == 0x01)
        {
            uint16_t wind_directionRaw = (buffer[3] << 8) | buffer[4];
            WDS.wind_direction = wind_directionRaw/10.0;   
        }
    }

    //current_sensor = 1;
} */

const char HTTP_SNS_WDS[] PROGMEM = "{s} Wind Direction {m} %d";
#define D_JSON_WDS "WindDirection"

void WDSShow(bool json)
{
    if(json)
    {
        ResponseAppend_P(PSTR(",\"%s\":{"), WDS.name);
        ResponseAppend_P(PSTR("\"" D_JSON_WDS "\":%d"), WDS.wind_direction);
        ResponseJsonEnd();
    }
#ifdef USE_WEBSERVER
    else
    {
        WSContentSend_PD(HTTP_SNS_WDS, WDS.wind_direction);
    }
#endif
}


bool Xsns121(uint32_t function)
{
    bool result = false;
    if(FUNC_INIT == function)
    {
        WDSInit();
    }
    else if(WDS.valid)
    {
        switch (function)
        {
        case FUNC_EVERY_250_MSECOND:
            //if(RS485.requestSent[1] == 1) break;
            WDSReadData();
            //ModbusPoll();
            break;
        case FUNC_JSON_APPEND:
            WDSShow(1);
            break;
#ifdef USE_WEBSERVER
        case FUNC_WEB_SENSOR:
            WDSShow(0);
            break;
#endif
        }
    }
    return result;
}

#endif
#endif