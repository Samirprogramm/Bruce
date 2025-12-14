#include "rf_jammer.h"
#include "core/display.h"
#include "rf_utils.h"

RFJammer::RFJammer(bool full) : fullJammer(full) { setup(); }

RFJammer::~RFJammer() { deinitRfModule(); }

void RFJammer::setup() {
    nTransmitterPin = bruceConfig.rfTx;
    if (!initRfModule("tx")) return;

    if (bruceConfig.rfModule == CC1101_SPI_MODULE) {
        nTransmitterPin = bruceConfigPins.CC1101_bus.io0;
    }

    sendRF = true;
    returnToMenu = false;

    display_banner();

    if (fullJammer) return run_full_jammer();
    else run_itmt_jammer();
}

void RFJammer::display_banner() {
    drawMainBorderWithTitle("RF Jammer");
    printSubtitle(String(fullJammer ? "Full Jammer" : "Intermittent Jammer"));

    padprintln("Sending...");
    padprintln("");
    padprintln("");

    tft.setTextColor(getColorVariation(bruceConfig.priColor), bruceConfig.bgColor);
    padprintln("Press [ESC] to stop.");
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
}

void RFJammer::run_full_jammer() {
    digitalWrite(nTransmitterPin, HIGH); // Jammer ON

    while (sendRF) {
        if (check(EscPress)) {
            sendRF = false;
            returnToMenu = true;
            break;
        }
        yield(); // Watchdog verhindern
    }

    digitalWrite(nTransmitterPin, LOW); // Jammer OFF
}

void RFJammer::run_itmt_jammer() {
    int tmr0 = millis();

    while (sendRF) {
        for (int sequence = 1; sequence < 50; sequence++) {
            for (int duration = 1; duration <= 3; duration++) {
                if (check(EscPress) || (millis() - tmr0) > 20000) {
                    sendRF = false;
                    returnToMenu = true;
                    break;
                }
                digitalWrite(nTransmitterPin, HIGH);
                for (int widthsize = 1; widthsize <= (1 + sequence); widthsize++) { delayMicroseconds(10); }

                digitalWrite(nTransmitterPin, LOW);
                for (int widthsize = 1; widthsize <= (1 + sequence); widthsize++) { delayMicroseconds(10); }
            }
            if (!sendRF) break;
        }
    }
    digitalWrite(nTransmitterPin, LOW);
}
