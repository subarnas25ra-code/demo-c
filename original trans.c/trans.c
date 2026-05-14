/*
=========================================================
 SECURE BANK ACCOUNT MANAGEMENT SYSTEM IN C
=========================================================

SECURITY FEATURES INCLUDED:
1. Password Authentication
2. Input Validation
3. Buffer Overflow Protection
4. Encrypted Customer Names
5. File Read/Write Validation
6. Audit Log System
7. Negative Balance Prevention
8. Account Range Validation
9. Secure File Handling
10. Transaction Security

=========================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_ACCOUNTS 100
#define PASSWORD "admin123"

// Structure Definition
struct clientData
{
    unsigned int acctNum;
    char lastName[15];
    char firstName[10];
    double balance;
};

// Function Prototypes
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void encryptDecrypt(char *data);
void logAction(const char *action);
int loginSystem(void);

// MAIN FUNCTION
int main()
{
    FILE *cfPtr;
    unsigned int choice;

    // LOGIN SECURITY
    if (!loginSystem())
    {
        printf("\nToo many failed attempts!\n");
        return 1;
    }

    // Open Binary File
    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL)
    {
        // Create file if not exists
        cfPtr = fopen("credit.dat", "wb+");

        if (cfPtr == NULL)
        {
            printf("File could not be opened.\n");
            return 1;
        }

        // Initialize Blank Records
        struct clientData blankClient = {0, "", "", 0.0};

        for (int i = 0; i < MAX_ACCOUNTS; i++)
        {
            fwrite(&blankClient, sizeof(struct clientData), 1, cfPtr);
        }
    }

    // MENU LOOP
    while ((choice = enterChoice()) != 5)
    {
        switch (choice)
        {
        case 1:
            textFile(cfPtr);
            break;

        case 2:
            updateRecord(cfPtr);
            break;

        case 3:
            newRecord(cfPtr);
            break;

        case 4:
            deleteRecord(cfPtr);
            break;

        default:
            printf("Invalid Choice!\n");
            break;
        }
    }

    fclose(cfPtr);

    printf("\nProgram Terminated Successfully.\n");

    return 0;
}

// LOGIN SYSTEM
int loginSystem(void)
{
    char password[20];
    int attempts = 3;

    while (attempts > 0)
    {
        printf("Enter Admin Password: ");

        scanf("%19s", password);

        if (strcmp(password, PASSWORD) == 0)
        {
            printf("\nLogin Successful.\n");
            logAction("Admin Login Successful");
            return 1;
        }
        else
        {
            attempts--;

            printf("Wrong Password! Attempts Left: %d\n", attempts);

            logAction("Failed Login Attempt");
        }
    }

    return 0;
}

// MENU FUNCTION
unsigned int enterChoice(void)
{
    unsigned int choice;

    printf("\n====================================");
    printf("\n     BANK MANAGEMENT SYSTEM");
    printf("\n====================================");

    printf("\n1 - Create Text File");
    printf("\n2 - Update Account");
    printf("\n3 - Add New Account");
    printf("\n4 - Delete Account");
    printf("\n5 - Exit");

    printf("\n\nEnter Your Choice: ");

    if (scanf("%u", &choice) != 1)
    {
        printf("Invalid Input.\n");
        exit(1);
    }

    return choice;
}

// CREATE TEXT FILE
void textFile(FILE *readPtr)
{
    FILE *writePtr;
    struct clientData client = {0, "", "", 0.0};

    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        printf("Unable to create text file.\n");
        return;
    }

    rewind(readPtr);

    fprintf(writePtr,
            "%-6s%-16s%-11s%10s\n",
            "Acct",
            "Last Name",
            "First Name",
            "Balance");

    while (fread(&client,
                 sizeof(struct clientData),
                 1,
                 readPtr) == 1)
    {
        if (client.acctNum != 0)
        {
            // Decrypt
            encryptDecrypt(client.lastName);
            encryptDecrypt(client.firstName);

            fprintf(writePtr,
                    "%-6d%-16s%-11s%10.2f\n",
                    client.acctNum,
                    client.lastName,
                    client.firstName,
                    client.balance);

            // Encrypt again
            encryptDecrypt(client.lastName);
            encryptDecrypt(client.firstName);
        }
    }

    fclose(writePtr);

    printf("accounts.txt created successfully.\n");

    logAction("Generated accounts.txt");
}

// ADD NEW RECORD
void newRecord(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};

    unsigned int accountNum;

    printf("Enter New Account Number (1-100): ");

    if (scanf("%u", &accountNum) != 1)
    {
        printf("Invalid Input.\n");
        return;
    }

    // SECURITY VALIDATION
    if (accountNum < 1 || accountNum > MAX_ACCOUNTS)
    {
        printf("Invalid Account Number.\n");
        return;
    }

    // Locate Position
    fseek(fPtr,
          (accountNum - 1) * sizeof(struct clientData),
          SEEK_SET);

    fread(&client,
          sizeof(struct clientData),
          1,
          fPtr);

    // Check Existing Account
    if (client.acctNum != 0)
    {
        printf("Account Already Exists.\n");
        return;
    }

    printf("Enter Last Name: ");
    scanf("%14s", client.lastName);

    printf("Enter First Name: ");
    scanf("%9s", client.firstName);

    printf("Enter Balance: ");

    if (scanf("%lf", &client.balance) != 1)
    {
        printf("Invalid Balance Input.\n");
        return;
    }

    // Prevent Negative Balance
    if (client.balance < 0)
    {
        printf("Balance Cannot Be Negative.\n");
        return;
    }

    client.acctNum = accountNum;

    // ENCRYPT DATA
    encryptDecrypt(client.lastName);
    encryptDecrypt(client.firstName);

    // Write Record
    fseek(fPtr,
          (accountNum - 1) * sizeof(struct clientData),
          SEEK_SET);

    if (fwrite(&client,
               sizeof(struct clientData),
               1,
               fPtr) != 1)
    {
        printf("File Write Error.\n");
    }
    else
    {
        printf("Account Created Successfully.\n");

        logAction("New Account Created");
    }
}

// UPDATE RECORD
void updateRecord(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};

    unsigned int account;
    double transaction;

    printf("Enter Account Number to Update: ");

    if (scanf("%u", &account) != 1)
    {
        printf("Invalid Input.\n");
        return;
    }

    // Validation
    if (account < 1 || account > MAX_ACCOUNTS)
    {
        printf("Invalid Account Number.\n");
        return;
    }

    fseek(fPtr,
          (account - 1) * sizeof(struct clientData),
          SEEK_SET);

    if (fread(&client,
              sizeof(struct clientData),
              1,
              fPtr) != 1)
    {
        printf("File Read Error.\n");
        return;
    }

    if (client.acctNum == 0)
    {
        printf("Account Does Not Exist.\n");
        return;
    }

    // Decrypt Data
    encryptDecrypt(client.lastName);
    encryptDecrypt(client.firstName);

    printf("\nAccount Details:\n");

    printf("%d %s %s %.2f\n",
           client.acctNum,
           client.lastName,
           client.firstName,
           client.balance);

    printf("\nEnter Transaction Amount (+Deposit / -Withdraw): ");

    if (scanf("%lf", &transaction) != 1)
    {
        printf("Invalid Transaction Input.\n");
        return;
    }

    // SECURITY CHECK
    if ((client.balance + transaction) < 0)
    {
        printf("Insufficient Balance.\n");
        return;
    }

    client.balance += transaction;

    printf("Updated Balance = %.2f\n",
           client.balance);

    // Encrypt Again
    encryptDecrypt(client.lastName);
    encryptDecrypt(client.firstName);

    fseek(fPtr,
          -sizeof(struct clientData),
          SEEK_CUR);

    if (fwrite(&client,
               sizeof(struct clientData),
               1,
               fPtr) != 1)
    {
        printf("File Update Error.\n");
    }
    else
    {
        printf("Record Updated Successfully.\n");

        logAction("Account Updated");
    }
}

// DELETE RECORD
void deleteRecord(FILE *fPtr)
{
    struct clientData client;
    struct clientData blankClient = {0, "", "", 0.0};

    unsigned int accountNum;

    printf("Enter Account Number to Delete: ");

    if (scanf("%u", &accountNum) != 1)
    {
        printf("Invalid Input.\n");
        return;
    }

    // Validation
    if (accountNum < 1 || accountNum > MAX_ACCOUNTS)
    {
        printf("Invalid Account Number.\n");
        return;
    }

    fseek(fPtr,
          (accountNum - 1) * sizeof(struct clientData),
          SEEK_SET);

    fread(&client,
          sizeof(struct clientData),
          1,
          fPtr);

    if (client.acctNum == 0)
    {
        printf("Account Does Not Exist.\n");
        return;
    }

    fseek(fPtr,
          (accountNum - 1) * sizeof(struct clientData),
          SEEK_SET);

    if (fwrite(&blankClient,
               sizeof(struct clientData),
               1,
               fPtr) != 1)
    {
        printf("Delete Failed.\n");
    }
    else
    {
        printf("Account Deleted Successfully.\n");

        logAction("Account Deleted");
    }
}

// SIMPLE XOR ENCRYPTION
void encryptDecrypt(char *data)
{
    while (*data)
    {
        *data = *data ^ 5;
        data++;
    }
}

// AUDIT LOG FUNCTION
void logAction(const char *action)
{
    FILE *logPtr;

    time_t now;
    time(&now);

    logPtr = fopen("log.txt", "a");

    if (logPtr != NULL)
    {
        fprintf(logPtr,
                "%s : %s",
                action,
                ctime(&now));

        fclose(logPtr);
    }
}
