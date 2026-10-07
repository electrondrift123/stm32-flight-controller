#include "main_rx.h"
#include <Arduino.h>
#include "pin_config.h"

// External SBUS inverter required before USART1 RX (PA10).
// PA9 is allocated as TX by Uart but is left unconnected.
static Uart receiverSerial(RC_SBUS_RX_PIN, RC_SBUS_TX_PIN);
static SbusParser parser;
static RcInput receiver;

bool main_rx_init(void) {
  receiverSerial.begin(100000, SERIAL_8E2);
  Serial.println("MC7RE-V2 SBUS on PA10: 100000 baud, 8E2, external inverter");
  return true; // UART setup only; link remains unsafe until healthy frames arrive.
}

RcCommands main_rx_poll(void) {
  SbusFrame frame;
  while (receiverSerial.available() > 0) {
    if (parser.push(uint8_t(receiverSerial.read()), frame)) {
      receiver.update(frame, millis());
#ifdef RC_INPUT_DIAGNOSTICS
      static uint32_t lastPrint = 0;
      if (uint32_t(millis() - lastPrint) >= 200) {
        lastPrint = millis();
        Serial.print("SBUS CH1..6:");
        for (uint8_t i = 0; i < 6; ++i) {
          Serial.print(' ');
          Serial.print(frame.channels[i]);
        }
        Serial.print(" lost="); Serial.print(frame.lost);
        Serial.print(" failsafe="); Serial.println(frame.failsafe);
      }
#endif
    }
  }
  return receiver.read(millis());
}

