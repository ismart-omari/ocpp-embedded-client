#pragma once
#include <Arduino.h>
#include "mjson.h"
#include "Requests/PendingCalls.h"
#include "Requests/BootNotificationReq.h"
#include "Requests/StatusNotificationReq.h"
#include "Requests/StartTransactionReq.h"
#include "Requests/Message.h"
#include "Requests/CallResult.h"
#include "Requests/Call.h" 
#include "Requests/CallError.h"
#include "JSONParser.h"


using std::string;

int Sender(const char *frame, int frame_len, void *privdata);

class Device;
class Message;

class OCPP_Client {
  public:
    struct jsonrpc_ctx ctx;
    // static void ocpp_cb(struct jsonrpc_request *r);
    // static int CTXInitCallback(const char *buf, int len, void *pivate_data) {};
    // static int Sender(const char* frame, int frame_len, void* privdata);
   
    Device * device_;
    OCPP_Client(Device * device);
    ~OCPP_Client();
    void SetDevice(Device * device);
    uint32_t counter = 0;
    void SendBootNotification();
    void SendStatusNotification(int connector, string error, string status);
    void SendStartTransaction(int connector, string idTag, int meterStart, string timestamp);
    void SendCall(PendingCall * call);
    void ReSendCall(PendingCall* call);
    void SendCallResult(Msg & msg); // TODO HERE
    void Update();
    PendingCalls * pending_calls_;
    BootNotificationReq * boot_notification_req;
    StatusNotificationReq * status_notification_req;
    StartTransactionReq * start_transaction_req;
    Message * message;

};