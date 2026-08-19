#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TABLE_SIZE 20
#define MIN_PORT 1024
#define MAX_PORT 65535
#define PPP_FLAG 0x7E  // 01111110
#define PPP_ESC  0x7D  // 01111101

typedef struct {
    char url[100];
    char ip_address[16];
    char mac_address[18];
    int is_Occupied;
} HashEntry;

// Function Prototypes (Existing)
int hashFunction(char domain[]);
void insertEntry(HashEntry table[], char domain[], char ip[], char mac[]);
void displayEntryTable(HashEntry table[]);
void updateEntry(HashEntry table[], char domain[], char ip[], char mac[]);
void deleteEntry(HashEntry table[], char domain[]);
int searchEntry(HashEntry table[], char domain[], char ipOut[], char macOut[]);
int readFile(char *filename, char message[]);
void print8BitBinary(unsigned char value);
void print16BitBinary(unsigned short value);
void getIPBinary(char ip[], char output[]);
void getMACBinary(char mac[], char output[]);

// New Framing Protocol Function Prototypes
void processPerFrameBitStuffing(int totalFrames, char message[], int messageLength, char srcMACBin[], char destMACBin[], char srcIPBin[], char destIPBin[]);
void processUnifiedBitStuffing(char dataLinkFinal[]);
void processPPPByteStuffing(char message[], int messageLength);

// PPP with checksum
unsigned short calculateChecksum(unsigned char *buf, int count);
int pppByteStuffing(unsigned char *input, int inputLen, unsigned char *output);
int pppByteDestuffing(unsigned char *input, int inputLen, unsigned char *output);
void runPPPChecksumSimulation(unsigned char *message, int messageLen);
// 2D parity checking
void processDynamic2DParity(char originalText[], char dataLinkFinalBits[], int totalMessageBits);
void calculate2DParitySender(char data[20][7], int totalRows, int rowParity[], int colParity[], int *intersectParity);
void verify2DParityReceiver(char data[20][7], int totalRows, int rowParity[], int colParity[], int intersectParity);

int hashFunction(char url[]) {
    int sum = 0;
    for (int i = 0; url[i] != '\0'; i++) {
        sum += (int)url[i];
    }
    return sum % TABLE_SIZE;
}

void insertEntry(HashEntry table[], char url[], char ip[], char mac[]) {
    int index = hashFunction(url);
    int startIndex = index;

    while (table[index].is_Occupied) {
        if (strcmp(table[index].url, url) == 0) {
            return; 
        }
        index = (index + 1) % TABLE_SIZE;
        if (index == startIndex) return;
    }
    strcpy(table[index].url, url);
    strcpy(table[index].ip_address, ip);
    strcpy(table[index].mac_address, mac);
    table[index].is_Occupied = 1;
}

void displayEntryTable(HashEntry table[]) {
    printf("\nIndex\t%-25s\t%-15s\t%s\n", "URL", "IP Address", "MAC Address");
    printf("-------------------------------------------------------------------------\n");
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (table[i].is_Occupied) {
            printf("%d\t%-25s\t%-15s\t%s\n", i, table[i].url, table[i].ip_address, table[i].mac_address);
        } else {
            printf("%d\t--- EMPTY ---\n", i);
        }
    }
}

void updateEntry(HashEntry table[], char url[], char ip[], char mac[]) {
    int index = hashFunction(url);
    int steps = 0;
    while (steps < TABLE_SIZE) {
        if (table[index].is_Occupied && strcmp(table[index].url, url) == 0) {
            strcpy(table[index].ip_address, ip);
            strcpy(table[index].mac_address, mac);
            printf("Entry updated successfully.\n");
            return; // Fixed missing semicolon here
        }
        index = (index + 1) % TABLE_SIZE;
        steps++;
    }
    printf("Domain not found.\n");
}

void deleteEntry(HashEntry table[], char url[]) {
    int index = hashFunction(url);
    int steps = 0;
    while (steps < TABLE_SIZE) {
        if (table[index].is_Occupied && strcmp(table[index].url, url) == 0) {
            table[index].is_Occupied = 0;
            printf("Entry deleted successfully.\n");
            return;
        }
        index = (index + 1) % TABLE_SIZE;
        steps++;
    }
    printf("Domain not found.\n");
}

int searchEntry(HashEntry table[], char url[], char ipOut[], char macOut[]) {
    int index = hashFunction(url);
    int steps = 0;
    while (steps < TABLE_SIZE) {
        if (table[index].is_Occupied && strcmp(table[index].url, url) == 0) {
            strcpy(ipOut, table[index].ip_address);
            strcpy(macOut, table[index].mac_address);
            return 1;
        }
        index = (index + 1) % TABLE_SIZE;
        steps++;
    }
    return 0;
}

int readFile(char *fname, char message[]) {
    FILE *fp = fopen(fname, "r");
    if (fp == NULL) {
        fp = fopen(fname, "w");
        if(fp) { fputs("Hello", fp); fclose(fp); }
        fp = fopen(fname, "r");
    }
    int count = 0;
    char ch;
    while ((ch = fgetc(fp)) != EOF && count < 999) {
        if(ch != '\r' && ch != '\n') { 
            message[count++] = ch;
        }
    }
    message[count] = '\0';
    fclose(fp);
    return count;
}

void print8BitBinary(unsigned char value) {
    for (int i = 7; i >= 0; i--) printf("%d", (value >> i) & 1);
}

void print16BitBinary(unsigned short value) {
    for (int i = 15; i >= 0; i--) printf("%d", (value >> i) & 1);
}

void getIPBinary(char ip[], char output[]) {
    int o1, o2, o3, o4;
    sscanf(ip, "%d.%d.%d.%d", &o1, &o2, &o3, &o4);
    int pos = 0;
    for (int i = 7; i >= 0; i--) output[pos++] = ((o1 >> i) % 2 != 0) ? '1' : '0';
    for (int i = 7; i >= 0; i--) output[pos++] = ((o2 >> i) % 2 != 0) ? '1' : '0';
    for (int i = 7; i >= 0; i--) output[pos++] = ((o3 >> i) % 2 != 0) ? '1' : '0';
    for (int i = 7; i >= 0; i--) output[pos++] = ((o4 >> i) % 2 != 0) ? '1' : '0';
    output[pos] = '\0';
}

void getMACBinary(char mac[], char output[]) {
    unsigned int m1, m2, m3, m4, m5, m6;
    sscanf(mac, "%x:%x:%x:%x:%x:%x", &m1, &m2, &m3, &m4, &m5, &m6);
    int pos = 0;
    for (int i = 7; i >= 0; i--) output[pos++] = ((m1 >> i) % 2 != 0) ? '1' : '0';
    for (int i = 7; i >= 0; i--) output[pos++] = ((m2 >> i) % 2 != 0) ? '1' : '0';
    for (int i = 7; i >= 0; i--) output[pos++] = ((m3 >> i) % 2 != 0) ? '1' : '0';
    for (int i = 7; i >= 0; i--) output[pos++] = ((m4 >> i) % 2 != 0) ? '1' : '0';
    for (int i = 7; i >= 0; i--) output[pos++] = ((m5 >> i) % 2 != 0) ? '1' : '0';
    for (int i = 7; i >= 0; i--) output[pos++] = ((m6 >> i) % 2 != 0) ? '1' : '0';
    output[pos] = '\0';
}

/* ============================================================
     EXTRACTED METHODS FOR FRAMING PROTOCOLS
   ============================================================ */

void processPerFrameBitStuffing(int totalFrames, char message[], int messageLength, char srcMACBin[], char destMACBin[], char srcIPBin[], char destIPBin[]) {
    printf("\n==============================================================\n");
    printf("             1. BIT STUFFING - PER FRAME PROCESSING\n");
    printf("==============================================================\n");

    for (int f = 0; f < totalFrames; f++) {
        char rawFramePayload[2000] = "";
        char bitTransmittedFrame[3000] = "";
        int packetIndex = f / 2;
        int isFirstFrameOfPacket = (f % 2 == 0);
        char frameDataBin[9] = "";
        
        if (isFirstFrameOfPacket) {
            for(int i = 7; i >= 0; i--) {
                char bit = ((message[packetIndex * 2] >> i) & 1) + '0';
                strncat(frameDataBin, &bit, 1);
            }
        } else {
            if(packetIndex * 2 + 1 < messageLength) {
                for(int i = 7; i >= 0; i--) {
                    char bit = ((message[packetIndex * 2 + 1] >> i) & 1) + '0';
                    strncat(frameDataBin, &bit, 1);
                }
            } else {
                strcpy(frameDataBin, "00000000");
            }
        }
        
        char trailerBin[] = "00000000";
        strcat(rawFramePayload, frameDataBin);
        strcat(rawFramePayload, srcMACBin);
        strcat(rawFramePayload, destMACBin);
        strcat(rawFramePayload, srcIPBin);
        strcat(rawFramePayload, destIPBin);
        strcat(rawFramePayload, trailerBin);

        printf("\n------------------------------------------------------------\n");
        printf("PROCESSING FRAME %d\n", f + 1);
        printf("------------------------------------------------------------\n");
        printf("Original Frame %d Payload Bit Stream:\n%s\n", f + 1, rawFramePayload);

        int originalBitsCount = strlen(rawFramePayload);
        int txIndex = 0;
        int consecutiveOnes = 0;
        int stuffedBitsCount = 0;

        strcpy(bitTransmittedFrame, "01111110");
        txIndex = 8;

        for (int i = 0; i < originalBitsCount; i++) {
            bitTransmittedFrame[txIndex++] = rawFramePayload[i];
            if (rawFramePayload[i] == '1') {
                consecutiveOnes++;
                if (consecutiveOnes == 5) {
                    bitTransmittedFrame[txIndex++] = '0'; 
                    stuffedBitsCount++;
                    consecutiveOnes = 0;
                }
            } else {
                consecutiveOnes = 0;
            }
        }

        strcpy(&bitTransmittedFrame[txIndex], "01111110");
        txIndex += 8;

        printf("Flag Pattern : 01111110\n");
        printf("Transmitted Bit-Oriented Frame %d:\n%s\n", f + 1, bitTransmittedFrame);
        printf("Original Data Bits     : %d\n", originalBitsCount);
        printf("Inserted Stuffed Bits  : %d\n", stuffedBitsCount);
        printf("Total Transmitted Bits : %d\n", txIndex);

        printf("\n[Receiver Side Destuffing for Frame %d]\n", f + 1);
        char bitRecoveredStream[2000] = "";
        int totalTxLength = strlen(bitTransmittedFrame);
        int rxIndex = 0;
        int rxConsecutiveOnes = 0;
        int verificationSuccess = 1;

        if (totalTxLength < 16 || strncmp(bitTransmittedFrame, "01111110", 8) != 0 || strncmp(&bitTransmittedFrame[totalTxLength - 8], "01111110", 8) != 0) {
            verificationSuccess = 0;
        } else {
            for (int i = 8; i < totalTxLength - 8; i++) {
                bitRecoveredStream[rxIndex++] = bitTransmittedFrame[i];
                if (bitTransmittedFrame[i] == '1') {
                    rxConsecutiveOnes++;
                    if (rxConsecutiveOnes == 5) {
                        if (i + 1 < totalTxLength - 8 && bitTransmittedFrame[i + 1] == '0') {
                            i++; 
                        }
                        rxConsecutiveOnes = 0; 
                    }
                } else {
                    rxConsecutiveOnes = 0;
                }
            }
            bitRecoveredStream[rxIndex] = '\0';
        }

        printf("Recovered Frame %d Payload Bit Stream:\n%s\n", f + 1, bitRecoveredStream);
        if (verificationSuccess && strcmp(rawFramePayload, bitRecoveredStream) == 0) {
            printf("FRAME %d STUFFING VERIFICATION: SUCCESS\n", f + 1);
        } else {
            printf("FRAME %d STUFFING VERIFICATION: FAILED\n", f + 1);
        }
    }
}

void processUnifiedBitStuffing(char dataLinkFinal[]) {
    printf("\n==============================================================\n");
    printf("     BIT STUFFING & DESTUFFING FOR UNIFIED DATA LINK LAYER\n");
    printf("==============================================================\n");
    
    printf("\nOriginal Data Link Layer Bit Stream:\n%s\n", dataLinkFinal);
    printf("\nFlag Pattern : 01111110\n");

    int unifiedOriginalBits = strlen(dataLinkFinal);
    char unifiedTxFrame[80000] = "";
    int utxIndex = 0;
    int uConsecutiveOnes = 0;
    int uStuffedBits = 0;

    strcpy(unifiedTxFrame, "01111110");
    utxIndex = 8;

    for (int i = 0; i < unifiedOriginalBits; i++) {
        unifiedTxFrame[utxIndex++] = dataLinkFinal[i];
        if (dataLinkFinal[i] == '1') {
            uConsecutiveOnes++;
            if (uConsecutiveOnes == 5) {
                unifiedTxFrame[utxIndex++] = '0';
                uStuffedBits++;
                uConsecutiveOnes = 0;
            }
        } else {
            uConsecutiveOnes = 0;
        }
    }
    strcpy(&unifiedTxFrame[utxIndex], "01111110");
    utxIndex += 8;

    printf("\nTransmitted Bit-Oriented Frame:\n%s\n", unifiedTxFrame);
    printf("\nOriginal Data Bits     : %d\n", unifiedOriginalBits);
    printf("Inserted Stuffed Bits  : %d\n", uStuffedBits);
    printf("Total Transmitted Bits : %d\n", utxIndex);

    printf("\n[Receiver Side Destuffing for Unified Frame]\n");
    char unifiedRecoveredStream[40000] = "";
    int urxIndex = 0;
    int urxConsecutiveOnes = 0;
    int uTotalTxLength = strlen(unifiedTxFrame);

    for (int i = 8; i < uTotalTxLength - 8; i++) {
        unifiedRecoveredStream[urxIndex++] = unifiedTxFrame[i];
        if (unifiedTxFrame[i] == '1') {
            urxConsecutiveOnes++;
            if (urxConsecutiveOnes == 5) {
                if (i + 1 < uTotalTxLength - 8 && unifiedTxFrame[i + 1] == '0') {
                    i++; 
                }
                urxConsecutiveOnes = 0;
            }
        } else {
            urxConsecutiveOnes = 0;
        }
    }
    unifiedRecoveredStream[urxIndex] = '\0';

    printf("Recovered Unified Payload Bit Stream:\n%s\n", unifiedRecoveredStream);
    if (strcmp(dataLinkFinal, unifiedRecoveredStream) == 0) {
        printf("\nBIT STUFFING VERIFICATION: SUCCESS\n");
        printf("Receiver recovered the original Data Link Layer output.\n");
    } else {
        printf("\nBIT STUFFING VERIFICATION: FAILED\n");
    }
}

void processPPPByteStuffing(char message[], int messageLength) {
    printf("\n==============================================================\n");
    printf("           2. PPP BYTE STUFFING - SENDER SIDE\n");
    printf("==============================================================\n");

    unsigned char pppCharPayload[500];
    int charPayloadLen = 0;

    for (int i = 0; i < messageLength; i++) {
        pppCharPayload[charPayloadLen++] = (unsigned char)message[i];
    }

    pppCharPayload[charPayloadLen++] = PPP_FLAG;
    pppCharPayload[charPayloadLen++] = PPP_ESC;

    printf("[Data from Upper Layer] (Raw Text Characters + Injected Control Tokens):\n");
    printf("As Text: ");
    for (int i = 0; i < charPayloadLen; i++) {
        if (pppCharPayload[i] == PPP_FLAG) printf("[FLAG] ");
        else if (pppCharPayload[i] == PPP_ESC) printf("[ESC] ");
        else printf("%c ", pppCharPayload[i]);
    }
    printf("\nAs Hex : ");
    for (int i = 0; i < charPayloadLen; i++) printf("%02X ", pppCharPayload[i]);
    printf("\n\n");

    unsigned char pppTxFrame[1000];
    int pppTxLen = 0;

    pppTxFrame[pppTxLen++] = PPP_FLAG; 
    pppTxFrame[pppTxLen++] = 0xFF;     
    pppTxFrame[pppTxLen++] = 0x03;     
    pppTxFrame[pppTxLen++] = 0x00;     
    pppTxFrame[pppTxLen++] = 0x21;     

    int stuffedCount = 0;
    for (int i = 0; i < charPayloadLen; i++) {
        if (pppCharPayload[i] == PPP_FLAG || pppCharPayload[i] == PPP_ESC) {
            pppTxFrame[pppTxLen++] = PPP_ESC;       
            pppTxFrame[pppTxLen++] = pppCharPayload[i];
            stuffedCount++;
        } else {
            pppTxFrame[pppTxLen++] = pppCharPayload[i];
        }
    }

    pppTxFrame[pppTxLen++] = 0x20;     
    pppTxFrame[pppTxLen++] = 0x26;     
    pppTxFrame[pppTxLen++] = PPP_FLAG; 

    printf("[Frame Sent] Complete PPP Frame (Hex Sequence):\n");
    for (int i = 0; i < pppTxLen; i++) printf("%02X ", pppTxFrame[i]);
    printf("\n\n");

    printf("Sender Metrics:\n");
    printf("-> Character Payload Bytes: %d\n", charPayloadLen);
    printf("-> Inserted Escape Bytes  : %d\n", stuffedCount);
    printf("-> Total Frame Size       : %d Bytes\n", pppTxLen);

    printf("\n==============================================================\n");
    printf("          PPP BYTE DESTUFFING - RECEIVER SIDE\n");
    printf("==============================================================\n");

    unsigned char pppRxPayload[500];
    int rxPayloadLen = 0;
    int isFrameValid = 1;

    if (pppTxLen < 8 || pppTxFrame[0] != PPP_FLAG || pppTxFrame[pppTxLen - 1] != PPP_FLAG) {
        isFrameValid = 0;
    } else {
        int endBoundary = pppTxLen - 3;
        for (int i = 5; i < endBoundary; i++) {
            if (pppTxFrame[i] == PPP_ESC) {
                i++; 
                pppRxPayload[rxPayloadLen++] = pppTxFrame[i]; 
            } else {
                pppRxPayload[rxPayloadLen++] = pppTxFrame[i]; // Fixed buggy duplicate increment assignment here
            }
        }
    }

    char finalCleanText[500] = "";
    int textIdx = 0;
    for (int i = 0; i < rxPayloadLen; i++) {
        if (pppRxPayload[i] != PPP_FLAG && pppRxPayload[i] != PPP_ESC) {
            finalCleanText[textIdx++] = (char)pppRxPayload[i];
        }
    }
    finalCleanText[textIdx] = '\0';

    printf("[Data to Upper Layer] Reclaimed Unstuffed Payload (Hex):\n");
    for (int i = 0; i < rxPayloadLen; i++) printf("%02X ", pppRxPayload[i]);
    printf("\n\nFinal Recovered Message Text: %s\n", finalCleanText);

    if (isFrameValid && rxPayloadLen == charPayloadLen && memcmp(pppCharPayload, pppRxPayload, charPayloadLen) == 0) {
        printf("\nPPP BYTE STUFFING VERIFICATION: SUCCESS\n");
    } else {
        printf("\nPPP BYTE STUFFING VERIFICATION: FAILED\n");
    }
    printf("==============================================================\n");
}
/* ============================================================
     CHECKSUM ALGORITHM (From slide instructions)
   ============================================================ */
unsigned short calculateChecksum(unsigned char *buf, int count) {
    unsigned int sum = 0;
    int i = 0;

    while (count > 1) {
        sum += (buf[i] << 8) | buf[i+1];
        i += 2;
        count -= 2;
    }

    if (count > 0) {
        sum += (buf[i] << 8);
    }

    while (sum >> 16) {
        unsigned int carry = sum >> 16;
        sum = (sum & 0xFFFF) + carry; 
    }

    return (unsigned short)(~sum);
}

/* ============================================================
     PPP BYTE STUFFING (With Checksum Embedded)
   ============================================================ */
int pppByteStuffing(unsigned char *input, int inputLen, unsigned char *output) {
    int outIdx = 0;
    output[outIdx++] = 0x7E; // Opening Flag
    
    for (int i = 0; i < inputLen; i++) {
        if (input[i] == 0x7E || input[i] == 0x7D) {
            output[outIdx++] = 0x7D; // Escape Byte
            output[outIdx++] = input[i] ^ 0x20;
        } else {
            output[outIdx++] = input[i];
        }
    }
    
    output[outIdx++] = 0x7E; // Closing Flag
    return outIdx; 
}

/* ============================================================
     PPP BYTE DESTUFFING (Receiver Side Recovery)
   ============================================================ */
int pppByteDestuffing(unsigned char *input, int inputLen, unsigned char *output) {
    int outIdx = 0;
    for (int i = 1; i < inputLen - 1; i++) {
        if (input[i] == 0x7D) {
            i++; 
            output[outIdx++] = input[i] ^ 0x20;
        } else {
            output[outIdx++] = input[i];
        }
    }
    return outIdx; 
}
void runPPPChecksumSimulation(unsigned char *message, int messageLen) {
    unsigned char senderBuffer[1024];
    unsigned char stuffedFrame[2048];
    unsigned char destuffedBuffer[1024];

    printf("\n==============================================================\n");
    printf("     5. PPP BYTE STUFFING WITH CHECKSUM SIMULATION            \n");
    printf("==============================================================\n");
    printf("Original Text Data: \"%s\"\n", message);

    int idx = 0;
    senderBuffer[idx++] = 0xFF; // Address
    senderBuffer[idx++] = 0x03; // Control
    senderBuffer[idx++] = 0x00; // Protocol High
    senderBuffer[idx++] = 0x21; // Protocol Low

    memcpy(&senderBuffer[idx], message, messageLen);
    idx += messageLen;

    int wasOdd = 0;
    if (idx % 2 != 0) {
        senderBuffer[idx++] = 0x00;
        wasOdd = 1;
    }

    unsigned short senderChecksum = calculateChecksum(senderBuffer, idx);
    printf("[SENDER] Calculated Checksum (FCS): 0x%04X\n", senderChecksum);

    senderBuffer[idx++] = (senderChecksum >> 8) & 0xFF;
    senderBuffer[idx++] = senderChecksum & 0xFF;

    int stuffedLength = pppByteStuffing(senderBuffer, idx, stuffedFrame);

    // --- CLEAN SINGLE-LINE PRINT FOR GENERATED FRAME ---
    printf("\nGenerated Stuffed Frame (%d bytes): ", stuffedLength);
    for (int i = 0; i < stuffedLength; i++) {
        printf("%02X ", stuffedFrame[i]);
    }
    printf("\n");

    // --- DYNAMIC USER INPUT FOR ERROR INJECTION ---
    char choice;
    printf("\nDo you want to inject a transmission error? (y/n): ");
    scanf(" %c", &choice);

    if (choice == 'y' || choice == 'Y') {
        int errIndex;
        printf("Enter the byte index to corrupt (0 to %d): ", stuffedLength - 1);
        scanf("%d", &errIndex);

        if (errIndex >= 0 && errIndex < stuffedLength) {
            stuffedFrame[errIndex] ^= 0xFF; // Flip bits at chosen index
            printf("[CHANNEL SIMULATION] Corrupted byte at index %d! New byte value: 0x%02X\n", 
                   errIndex, stuffedFrame[errIndex]);
        } else {
            printf("[WARNING] Invalid index chosen! Transmitting without error.\n");
        }
    }

    printf("\n[CHANNEL] Transmitting Hex Stream: ");
    for (int i = 0; i < stuffedLength; i++) printf("%02X ", stuffedFrame[i]);
    printf("\n\n");

    // Receiver Side Processing
    int destuffedLength = pppByteDestuffing(stuffedFrame, stuffedLength, destuffedBuffer);
    unsigned short receiverChecksum = calculateChecksum(destuffedBuffer, destuffedLength);

    printf("[RECEIVER] Verification Checksum Result: 0x%04X\n", receiverChecksum);

    if (receiverChecksum == 0) {
        printf("RESULT: Frame data integrity verified. NO ERRORS.\n");

        int payloadStart = 4;
        int payloadEnd = wasOdd ? (destuffedLength - 3) : (destuffedLength - 2);

        char cleanMessage[500] = "";
        int msgIdx = 0;
        for (int i = payloadStart; i < payloadEnd; i++) {
            cleanMessage[msgIdx++] = (char)destuffedBuffer[i];
        }
        cleanMessage[msgIdx] = '\0';
        printf("Extracted Message payload: \"%s\"\n", cleanMessage);
    } else {
        printf("RESULT: Error detected! Frame checksum invalid. Frame DISCARDED.\n");
    }
}
/* ============================================================
     WRAPPER METHOD TO PARSE DATA LINK LAYER BITS INTO MATRIX
   ============================================================ */

void processDynamic2DParity(char originalText[], char dataLinkFinalBits[], int totalMessageBits) {
    // 7 data bits per row as per the PPT grid structure
    printf("==============================================================\n");
    printf("        4. DYNAMIC 2D PARITY CHECK (DATA LINK DATA BITS)\n");
    printf("==============================================================\n");
    int totalRows = (totalMessageBits + 6) / 7; 
    char dataMatrix[20][7];
    int rowParity[20] = {0};
    int colParity[7] = {0};
    int intersectParity = 0;

    // Print the raw message and its immediate pure binary form first
    printf("Message read from file: \"%s\"\n", originalText);
    printf("Converting to binary  : ");
    for (int i = 0; i < totalMessageBits; i++) {
        printf("%c", dataLinkFinalBits[i]);
        if ((i + 1) % 8 == 0) printf(" "); // separate into 8-bit bytes for clean view
    }
    printf("\n\n");

    // Fill matrix sequentially using the Data Link data bits stream
    int bitIdx = 0;
    for (int i = 0; i < totalRows; i++) {
        for (int j = 0; j < 7; j++) {
            if (bitIdx < totalMessageBits) {
                dataMatrix[i][j] = dataLinkFinalBits[bitIdx++];
            } else {
                dataMatrix[i][j] = '0'; // Padding if bits do not fit grid evenly
            }
        }
    }

    
    // SENDER SIDE LOGIC
    printf("[SENDER SIDE] Generating Matrix from Data Link Bits (Total Rows: %d)...\n", totalRows);
    calculate2DParitySender(dataMatrix, totalRows, rowParity, colParity, &intersectParity);

    printf("Transmitting Stream: ");
    for (int i = 0; i < totalRows; i++) {
        for (int j = 0; j < 7; j++) printf("%c", dataMatrix[i][j]);
        printf("%d ", rowParity[i]);
    }
    for (int j = 0; j < 7; j++) printf("%d", colParity[j]);
    printf("%d\n\n", intersectParity);

    // RECEIVER SIDE LOGIC
    printf("[RECEIVER SIDE] Simulating Data Arrival & Verification...\n");
    verify2DParityReceiver(dataMatrix, totalRows, rowParity, colParity, intersectParity);
}
/* ============================================================
     2D PARITY SENDER ALGORITHM 
   ============================================================ */
void calculate2DParitySender(char data[20][7], int totalRows, int rowParity[], int colParity[], int *intersectParity) {
    // Count row parities (Set to 1 if count of 1s is odd, else 0)
    for (int i = 0; i < totalRows; i++) {
        int oneCount = 0;
        for (int j = 0; j < 7; j++) {
            if (data[i][j] == '1') oneCount++;
        }
        rowParity[i] = (oneCount % 2 == 1) ? 1 : 0;
    }

    // Count column parities
    for (int j = 0; j < 7; j++) {
        int oneCount = 0;
        for (int i = 0; i < totalRows; i++) {
            if (data[i][j] == '1') oneCount++;
        }
        colParity[j] = (oneCount % 2 == 1) ? 1 : 0;
    }

    // Calculate dynamic intersection bit
    int intersectCount = 0;
    for (int i = 0; i < totalRows; i++) {
        if (rowParity[i] == 1) intersectCount++;
    }
    *intersectParity = (intersectCount % 2 == 1) ? 1 : 0;
}

/* ============================================================
     2D PARITY RECEIVER ALGORITHM
   ============================================================ */
void verify2DParityReceiver(char data[20][7], int totalRows, int rowParity[], int colParity[], int intersectParity) {
    int errorDetected = 0;

    // Verify row components
    for (int i = 0; i < totalRows; i++) {
        int count = 0;
        for (int j = 0; j < 7; j++) {
            if (data[i][j] == '1') count++;
        }
        int calcParity = (count % 2 == 1) ? 1 : 0;
        if (calcParity != rowParity[i]) {
            printf(" -> Error flagged in Row %d\n", i + 1);
            errorDetected = 1;
        }
    }

    // Verify column components
    for (int j = 0; j < 7; j++) {
        int count = 0;
        for (int i = 0; i < totalRows; i++) {
            if (data[i][j] == '1') count++;
        }
        int calcParity = (count % 2 == 1) ? 1 : 0;
        if (calcParity != colParity[j]) {
            printf(" -> Error flagged in Column %d\n", j + 1);
            errorDetected = 1;
        }
    }

    if (!errorDetected) {
        printf("Data bits received WITHOUT error.\n");
    } else {
        printf("Data bits received WITH error(s).\n");
    }
}
int main() {
    HashEntry hashTable[TABLE_SIZE];
    char message[1000];
    int messageLength = 0;

    for (int i = 0; i < TABLE_SIZE; i++) hashTable[i].is_Occupied = 0;

    insertEntry(hashTable, "www.google.com", "142.250.10.1", "AA:11:22:33:44:55");
    insertEntry(hashTable, "www.mepcoeng.ac.in", "117.239.182.3", "BB:22:33:44:55:66");
    insertEntry(hashTable, "www.wikipedia.org", "198.35.26.1", "DD:44:55:66:77:88");
    insertEntry(hashTable, "www.github.com", "140.82.121.4", "CC:AA:BB:CC:DD:EE");
    insertEntry(hashTable, "www.yahoo.com", "98.137.11.163", "00:11:A2:B3:C4:D5");
    insertEntry(hashTable, "www.microsoft.com", "20.112.52.29", "12:34:56:78:9A:BC");
    insertEntry(hashTable, "www.amazon.com", "205.251.242.103", "FE:DC:BA:98:76:54");
    insertEntry(hashTable, "www.reddit.com", "151.101.1.140", "A1:B2:C3:D4:E5:F6");
    insertEntry(hashTable, "www.linkedin.com", "108.174.10.10", "44:55:66:77:88:99");
    insertEntry(hashTable, "www.apple.com", "17.253.144.10", "11:AA:22:BB:33:CC");

    int choice = 0;
    char url[100], ip[16], mac[18];

    while (choice != 5) {
        printf("\n======= MENU =======\n");
        printf("1. Display Hash Table\n");
        printf("2. Add New URL Entry\n");
        printf("3. Update Existing Entry\n");
        printf("4. Delete Entry\n");
        printf("5. Network Layers\n");
        printf("6. Exit\n");
        printf("Select an option (1-6): ");
        if (scanf("%d", &choice) != 1) break;

        if (choice == 1) displayEntryTable(hashTable);
        else if (choice == 2) {
            printf("Enter URL: "); scanf("%99s", url);
            printf("Enter IP: "); scanf("%15s", ip);
            printf("Enter MAC: "); scanf("%17s", mac);
            insertEntry(hashTable, url, ip, mac);
        } else if (choice == 3) {
            printf("Enter URL to Update: "); scanf("%99s", url);
            printf("Enter New IP: "); scanf("%15s", ip);
            printf("Enter New MAC: "); scanf("%17s", mac);
            updateEntry(hashTable, url, ip, mac);
        } else if (choice == 4) {
            printf("Enter URL to Delete: "); scanf("%99s", url);
            deleteEntry(hashTable, url);
        } else if (choice == 5) break;
        else if (choice == 6) exit(0);
        else printf("Invalid selection.\n");
    }

    char filename[100];
    printf("Enter the file name: ");
    scanf("%99s", filename);

    messageLength = readFile(filename, message);
    if (messageLength < 0) {
        printf("Error: Could not open %s\n", filename);
        return 1;
    }

    char srcUrl[100], destUrl[100];
    char srcIP[16], srcMAC[18], destIP[16], destMAC[18];

    printf("\n================ INPUT =======================\n");
    printf("Enter Source URL      : "); scanf("%99s", srcUrl);
    printf("Enter Destination URL : "); scanf("%99s", destUrl);

    if (!searchEntry(hashTable, srcUrl, srcIP, srcMAC) || !searchEntry(hashTable, destUrl, destIP, destMAC)) {
        printf("Error: URL records could not be recovered from hash matrix.\n");
        return 1;
    }

    printf("\n================ HASH TABLE ==================\n");
    printf("Source      URL: %-20s -> IP: %-13s MAC: %s\n", srcUrl, srcIP, srcMAC);
    printf("Destination URL: %-20s -> IP: %-13s MAC: %s\n", destUrl, destIP, destMAC);

    // LAYER 1: APPLICATION LAYER
    printf("\n================ LAYER 1 : APPLICATION LAYER ================\n");
    printf("Original Message : %s\n", message);
    printf("Message in Binary    :\n");
    for (int i = 0; i < messageLength; i++) print8BitBinary((unsigned char)message[i]);
    int totalBits = messageLength * 8;
    printf("\nTotal bits generated : %d\n", totalBits);

    // LAYER 2: TRANSPORT LAYER
    printf("\n================ LAYER 2 : TRANSPORT LAYER ==================\n");
    srand(time(NULL));
    unsigned short srcPort = (rand() % (MAX_PORT - MIN_PORT + 1)) + MIN_PORT;
    unsigned short destPort = (rand() % (MAX_PORT - MIN_PORT + 1)) + MIN_PORT;
    printf("Source Port      : %-6d -> Binary: ", srcPort); print16BitBinary(srcPort); printf("\n");
    printf("Destination Port : %-6d -> Binary: ", destPort); print16BitBinary(destPort); printf("\n");

    // LAYER 3: NETWORK LAYER
    printf("\n================ LAYER 3 : NETWORK LAYER =====================\n");
    char srcIPBin[33], destIPBin[33];
    getIPBinary(srcIP, srcIPBin);
    getIPBinary(destIP, destIPBin);

    char networkFinal[10000] = "";
    int totalPackets = (messageLength + 1) / 2;

    for (int p = 0; p < totalPackets; p++) {
        printf("\n-- Packet %d --\n", p + 1);
        printf("  Source IP            : %s (%s)\n", srcIPBin, srcIP);
        printf("  Destination IP       : %s (%s)\n", destIPBin, destIP);
        printf("  Data (16 bits)       : ");

        for(int i=7; i>=0; i--) {
            char bit = ((message[p*2] >> i) & 1) + '0';
            printf("%c", bit);
            strncat(networkFinal, &bit, 1);
        }
        if(p*2+1 < messageLength) {
            for(int i=7; i>=0; i--) {
                char bit = ((message[p*2+1] >> i) & 1) + '0';
                printf("%c", bit);
                strncat(networkFinal, &bit, 1);
            }
        } else {
            printf("00000000");
            strcat(networkFinal, "00000000");
        }
        printf("\n");
    }
    strcat(networkFinal, srcIPBin);
    strcat(networkFinal, destIPBin);
    printf("\n Final Output         : %s\n", networkFinal);
    printf("\nTotal Packets Created : %d\n", totalPackets);

    // LAYER 4: DATA LINK LAYER
    printf("\n================ LAYER 4 : DATA LINK LAYER ====================\n");
    char srcMACBin[49], destMACBin[49];
    getMACBinary(srcMAC, srcMACBin);
    getMACBinary(destMAC, destMACBin);

    char dataLinkFinal[20000] = "";
    int totalFrames = totalPackets * 2;
    // Extract purely the clean Data Link data bits before framing headers/trailers
    char dataBitsOnly[5000] = "";
    for (int p = 0; p < totalPackets; p++) {
        for (int i = 7; i >= 0; i--) {
            char bit = ((message[p * 2] >> i) & 1) + '0';
            strncat(dataBitsOnly, &bit, 1);
        }
        if (p * 2 + 1 < messageLength) {
            for (int i = 7; i >= 0; i--) {
                char bit = ((message[p * 2 + 1] >> i) & 1) + '0';
                strncat(dataBitsOnly, &bit, 1);
            }
        } else {
            strcat(dataBitsOnly, "00000000");
        }
    }

    for (int p = 0; p < totalPackets; p++) {
        // Frame A
        printf("\n-- Frame %d (from Packet %d) --\n", (p*2)+1, p+1);
        printf("  Source IP          : %s (%s)\n", srcIPBin, srcIP);
        printf("  Destination IP     : %s (%s)\n", destIPBin, destIP);
        printf("  Source MAC         : %s (%s)\n", srcMACBin, srcMAC);
        printf("  Destination MAC    : %s (%s)\n", destMACBin, destMAC);
        printf("  Data (8 bits)      : ");
        for(int i=7; i>=0; i--) {
            char bit = ((message[p*2] >> i) & 1) + '0';
            printf("%c", bit);
            strncat(dataLinkFinal, &bit, 1);
        }
        printf("\n  Trailer            : 00000000\n");

        // Frame B
        printf("\n-- Frame %d (from Packet %d) --\n", (p*2)+2, p+1);
        printf("  Source IP          : %s (%s)\n", srcIPBin, srcIP);
        printf("  Destination IP     : %s (%s)\n", destIPBin, destIP);
        printf("  Source MAC         : %s (%s)\n", srcMACBin, srcMAC);
        printf("  Destination MAC    : %s (%s)\n", destMACBin, destMAC);
        printf("  Data (8 bits)      : ");
        if(p*2+1 < messageLength) {
            for(int i=7; i>=0; i--) {
                char bit = ((message[p*2+1] >> i) & 1) + '0';
                printf("%c", bit);
                strncat(dataLinkFinal, &bit, 1);
            }
        } else {
            printf("00000000");
            strcat(dataLinkFinal, "00000000");
        }
        printf("\n  Trailer            : 00000000\n");
    }
    strcat(dataLinkFinal, srcMACBin);
    strcat(dataLinkFinal, destMACBin);
    strcat(dataLinkFinal, srcIPBin);
    strcat(dataLinkFinal, destIPBin);

    printf("\nFinal Output         : %s\n", dataLinkFinal);
    printf("\nTotal Frames Created : %d\n", totalFrames);

    /* ============================================================
                      CALLING FRAMING PROTOCOL METHODS
       ============================================================ */
    printf("\n==============================================================\n");
    printf("                  FRAMING PROTOCOLS DEPLOYMENT\n");
    printf("==============================================================\n");

    // 1. Process Per-Frame Bit Stuffing
    processPerFrameBitStuffing(totalFrames, message, messageLength, srcMACBin, destMACBin, srcIPBin, destIPBin);

    // 2. Process Unified Bit Stream Stuffing
    processUnifiedBitStuffing(dataLinkFinal);

    // 3. Process Character Payload-Only PPP Byte Stuffing/Destuffing
    processPPPByteStuffing(message, messageLength);

    runPPPChecksumSimulation((unsigned char *)message, messageLength);
  // 4. Run the Dynamic 2D Parity Check using extracted Data Link bits
    processDynamic2DParity(message, dataBitsOnly, strlen(dataBitsOnly));
    // FINAL SUMMARY PRINT
    printf("\n================ SUMMARY ======================================\n");
    printf("Total bits in message : %d\n", totalBits);
    printf("Total packets created : %d\n", totalPackets);
    printf("Total frames created  : %d\n", totalFrames);

    return 0;
}