
#ifndef ADAFRUIT_H
#define ADAFRUIT_H

#include "Desfire.h"
#include "Secrets.h"
#include "UserManager.h"

#define RESET_PIN 16
#define SPI_CS_PIN 4
#define FILE_ID 0
#define FILE_SIZE_IN_BYTES 48
#define APPLICATION_ID 0x334455
#define MIFARE_CLASSIC_BLOCK_SIZE 16

extern AES nfc_PiccMasterKey;
extern Desfire nfc_PN532;
extern uint64_t lastId;
extern bool initSuccess;

struct kCard
{
    byte u8_UidLength;  // UID = 4 or 7 bytes
    byte u8_KeyVersion; // for Desfire random ID cards
    bool b_PN532_Error; // true -> the error comes from the PN532, false -> crypto error
    eCardType e_CardType;
};

// Reset the PN532 chip and initialize, set initSuccess = true on success
void InitReader(bool showError);

// If the card is personalized -> authenticate with SECRET_PICC_MASTER_KEY,
// otherwise authenticate with the factory default DES key.
bool AuthenticatePICC(byte *pu8_KeyVersion);

// Reads the card in the RF field.
// In case of a Random ID card reads the real UID of the card (requires PICC authentication)
// ATTENTION: If no card is present, this function returns true. This is not an error. (check that pk_Card->u8_UidLength > 0)
// pk_Card->u8_KeyVersion is > 0 if a random ID card did a valid authentication with SECRET_PICC_MASTER_KEY
// pk_Card->b_PN532_Error is set true if the error comes from the PN532.
bool ReadCard(byte u8_UID[8], kCard *pk_Card);

// returns true if the cause of the last error was a Timeout.
// This may happen for Desfire cards when the card is too far away from the reader.
bool IsDesfireTimeout();

// Trigerrs the RFID chip
// Returns the user ID or -1 on error
int TrigerRfid();

#endif // ADAFRUIT_H
