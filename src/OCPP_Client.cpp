#include "OCPP_Client.h"
#include "mjson.h"

using std::string;

void OCPP_Client::ocpp_cb(struct jsonrpc_request *r) {
    auto *self = static_cast<OCPP_Client *>(r->userdata);
    self->ProcessMessage(r);
}

// Gets called by the RPC engine to send a reply frame
int OCPP_Client::Sender(const char *frame, int frame_len, void *privdata) {
  return Serial1.write(frame, frame_len);
}


OCPP_Client::OCPP_Client(){
    // jsonrpc_ctx_init(&ctx, emptyfunc, nullptr);
    jsonrpc_ctx_init(&ctx, &CTXInitCallback, NULL);

    pending_calls_ = new PendingCalls;
    boot_notification_req = new BootNotificationReq();
    boot_notification_conf = new BootNotificationConf();
    // call = new Call();
    jsonrpc_ctx_export(&ctx, "OCPP.Message", ocpp_cb, this);
    
    Call * call = new Call(boot_notification_req->Action, boot_notification_req->GetPayload());
    SendCall(call);

    jsonrpc_ctx_init(&ctx, NULL, NULL);
    jsonrpc_ctx_export(&ctx, "OCPP.Message", OCPP_Client::ocpp_cb, this);
}

void OCPP_Client::Update(){
    if (Serial1.available() > 0) 
        jsonrpc_ctx_process_byte(&ctx, Serial1.read(), OCPP_Client::Sender, NULL);
}


void OCPP_Client::ProcessCallResult(struct jsonrpc_request *r) {
    string id;
    string payload;
    string action;

    if(!GetUniqueId(r, id)) return;
    if(!GetPayload(r, payload)) return; // TODO handle null
    bool result = pending_calls_->GetCallActionWithId(id, action);
    if (!result) {
        jsonrpc_return_error(r, result, "Unknown response", payload.c_str());
        return;
    }
    jsonrpc_return_success(r, "%Q %Q %Q %Q", "got result for id", id.c_str(), payload.c_str(), action); 

    // if (action == "BootNotification") boot_notification_conf->Parse(payload);
}

void OCPP_Client::ProcessCallError(struct jsonrpc_request *r) {}

void OCPP_Client::SendCall(Call * call) {
    // send the call
    mjson_printf(Sender, NULL, call->call_format, call->MessageTypeId, call->UniqueId.c_str(), call->Action.c_str(), call->Payload.c_str());
    pending_calls_->StoreCall(call);
    
}

bool OCPP_Client::GetUniqueId(struct jsonrpc_request *r, string & UniqueId) {
    char buff[100];
    int result = mjson_get_string(r->params, r->params_len, "$[1]", buff, sizeof(buff));
    if (result == 0) {
        jsonrpc_return_error(r, result, "Failed to parse the request", NULL);
        return false;
    }
    UniqueId = buff;
    return true;
}

bool OCPP_Client::GetPayload(struct jsonrpc_request *r, string & Payload) {
    const char *p;
    int n;
    if (mjson_find(r->params, r->params_len, "$[2]", &p, &n) == MJSON_TOK_OBJECT) {
        Payload.assign(p, n);
        return true;
    }
    else {
        jsonrpc_return_error(r, 1, "Payload not found", NULL);
        return false;
    }
}

void OCPP_Client::ProcessMessage(struct jsonrpc_request *r) {
    double MessageTypeId;

    if (!mjson_get_number(r->params, r->params_len, "$[0]", &MessageTypeId))  
        return;

    if (MessageTypeId == CALLRESULT) {
        ProcessCallResult(r);
    }

    if (MessageTypeId == CALLERROR) {
        ProcessCallError(r);
    }
}