#pragma once
#include "Device.h"
#include "Requests/AbstractHandler.h"
#include "Types/Types.h"
#include "JSONParser.h"
#include "States/State.h"

class StartTransactionConf : public AbstractHandler {
    public:   
        Msg Handle(Msg & msg) override;
};

