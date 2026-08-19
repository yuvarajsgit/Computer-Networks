#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
int main()
{
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function Declarations
unsigned short calculateChecksum(unsigned char *buf, int count);
int pppByteStuffing(unsigned char *input, int inputLen, unsigned char *output);
int pppByteDestuffing(unsigned char *input, int inputLen, unsigned char *output);
void runPPPChecksumSimulation(unsigned char *message, int messageLen);

/* ============================================================
     SIMPLIFIED CHECKSUM ALGORITHM
   ============================================================ */
unsigned short calculateChecksum(unsigned char *buf, int count) {
    unsigned int sum = 0;

    // 1. Combine pairs of bytes into 16-bit words (multiply high byte by 256)
    for (int i = 0; i < count - 1; i += 2) {
        int word = (buf[i] * 256) + buf[i + 1];
        sum = sum + word;
    }

    // 2. Add remaining byte if length is odd
    if (count % 2 != 0) {
        sum = sum + (buf[count - 1] * 256);
    }

    // 3. Fold overflow bits back into 16-bit range
    while (sum > 65535) {
        int carry = sum / 65536;      // Extract upper bits
        sum = (sum % 65536) + carry;  // Add carry back to lower 16 bits
    }

    // 4. Flip all bits (subtract sum from 65535)
    return (unsigned short)(65535 - sum);
}

/* ============================================================
     SIMPLIFIED PPP BYTE STUFFING
   ============================================================ */
int pppByteStuffing(unsigned char *input, int inputLen, unsigned char *output) {
    int outIdx = 0;
    
    // Add Start Flag (0x7E = 126 in decimal)
    output[outIdx] = 126;
    outIdx++;

    for (int i = 0; i < inputLen; i++) {
        // Look for Flag (126) or Escape Byte (125)
        if (input[i] == 126 || input[i] == 125) {
            output[outIdx] = 125; // Insert Escape Byte (0x7D)
            outIdx++;
            output[outIdx] = input[i] ^ 32; // Modify byte (0x20 = 32)
            outIdx++;
        } else {
            output[outIdx] = input[i];
            outIdx++;
        }
    }

    // Add End Flag (126)
    output[outIdx] = 126;
    outIdx++;

    return outIdx; 
}

/* ============================================================
     SIMPLIFIED PPP BYTE DESTUFFING
   ============================================================ */
int pppByteDestuffing(unsigned char *input, int inputLen, unsigned char *output) {
    int outIdx = 0;

    // Skip first flag (i = 1) and last flag (inputLen - 1)
    for (int i = 1; i < inputLen - 1; i++) {
        if (input[i] == 125) { // Found Escape Byte
            i++; // Skip escape byte and take next byte
            output[outIdx] = input[i] ^ 32;
            outIdx++;
        } else {
            output[outIdx] = input[i];
            outIdx++;
        }
    }

    return outIdx; 
}

/* ============================================================
     SIMPLIFIED PPP SIMULATION RUNNER
   ============================================================ */
void runPPPChecksumSimulation(unsigned char *message, int messageLen) {
    unsigned char senderBuffer[1024];
    unsigned char stuffedFrame[2048];
    unsigned char destuffedBuffer[1024];

    printf("\n==============================================================\n");
    printf("     PPP BYTE STUFFING WITH CHECKSUM SIMULATION               \n");
    printf("==============================================================\n");
    printf("Original Text Data: \"%s\"\n", message);

    // Build Header: Address (255), Control (3), Protocol (0, 33)
    int idx = 0;
    senderBuffer[idx++] = 255; // 0xFF
    senderBuffer[idx++] = 3;   // 0x03
    senderBuffer[idx++] = 0;   // 0x00
    senderBuffer[idx++] = 33;  // 0x21

    // Append Message Payload
    for (int i = 0; i < messageLen; i++) {
        senderBuffer[idx++] = message[i];
    }

    // Pad with 0 if length is odd
    int wasOdd = 0;
    if (idx % 2 != 0) {
        senderBuffer[idx++] = 0;
        wasOdd = 1;
    }

    // Calculate Checksum over Header + Payload
    unsigned short senderChecksum = calculateChecksum(senderBuffer, idx);
    printf("[SENDER] Calculated Checksum (FCS): 0x%04X\n", senderChecksum);

    // Split 16-bit Checksum into two bytes and attach to frame
    senderBuffer[idx++] = senderChecksum / 256; // High Byte
    senderBuffer[idx++] = senderChecksum % 256; // Low Byte

    // Perform Byte Stuffing
    int stuffedLength = pppByteStuffing(senderBuffer, idx, stuffedFrame);

    printf("\nGenerated Stuffed Frame (%d bytes): ", stuffedLength);
    for (int i = 0; i < stuffedLength; i++) {
        printf("%02X ", stuffedFrame[i]);
    }
    printf("\n");

    // Optional Error Injection
    char choice;
    printf("\nDo you want to inject a transmission error? (y/n): ");
    scanf(" %c", &choice);

    if (choice == 'y' || choice == 'Y') {
        int errIndex;
        printf("Enter byte index to corrupt (0 to %d): ", stuffedLength - 1);
        scanf("%d", &errIndex);

        if (errIndex >= 0 && errIndex < stuffedLength) {
            stuffedFrame[errIndex] = stuffedFrame[errIndex] + 1; // Corrupt byte by changing its value
            printf("[CHANNEL SIMULATION] Corrupted byte at index %d! New byte value: 0x%02X\n", 
                   errIndex, stuffedFrame[errIndex]);
        } else {
            printf("[WARNING] Invalid index! Transmitting without error.\n");
        }
    }

    printf("\n[CHANNEL] Transmitting Hex Stream: ");
    for (int i = 0; i < stuffedLength; i++) {
        printf("%02X ", stuffedFrame[i]);
    }
    printf("\n\n");

    // Receiver Side Processing
    int destuffedLength = pppByteDestuffing(stuffedFrame, stuffedLength, destuffedBuffer);
    unsigned short receiverChecksum = calculateChecksum(destuffedBuffer, destuffedLength);

    printf("[RECEIVER] Verification Checksum Result: 0x%04X\n", receiverChecksum);

    if (receiverChecksum == 0) {
        printf("RESULT: Frame data integrity verified. NO ERRORS.\n");

        int payloadStart = 4; // Skip 4-byte header
        int payloadEnd = destuffedLength - 2; // Exclude checksum bytes
        if (wasOdd) {
            payloadEnd = payloadEnd - 1; // Exclude odd padding
        }

        char cleanMessage[500];
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
     MAIN FUNCTION
   ============================================================ */
int main() {
    unsigned char testMessage[] = "Hello";
    int messageLen = strlen((char *)testMessage);

    runPPPChecksumSimulation(testMessage, messageLen);

    return 0;
}

Start
  │
  ▼
sum = 0
  │
  ▼
Take 2 bytes
  │
  ▼
Combine into 16-bit word
  │
  ▼
Add to sum
  │
  ▼
Repeat until all pairs processed
  │
  ▼
Odd byte left?
  │
 ├── Yes → Treat as High Byte (XX00) and add
 └── No
  │
  ▼
sum > 65535?
  │
 ├── Yes → Fold carry
 └── No
  │
  ▼
Take One's Complement
  │
  ▼
Return Checksum