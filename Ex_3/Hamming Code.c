#include <stdio.h>
#include <math.h>
#include <string.h>

// Prototype Declaration
void processHammingCode(char originalText[], char dataBitsOnly[]);
void calculateHammingSender(int data[], int dataLen, int encoded[], int *totalBits);
void verifyHammingReceiver(int received[], int totalBits);

/* ============================================================
     HAMMING CODE SENDER ALGORITHM
   ============================================================ */
void calculateHammingSender(int data[], int dataLen, int encoded[], int *totalBits) {
    int r = 0;
    // Calculate required redundant/parity bits (2^r >= m + r + 1)
    while ((1 << r) < (dataLen + r + 1)) {
        r++;
    }
    *totalBits = dataLen + r;

    // Place data bits and leave positions that are powers of 2 (1, 2, 4, 8...) for parity
    int j = 0;
    for (int i = 1; i <= *totalBits; i++) {
        if ((i & (i - 1)) == 0) {
            encoded[i] = 0; // Temporary 0 for parity bit positions
        } else {
            encoded[i] = data[j++];
        }
    }

    // Calculate Even Parity for each parity bit
    for (int i = 0; i < r; i++) {
        int parityPos = (1 << i); // 1, 2, 4, 8...
        int parityVal = 0;

        for (int k = 1; k <= *totalBits; k++) {
            if (k & parityPos) {
                parityVal ^= encoded[k];
            }
        }
        encoded[parityPos] = parityVal;
    }
}

/* ============================================================
     HAMMING CODE RECEIVER ALGORITHM (Error Detection & Correction)
   ============================================================ */
void verifyHammingReceiver(int received[], int totalBits) {
    int errorPos = 0;
    int r = 0;

    while ((1 << r) <= totalBits) {
        r++;
    }

    // Recalculate parity bits to check for syndrome/error position
    for (int i = 0; i < r; i++) {
        int parityPos = (1 << i);
        int parityVal = 0;

        for (int k = 1; k <= totalBits; k++) {
            if (k & parityPos) {
                parityVal ^= received[k];
            }
        }

        if (parityVal != 0) {
            errorPos += parityPos;
        }
    }

    if (errorPos == 0) {
        printf(">>>> HAMMING VERDICT: Data received WITHOUT ANY ERROR.\n");
    } else {
        printf(">>>> HAMMING VERDICT: ERROR DETECTED at bit position %d!\n", errorPos);
        printf("Correcting bit %d from %d to %d...\n", errorPos, received[errorPos], received[errorPos] ^ 1);
        
        // Correct the flipped bit
        received[errorPos] ^= 1;

        printf("Corrected Stream : ");
        for (int i = 1; i <= totalBits; i++) {
            printf("%d", received[i]);
        }
        printf("\n");
    }
}

/* ============================================================
     INTERACTIVE HAMMING CODE (ERROR CORRECTION)
   ============================================================ */
void processHammingCode(char originalText[], char dataBitsOnly[]) {
    printf("\n==============================================================\n");
    printf("            6. HAMMING CODE (ERROR CORRECTION)                \n");
    printf("==============================================================\n");
    printf("Original Payload Text : \"%s\"\n", originalText);

    // Convert input character bit string to integer array
    int dataLen = strlen(dataBitsOnly);
    int data[1024];
    for (int i = 0; i < dataLen; i++) {
        data[i] = dataBitsOnly[i] - '0';
    }

    int encoded[2048] = {0};
    int totalBits = 0;

    // SENDER SIDE
    printf("\n[SENDER SIDE] Generating Hamming Code for %d Data Bits...\n", dataLen);
    calculateHammingSender(data, dataLen, encoded, &totalBits);

    printf("Encoded Transmitted Stream : ");
    for (int i = 1; i <= totalBits; i++) {
        printf("%d", encoded[i]);
    }
    printf(" (Total Bits: %d)\n\n", totalBits);

    // INTERACTIVE USER PROMPT
    char choice;
    printf("Do you want to inject an error? (y/n): ");
    scanf(" %c", &choice);

    int rxBuffer[2048];
    memcpy(rxBuffer, encoded, sizeof(encoded));

    if (choice == 'y' || choice == 'Y') {
        int errPos;
        printf("Enter bit position to inject error (1 to %d): ", totalBits);
        scanf("%d", &errPos);

        if (errPos >= 1 && errPos <= totalBits) {
            // Flip the specified bit
            rxBuffer[errPos] ^= 1;
            
            printf("\n[INJECTING ERROR] Bit at position %d flipped!\n", errPos);
            printf("Corrupted Received Stream  : ");
            for (int i = 1; i <= totalBits; i++) {
                printf("%d", rxBuffer[i]);
            }
            printf("\n\n");
        } else {
            printf("\n[WARNING] Invalid bit position! Transmitting without error injection.\n\n");
        }
    } else {
        printf("\n[NO ERROR INJECTED] Transmitting clean stream...\n\n");
    }

    // RECEIVER SIDE
    printf("[RECEIVER SIDE] Verifying Received Stream...\n");
    verifyHammingReceiver(rxBuffer, totalBits);
}
int main()
{
// 5. Run Hamming Code Error Correction
    processHammingCode(message, dataBitsOnly);
	
	return 0;
}