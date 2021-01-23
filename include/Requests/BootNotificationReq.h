#pragma once
#include <Arduino.h>

extern "C" {
  #include "mjson.h"
}
using namespace std;
#include "Requests/AbstractHandler.h"

class BootNotificationReq : public AbstractHandler {
    private:
        string chargePointModel_;
        string chargePointVendor_;
        string chargePointSerialNumber_;
        string firmwareVersion_;
        string iccid_;
        string imsi_;
        string meterSerialNumber_;
        string meterType_;

    public:
        BootNotificationReq();
        // ~BootNotificationReq();
        string Payload();
        const string Action = "BootNotification";

        /*
        * {"chargePointModel": "modelX", "chargePointVendor": "vendorX"}
        */
        const char * payload_format = "{%Q: %Q, %Q: %Q}";
        // string & Handle(string & msg);
};