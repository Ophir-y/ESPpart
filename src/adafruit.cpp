#include "adafruit.h"

// Reset the PN532 chip and initialize, set initSuccess = true on success
void InitReader(bool showError)
{
	if (showError)
	{
		Utils::Print("Communication Error -> Reset PN532\r\n");
	}

	do
	{

		initSuccess = false;

		// Reset the PN532
		nfc_PN532.begin(); // delay > 400 ms

		byte IC, VersionHi, VersionLo, Flags;
		if (!nfc_PN532.GetFirmwareVersion(&IC, &VersionHi, &VersionLo, &Flags))
		{
			SerialClass::Print("GetFirmwareVersion Failed.\r\n");
			break;
		}

		char Buf[80];
		sprintf(Buf, "Chip: PN5%02X, Firmware version: %d.%d\r\n", IC, VersionHi, VersionLo);
		Utils::Print(Buf);
		sprintf(Buf, "Supports ISO 14443A:%s, ISO 14443B:%s, ISO 18092:%s\r\n", (Flags & 1) ? "Yes" : "No",
				(Flags & 2) ? "Yes" : "No",
				(Flags & 4) ? "Yes" : "No");
		Utils::Print(Buf);

		// Set the max number of retry attempts to read from a card.
		// This prevents us from waiting forever for a card, which is the default behaviour of the PN532.
		if (!nfc_PN532.SetPassiveActivationRetries())
			break;

		// configure the PN532 to read RFID tags
		if (!nfc_PN532.SamConfig())
			break;

		initSuccess = true;
	} while (false);
}

// If the card is personalized -> authenticate with SECRET_PICC_MASTER_KEY,
// otherwise authenticate with the factory default DES key.
bool AuthenticatePICC(byte *pu8_KeyVersion)
{

	if (!nfc_PN532.SelectApplication(APPLICATION_ID)) // PICC level
		return false;

	if (!nfc_PN532.GetKeyVersion(1, pu8_KeyVersion)) // Get version of PICC master key
		return false;

	// The factory default key has version 0, while a personalized card has key version CARD_KEY_VERSION
	if (*pu8_KeyVersion == CARD_KEY_VERSION)
	{

		if (!nfc_PN532.Authenticate(1, &nfc_PiccMasterKey))
			return false;
	}
	else
	{ // The card is still in factory default state

		if (!nfc_PN532.Authenticate(1, &nfc_PN532.DES2_DEFAULT_KEY))
			return false;
	}
	return true;
}

// Reads the card in the RF field.
// In case of a Random ID card reads the real UID of the card (requires PICC authentication)
// ATTENTION: If no card is present, this function returns true. This is not an error. (check that pk_Card->u8_UidLength > 0)
// pk_Card->u8_KeyVersion is > 0 if a random ID card did a valid authentication with SECRET_PICC_MASTER_KEY
// pk_Card->b_PN532_Error is set true if the error comes from the PN532.
bool ReadCard(byte u8_UID[8], kCard *pk_Card)
{

	memset(pk_Card, 0, sizeof(kCard));

	if (!nfc_PN532.ReadPassiveTargetID(u8_UID, &pk_Card->u8_UidLength, &pk_Card->e_CardType))
	{

		pk_Card->b_PN532_Error = true;
		return false;
	}
	char s[2] = {'a' + pk_Card->e_CardType};

	if (pk_Card->e_CardType != CARD_Unknown)
	{ // In our case we must get into this condition

		if (!AuthenticatePICC(&pk_Card->u8_KeyVersion))
			return false;

		// replace the random ID with the real UID
		if ((pk_Card->e_CardType == CARD_DesRandom) && (!nfc_PN532.GetRealCardID(u8_UID)))
			return false;

		pk_Card->u8_UidLength = 7; // We use Desfir so we use 7 bytes. random ID is only 4 bytes
	}
	return true;
}

// returns true if the cause of the last error was a Timeout.
// This may happen for Desfire cards when the card is too far away from the reader.
bool IsDesfireTimeout()
{

	// For more details about this error see comment of GetLastPN532Error()
	if (nfc_PN532.GetLastPN532Error() == 0x01)
	{ // Timeout

		Utils::Print("A Timeout mostly means that the card is too far away from the reader.\r\n");

		return true;
	}
	return false;
}

int TrigerRfid()
{
	if (!initSuccess)
	{
		InitReader(true); // true for printing message to serial
		return -1;
	}

	kUser k_User; // user data struct: name and ID
	kCard k_Card; // PICC data struct: UID length, key version, card type

	if (!ReadCard(k_User.ID.u8, &k_Card))
	{
		if (IsDesfireTimeout())
			; // Prints the error inside the function

		else if (k_Card.b_PN532_Error)
		{

			InitReader(true); // for another error (but timeout) reset the chip
		}

		return -1;
	}

	if (lastId == k_User.ID.u64) // If this is the same card (without taking it off the reader) returns
		return 0;

	// Otherwise a new card presented
	char Buf[80];

	SerialClass::Print("Start\n\n");

	// Instance for return status from RFID chip
	DESFireStatus e_Status;

	int id = 0;

	// Output data buffer
	byte dataBuffer[FILE_SIZE_IN_BYTES] = {0};

	// Checks if the presented card is MiFare Classic
	if (k_Card.u8_UidLength == 4)
	{

		// Preparing the parameters for the RFID chip
		TX_BUFFER(i_Params, 16)
		i_Params.AppendBuf(SECRET_PICC_MIFARE_CLASSIC, 6);
		i_Params.AppendBuf(k_User.ID.u8, k_Card.u8_UidLength);

		// Authentication request
		if (nfc_PN532.DataExchange(MIFARE_CMD_AUTH_A, 9, i_Params, i_Params.GetCount()))
		{

			// Sending command to the RFID chip for reading its data
			if (nfc_PN532.DataExchange(MIFARE_CMD_READ, 9, dataBuffer, 0))
			{
				e_Status = ST_Success;

				SerialClass::Print("Read Success.\r\n");

				for (int i = MIFARE_CLASSIC_BLOCK_SIZE - 1; i >= 0; i--)
				{
					if (dataBuffer[i] == 0)
						continue;
					// sprintf(Buf, "data[%d]: %x\r\n", i, dataBuffer[i]);
					// Utils::Print(Buf);
					id += (dataBuffer[i] << (8 * (MIFARE_CLASSIC_BLOCK_SIZE - 1 - i)));
				}
			}
			else
			{
				SerialClass::Print("Error: Read Failed.\r\n");
				e_Status = ST_ReadClassicFailed;
			}
		}
		else
		{

			e_Status = ST_AuthentError;
		}
	}
	else if (k_Card.u8_UidLength == 7)
	{ // Desfire

		int bytesToRead = FILE_SIZE_IN_BYTES;

		// Preparing the parameters for the RFID chip
		TX_BUFFER(i_Params, 7);				// 7 bytes for parameters
		i_Params.AppendUint8(FILE_ID);		// In our case the file ID is "0"
		i_Params.AppendUint24(0);			// Offset from the head of the file
		i_Params.AppendUint24(bytesToRead); // Amount of bytes required to read from file
		// Initialization of the data buffer and a pointer to its head
		byte *ptr = dataBuffer;

		// Sending command to the RFID chip for reading data according to the parameters
		// Returns the amount of bytes that actually read or -1 on error
		int readBytes = nfc_PN532.DataExchange(DF_INS_READ_DATA, &i_Params, ptr, bytesToRead, &e_Status, MAC_TmacRcrypt);

		if (readBytes <= 0)
		{
			SerialClass::Print("Error\r\n");
		}

		// Promoting the pointer of the data buffer to prevent overwrite data
		ptr += readBytes;

		while (e_Status == ST_MoreFrames)
		{

			if (ptr - dataBuffer > FILE_SIZE_IN_BYTES)
			{ // Sanity check to prevent segmentation fault
				SerialClass::Print("Error\r\n");
				return -1;
			}

			readBytes = nfc_PN532.DataExchange(DF_INS_ADDITIONAL_FRAME, NULL, ptr, bytesToRead, &e_Status, MAC_TmacRcrypt);
			if (readBytes <= 0)
			{
				SerialClass::Print("Error\r\n");
			}
		}

		if (e_Status == ST_Success)
		{
			SerialClass::Print("Read Success.\r\n");

			for (int i = 0; i < ptr - dataBuffer; i++)
			{
				if (dataBuffer[i] == 0)
					continue;
				// sprintf(Buf, "data[%d]: %x\r\n", i, dataBuffer[i]);
				// Utils::Print(Buf);
				id += (dataBuffer[i] << (8 * i));
			}
		}
	}
	else
	{ // No card was presented
		return 0;
	}

	if (e_Status == ST_Success)
	{

		sprintf(Buf, "Presented ID: %d\r\n", id);
		Utils::Print(Buf);
		return id;
	}
	else
	{
		SerialClass::Print("Read Failed.\r\n");
		return -1;
	}
}
