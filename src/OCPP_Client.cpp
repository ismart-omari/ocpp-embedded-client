#include "OCPP_Client.h"

using std::string;
extern HardwareSerial comser;
extern HardwareSerial logser;

int check_if_frame_is_complete(char * buf, int len, string & frame) {
    int n = mjson(buf, len, NULL, NULL);
    if (n == 0) {
        // Not full JSON object was buffered
        return 0; 
    } else if (n < 0) {
        //handle_error(); // Invalid JSON
        return -1;
    } else {
        // JSON object length is `n`. Parse, and then remove n bytes from the input
        frame.assign(buf, n);
        return 1;
    }
}

int Sender(const char *frame, int frame_len, void *privdata) {
    return comser.write(frame, frame_len);
}

OCPP_Client::OCPP_Client() {//(Device * device): device_(device){
    jsonrpc_init(NULL, NULL);
    pending_calls_ = new PendingCalls();
    boot_notification_req = new BootNotificationReq();
    boot_notification_conf = new BootNotificationConf();
}

void OCPP_Client::setDevice(Device * device) {
    device_ = device;
}

void OCPP_Client::SendBootNotification() {
    Call *call = new Call(boot_notification_req->Action, boot_notification_req->GetPayload());
    SendCall(call);
}

void OCPP_Client::Update(){
    static char buf[200];
    static int len;
    static string msg;

    pending_calls_->Update();

    if (comser.available()) {
        char ch = comser.read();
        buf[len++] = ch;
        int res = check_if_frame_is_complete(buf, len, msg);
        if(res>0) {
            logser.println(msg.c_str());
            std::fill_n(buf, 200, 0);
            len = 0;
            ProcessMessage(msg);
        }        
    } 
    
}

void OCPP_Client::ProcessCall(string & msg) {
    string payload;
    if(!GetPayload(msg, payload)) {
        logser.println("Payload not found!");
        return;
    }
    logser.println(msg.c_str());
}

void OCPP_Client::ProcessCallResult(string & msg) {
    string id;
    string payload = "{\"status\":\"Accepted\",\"currentTime\":\"2021-01-19T19:07:39.537Z\",\"interval\":14400}";
    string action;

    if(!GetUniqueId(msg, id)) return;
    // if(!GetPayload(msg, payload)) return; // TODO handle null

    bool result = pending_calls_->GetCallActionWithId(id, action);
    if (result) {
        // logser.printf("got result for id:%s payload:%s action:%s", id.c_str(), payload.c_str(), action.c_str());
        if (action == "BootNotification") {
            logser.println("got response for BootNotification");
            boot_notification_conf->MessageHandler(payload);
        }
    }
    else logser.println("Unknown response");
}

void OCPP_Client::ProcessCallError(string & msg) {
    logser.println(msg.c_str());
}

void OCPP_Client::SendCall(Call* call) {
    // send the call
    mjson_printf(Sender, NULL, call->call_format, call->MessageTypeId, call->UniqueId.c_str(), call->Action.c_str(), call->Payload.c_str());
    logser.print("sent msg");
    logser.println(call->UniqueId.c_str());
    pending_calls_->StoreCall(call);
    
}

bool OCPP_Client::GetUniqueId(string & msg, string & UniqueId) {
    char buff[100];
    int result = mjson_get_string(msg.c_str(), msg.length(), "$[1]", buff, sizeof(buff));
    if (result == 0) {
        // (r, result, "Failed to parse the request", NULL);
        return false;
    }
    UniqueId = buff;
    return true;
}

bool OCPP_Client::GetPayload(string & msg, string & Payload) {
    const char *p;
    int n;
    if (mjson_find(msg.c_str(), msg.length(), "$[2]", &p, &n) == MJSON_TOK_OBJECT) {
        Payload.assign(p, n);
        return true;
    }
    else {
        // jsonrpc_return_error(r, 1, "Payload not found", NULL);
        return false;
    }
}

void OCPP_Client::ProcessMessage(string & msg) {
    double MessageTypeId;

    if (!mjson_get_number(msg.c_str(), msg.length(), "$[0]", &MessageTypeId)) {
        logser.println("message type not found");
        return;
    }
    
    if (MessageTypeId == CALLRESULT) {
        logser.println("got CALLRESULT");
        ProcessCallResult(msg);
    }

    if (MessageTypeId == CALLERROR) {
        logser.println("got CALLERROR");
        ProcessCallError(msg);
    }
    if (MessageTypeId == CALL) {
        logser.println("got CALL");
        ProcessCall(msg);
    }
}