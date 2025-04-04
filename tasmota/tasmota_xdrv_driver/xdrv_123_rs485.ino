#ifdef USE_RS485

#define XDRV_123 123

#include <TasmotaModbus.h>


struct RS485t
{
    bool active = false;
    uint8_t tx = 0;
    uint8_t rx = 0;
    TasmotaModbus *Rs485Modbus = nullptr;
    //uint32_t sensor_active[4];
    int requestSent[3] = {0,0,0};
    
    uint32_t lastRequestTime; 
}RS485;

void Rs485Init(void)
{
    RS485.active = false;
    if(PinUsed(GPIO_RS485_RX) && PinUsed(GPIO_RS485_TX))
    {
        if(RS485.active)
        {
            AddLog(LOG_LEVEL_ERROR, "RS485: RS485 serial can be configured only on 1 time");
            
        }
        if(TasmotaGlobal.rs485_enabled)
        {
            AddLog(LOG_LEVEL_ERROR, "RS485: RS485 serial failed because RX/TX already configured");
        }
        else
        {
            RS485.rx = Pin(GPIO_RS485_RX);
            RS485.tx = Pin(GPIO_RS485_TX);
            RS485.active = true;
        }
    }

    if(RS485.active)
    {
        RS485.Rs485Modbus = new TasmotaModbus(RS485.rx,RS485.tx);
        uint8_t result = RS485.Rs485Modbus -> Begin();
        if(result)
        {
            if (2 == result)
            {
                ClaimSerial();
            }
            TasmotaGlobal.rs485_enabled = true;
            AddLog(LOG_LEVEL_INFO, PSTR("RS485: RS485 using GPIO%i(RX) and GPIO%i(TX)"),RS485.rx,RS485.tx);
        }
        else
        {
            delete RS485.Rs485Modbus;
            RS485.Rs485Modbus = nullptr;
            RS485.active = false;
        }

    }
}


bool Xdrv123(uint32_t function)
{
    bool result = false;
    if(FUNC_PRE_INIT == function)
    {
        Rs485Init();
    }
    else if(RS485.active)
    {
        switch (function)
        {
            case FUNC_ACTIVE:
                result = true;
                break;
        }
    }
    return result;
}

#endif