#include <Arduino.h>
#include "Pins.h"
#include "OCPP_Client.h"


extern OCPP_Client * client;

void setup() {
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, HIGH);
  pinMode(ERROR_LED, OUTPUT);
  jsonrpc_init(NULL, NULL);
  
  Serial1.begin(9600);
  Serial1.println("SPLC-T1 ready");
  client = new OCPP_Client();
}



void loop() {
  client->Update();
}