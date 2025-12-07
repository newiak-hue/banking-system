#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <direct.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
//windows and other os have different "make directory" function name
#ifdef _WIN32
    #include <direct.h>
    #define MKDIR(path) _mkdir(path)
#else
    #include <sys/stat.h>
    #include <sys/types.h>
    #define MKDIR(path) mkdir(path, 0755)
#endif

void logAction(char *action) {
    //function to log action
    FILE *fp = fopen("database/transaction.log", "a");
    if (fp == NULL) {
        perror("Error writing to transaction log");
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d] %s\n",
            t->tm_year + 1900,
            t->tm_mon + 1,
            t->tm_mday,
            t->tm_hour,
            t->tm_min,
            t->tm_sec,
            action);

    fclose(fp);
}

bool validateIC(char *ic) {
    //check if inputted ic fits format (12 digits)
    if (strlen(ic) != 12) {
        printf("IC must be exactly 12 digits.\n");
        return false;
    }

    for (int i = 0; i < 12; i++) {
        if (!isdigit(ic[i])) {
            printf("IC must contain digits only.\n");
            return 0;
        }
    }

    return true;
}

bool validateAccount(char *account) {
    //check if user selected an actual account type
    char *accountTypes[] = {"savings","current" ,"saving"};
    if (strcmp(account, "1") == 0) {
        strcpy(account, "savings");
    }
    else if (strcmp(account, "2") == 0) {
        strcpy(account, "current");
    }
    int length = sizeof(accountTypes) / sizeof(accountTypes[0]);
    for (int i = 0; i < length; i++) {
        if (strcmp(account, accountTypes[i]) == 0) {
            return true;
        }
    }
    printf("Please choose one of the two options.\n");
    return false;
}

bool validatePin(char *pin) {
    //check if pin is in the correct format (4 digits)
    if (strlen(pin) != 4) {
        printf("Pin must be exactly 4 digits.\n");
        return false;
    }
    for (int i = 0; i < 4; i++) {
        if (!isdigit(pin[i])) {
            printf("Pin must contain digits only.\n");
            return false;
        }
    }
    return true;
}

bool checkExists(const char *accNum) {
    //helper function to check if accounts exists
    FILE *index = fopen("database/index.txt", "r");
    if (index == NULL) {
        return false;
    }

    char line[50];

    while (fgets(line, sizeof(line), index)) {
        line[strcspn(line, "\n")] = 0;

        if (strcmp(line, accNum) == 0) {
            fclose(index);
            return true;
        }
    }

    fclose(index);
    return false;
}

void generateAccount(char *output) {
    //function to generate account number in the correct format (random 7-9 digits)
    while (true) {
        int length = (rand() % 3) + 7;

        for (int i = 0; i < length; i++) {
            output[i] = '0' + (rand() % 10);
        }
        output[length] = '\0';

        if (output[0] == '0')
            output[0] = '1' + (rand() % 9);

        if (!checkExists(output)) {
            return;
        }

    }
}

bool checkCancel(char *input, char *actionName, char *logMessage) {
    //helper function to check if user typed "exit" or "cancel"
    char temp[50];

    for (int i = 0; i < strlen(input); i++) {
        temp[i] = (char)tolower(input[i]);
    }
    temp[strlen(input)] = '\0';

    if (strcmp(temp, "cancel") == 0 || strcmp(temp, "exit") == 0) {
        printf("%s.\n", actionName);
        logAction(logMessage);
        return true;
    }
    return false;
}

int loadAcc(char account[][50]) {
    //function to load account
    FILE *file = fopen("database/index.txt", "r");
    if (!file) {
        printf("No accounts found.\n");
    }

    int count = 0;

    while (fgets(account[count], sizeof(account[count]), file)) {
        account[count][strcspn(account[count], "\n")] = '\0';
        count++;
    }

    fclose(file);

    if (count == 0) {
        printf("No accounts available.\n");
    }
    return count;
}

bool getAccInfo(char *accFile, char *name, char *ic, char *accType, char *pin,float *balance) {
    //function to get all the account info (name,ic,account type, pin, balance)
    FILE *acc = fopen(accFile, "r");
    if (acc == NULL) {
        printf("Error opening account file.\n");
        return false;
    }

    char line[50];

    while (fgets(line, sizeof(line), acc) != NULL) {
        if (strncmp(line, "Name:", 5) == 0) {
            sscanf(line, "Name: %49[^\n]", name);
        } else if (strncmp(line, "IC:", 3) == 0) {
            sscanf(line, "IC: %12s", ic);
        } else if (strncmp(line, "Account Type:", 13) == 0) {
            sscanf(line, "Account Type: %49s", accType);
        } else if (strncmp(line, "PIN:", 4) == 0) {
            sscanf(line, "PIN: %4s", pin);
        } else if (strncmp(line, "Balance:", 8) == 0) {
            sscanf(line, "Balance: %f", balance);
        }
    }
    fclose(acc);
    return true;
}

bool writeAccInfo(char *accFile, char *name, char *ic, char *accType, char *pin,float *balance) {
    //function to write account info after performing action
    FILE *acc = fopen(accFile, "w");
    if (!acc) {
        printf("Error opening account file.\n");
        return false;
    }

    fprintf(acc, "Name: %s\n", name);
    fprintf(acc, "IC: %s\n", ic);
    fprintf(acc, "Account Type: %s\n", accType);
    fprintf(acc, "PIN: %s\n", pin);
    fprintf(acc, "Balance: %.2f\n", *balance);

    fclose(acc);
    return true;
}

void create() {
    char name[50];
    char ic[50];
    char accountType[50];
    char pin[50];
    printf("Type 'cancel' or 'exit' to cancel account creation at any point.\n");
    printf("Enter your name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;

    if (checkCancel(name,"Account creation cancelled", "Cancelled account creation")) return;

    while (true) {
        printf("Enter your Identification Card(IC) number (12 digits): ");
        fgets(ic, sizeof(ic), stdin);
        ic[strcspn(ic, "\n")] = 0;
        if (checkCancel(ic,"Account creation cancelled", "Cancelled account creation")) return;
        if (validateIC(ic)) {
            break;
        }
    }

    while (true) {
        printf("Enter your account type (1:Savings / 2:Current): ");
        fgets(accountType, sizeof(accountType), stdin);
        accountType[strcspn(accountType, "\n")] = 0;
        for (int i = 0; accountType[i] != '\0'; i++) {
            accountType[i] = (char)tolower(accountType[i]);
        }
        if (checkCancel(accountType,"Account creation cancelled", "Cancelled account creation")) return;
        if (validateAccount(accountType)) {
            break;
        }
    }

    while (true) {
        printf("Enter your 4-digit PIN: ");
        fgets(pin, sizeof(pin), stdin);
        pin[strcspn(pin, "\n")] = 0;
        if (checkCancel(pin,"Account creation cancelled", "Cancelled account creation")) return;
        if (validatePin(pin)) {
            break;
        }
    }

    char accNum[50];
    char fileName[50];
    generateAccount(accNum);

    sprintf(fileName, "database/index.txt");
    FILE *fp1 = fopen("database/index.txt", "a");
    if (fp1 == NULL) {
        perror("Error opening index file: ");
        return;
    }
    fprintf(fp1, "%s\n", accNum);
    fclose(fp1);

    snprintf(fileName, sizeof(fileName), "database/%s.txt", accNum);
    FILE *file = fopen(fileName, "w");
    if (file == NULL) {
        perror("Error opening account file: ");
        return;
    }

    fprintf(file, "Name: %s\n", name);
    fprintf(file, "IC: %s\n", ic);
    fprintf(file, "Account Type: %s\n", accountType);
    fprintf(file, "PIN: %s\n", pin);
    fprintf(file, "Balance: %.2f\n", 0.0);
    fclose(file);
    logAction("Account created successfully");
    printf("=== CREATION SUCCESSFUL ===\n");
}

void start() {
    //function to load account session info and create directory if not exists
    if (MKDIR("database") == 0) {
        printf("New directory created successfully.\n");
    } else {
        if (errno == EEXIST) {
            printf("Directory found.\n");
        } else {
            perror("Error creating directory");
            return;
        }
    }

    FILE *file = fopen("database/index.txt", "r");
    if (file == NULL) {
        file = fopen("database/index.txt", "w");
        if (file == NULL) {
            perror("Error creating index file: ");
            return;
        }
        fclose(file);

        printf("No accounts found.\n");
        return;
    }

    int count = 0;
    char line[50];
    while (fgets(line, sizeof(line), file) != NULL) {
        count++;
    }
    fclose(file);
    if (count == 0) {
        printf("No accounts found.\n");
    } else {
        printf("Account(s) loaded: %d.\n", count);
    }
    logAction("Program started");

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    printf("Current date & time: [%04d-%02d-%02d %02d:%02d:%02d] \n",
            t->tm_year + 1900,
            t->tm_mon + 1,
            t->tm_mday,
            t->tm_hour,
            t->tm_min,
            t->tm_sec);
}

void delete() {
    char account[100][50];
    int count = loadAcc(account);

    printf("Type 'cancel' or 'exit' to cancel account deletion at any point.\n");
    printf("Choose one of the following account(s) to delete:\n");

    for (int i = 0; i < count; i++) {
        printf("%d: %s\n", i+1 ,account[i]);
    }

    int choice = 0;
    char buf[20];

    while (true) {
        printf("Choose account to delete: ");
        if (!fgets(buf, sizeof(buf), stdin)) {
            printf("Input error.\n");
            return;
        }
        buf[strcspn(buf, "\n")] = '\0';
        if (checkCancel(buf,"Account deletion cancelled","Cancelled account deletion")) return;
        choice = atoi(buf);
        if (choice >= 1 && choice <= count) {
            break;
        }
        int found = 0;
        for (int i = 0; i < count; i++) {
            if (strcmp(account[i], buf) == 0) {
                choice = i + 1;
                found = 1;
                break;
            }
        }
        if (found == 1) break;
        printf("Invalid choice. Try again.\n");
    }

    char *accDelete = account[choice - 1];
    char accFile[150];
    snprintf(accFile, sizeof(accFile), "database/%s.txt", accDelete);

    char name[50], ic[50], accType[50], pin[10];
    float balance;
    if (!getAccInfo(accFile,name,ic,accType, pin, &balance)) {
        printf("Error: Unable to load account info. Deletion cancelled.\n");
        return;
    }


    const char *userID = ic + strlen(ic) - 4;

    char inputAcc[50], inputIC[10], inputPIN[10];

    printf("--- Account Confirmation Required ---\n");

    printf("Reenter account number: ");
    fgets(inputAcc, sizeof(inputAcc), stdin);
    inputAcc[strcspn(inputAcc, "\n")] = '\0';
    if (checkCancel(inputAcc,"Account deletion cancelled","Cancelled account deletion")) return;

    printf("Enter last 4 characters of IC number: ");
    fgets(inputIC, sizeof(inputIC), stdin);
    inputIC[strcspn(inputIC, "\n")] = '\0';
    if (checkCancel(inputIC,"Account deletion cancelled","Cancelled account deletion")) return;

    printf("Enter 4-digit PIN: ");
    fgets(inputPIN, sizeof(inputPIN), stdin);
    inputPIN[strcspn(inputPIN, "\n")] = '\0';
    if (checkCancel(inputPIN,"Account deletion cancelled","Cancelled account deletion")) return;

    if (strcmp(inputAcc, accDelete) == 0 && strcmp(inputIC, userID) == 0 && strcmp(inputPIN, pin) == 0){
        printf("Verification successful. Account will be deleted.\n");
    } else {
        printf("Verification failed. Account will NOT be deleted, returning to menu...");
        return;
    }

    if (remove(accFile) != 0) {
        printf("Warning: could not delete data file '%s'\n", accFile);
    }

    FILE *out = fopen("database/index.txt", "w");
    if (!out) {
        printf("Error writing index file\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        if (i != choice - 1) {
            fprintf(out, "%s\n", account[i]);
        }
    }

    fclose(out);

    printf("=== DELETION SUCCESSFUL ===\n");
    printf("Account '%s' deleted successfully.\n", accDelete);
    logAction("Account deleted successfully");
}

void deposit() {
    char account[100][50];
    int count = loadAcc(account);

    printf("Type 'cancel' or 'exit' to cancel depositing amount at any point.\n");
    printf("Choose one of the following account(s) to deposit: \n");

    for (int i = 0; i < count; i++) {
        printf("%d: %s\n", i+1 ,account[i]);
    }

    int choice = 0;
    char buf[20];

    while (true) {
        printf("Choose account to deposit: ");
        if (!fgets(buf, sizeof(buf), stdin)) {
            printf("Input error.\n");
            return;
        }
        buf[strcspn(buf, "\n")] = '\0';
        if (checkCancel(buf,"Depositing amount cancelled","Cancelled depositing amount")) return;
        choice = atoi(buf);
        if (choice >= 1 && choice <= count) {
            break;
        }
        int found = 0;
        for (int i = 0; i < count; i++) {
            if (strcmp(account[i], buf) == 0) {
                choice = i + 1;
                found = 1;
                break;
            }
        }
        if (found == 1) break;
        printf("Invalid choice. Try again.\n");
    }

    char *accDeposit = account[choice - 1];
    char accFile[150];
    snprintf(accFile, sizeof(accFile), "database/%s.txt", accDeposit);

    char name[50], ic[50], accType[50], pin[10];
    float balance;
    if (!getAccInfo(accFile,name,ic,accType, pin, &balance)) {
        printf("Error: Unable to load account info. Depositing amount cancelled.\n");
        return;
    }

    char inputPIN[10], inputBalance[50];
    float amount;

    while (true) {
        printf("Enter amount to deposit (must be > RM0 and <= RM50,000): ");
        fgets(inputBalance, sizeof(inputBalance), stdin);
        inputBalance[strcspn(inputBalance, "\n")] = '\0';

        if (checkCancel(inputBalance, "Depositing amount", "Cancelled depositing amount")) return;

        amount = atof(inputBalance);

        if (amount < 1 || amount > 50000) {
            printf("Invalid amount.\n");
        } else {
            break;
        }
    }

    printf("Enter 4-digit PIN to verify: ");
    fgets(inputPIN, sizeof(inputPIN), stdin);
    inputPIN[strcspn(inputPIN, "\n")] = '\0';
    if (checkCancel(inputPIN,"Depositing amount","Cancelled depositing amount")) return;

    if (strcmp(inputPIN, pin) == 0) {
        printf("Verification successful.\n");
    } else {
        printf("Verification failed. PIN is wrong, returning to menu...\n");
        return;
    }

    balance += amount;

    if (!writeAccInfo(accFile,name,ic,accType, pin, &balance)) {
        printf("Error: Unable to save account info. Depositing amount cancelled.\n");
        return;
    }

    printf("=== DEPOSIT SUCCESSFUL ===\n");
    printf("Successfully deposited RM %.2f into account %s.\n", amount, accDeposit);
    printf("New balance: RM %.2f\n", balance);
    logAction("Deposited amount successfully");
}

void withdraw() {
    char account[100][50];
    int count = loadAcc(account);

    printf("Type 'cancel' or 'exit' to cancel withdrawal at any point.\n");
    printf("Choose one of the following account(s) to withdraw from:\n");

    for (int i = 0; i < count; i++) {
        printf("%d: %s\n", i + 1, account[i]);
    }

    int choice = 0;
    char buf[20];

    while (true) {
        printf("Choose account to withdraw from: ");
        if (!fgets(buf, sizeof(buf), stdin)) {
            printf("Input error.\n");
            return;
        }

        buf[strcspn(buf, "\n")] = '\0';
        if (checkCancel(buf,"Withdrawal cancelled","Cancelled withdrawal")) return;

        choice = atoi(buf);

        if (choice >= 1 && choice <= count) break;

        int found = 0;
        for (int i = 0; i < count; i++) {
            if (strcmp(account[i], buf) == 0) {
                choice = i + 1;
                found = 1;
                break;
            }
        }
        if (found) break;

        printf("Invalid choice. Try again.\n");
    }

    char *accWithdraw = account[choice - 1];
    char accFile[150];
    snprintf(accFile, sizeof(accFile), "database/%s.txt", accWithdraw);

    char name[50], ic[50], accType[50], pin[10];
    float balance = 0;
    if (!getAccInfo(accFile,name,ic,accType, pin, &balance)) {
        printf("Error: Unable to load account info. Withdrawing amount cancelled.\n");
        return;
    }

    printf("Current balance for %s: RM %.2f\n", accWithdraw, balance);

    char inputAmount[50], inputPIN[10];
    float amount;

    while (true) {
        printf("Enter amount to withdraw (must be > RM0): ");
        fgets(inputAmount, sizeof(inputAmount), stdin);
        inputAmount[strcspn(inputAmount, "\n")] = '\0';

        if (checkCancel(inputAmount, "Withdrawal cancelled", "Cancelled withdrawal")) return;

        amount = atof(inputAmount);

        if (amount <= 0) {
            printf("Amount must be > RM0.\n");
            continue;
        }

        if (amount > balance) {
            printf("Insufficient balance. You only have RM %.2f.\n", balance);
        } else {
            break;
        }
    }

    printf("Enter 4-digit PIN to verify: ");
    fgets(inputPIN, sizeof(inputPIN), stdin);
    inputPIN[strcspn(inputPIN, "\n")] = '\0';
    if (checkCancel(inputPIN,"Withdrawal cancelled","Cancelled withdrawal")) return;

    if (strcmp(inputPIN, pin) == 0) {
        printf("Verification successful.\n");
    } else {
        printf("Verification failed. PIN is wrong, returning to menu...\n");
        return;
    }

    balance -= amount;

    if (!writeAccInfo(accFile,name,ic,accType, pin, &balance)) {
        printf("Error: Unable to save account info. Withdrawing amount cancelled.\n");
        return;
    }

    printf("=== WITHDRAWAL SUCCESSFUL ===\n");
    printf("Successfully withdrew RM %.2f from account %s.\n", amount, accWithdraw);
    printf("New balance: RM %.2f\n", balance);
    logAction("Withdrew amount successfully");
}

void remittance() {
    char account[100][50];
    int count = loadAcc(account);

    if (count < 2) {
        printf("Not enough accounts to perform remittance.\n");
        return;
    }

    printf("Type 'cancel' or 'exit' to cancel remittance at any point.\n");

    printf("Choose the SENDER account: \n");
    for (int i = 0; i < count; i++) {
        printf("%d: %s\n", i + 1, account[i]);
    }

    int senderIndex = 0;
    char buf[50];

    while (true) {
        printf("Sender account: ");
        fgets(buf, sizeof(buf), stdin);
        buf[strcspn(buf, "\n")] = '\0';
        if (checkCancel(buf, "Remittance cancelled.", "Cancelled remittance")) return;

        int choice = atoi(buf);
        if (choice >= 1 && choice <= count) {
            senderIndex = choice - 1;
            break;
        }
        int found = 0;
        for (int i = 0; i < count; i++) {
            if (strcmp(account[i], buf) == 0) {
                senderIndex = i;
                found = 1;
                break;
            }
        }
        if (found == 1) break;
        printf("Invalid choice. Try again.\n");
    }

    char senderAcc[50];
    strcpy(senderAcc, account[senderIndex]);

    char senderFile[150];
    snprintf(senderFile, sizeof(senderFile), "database/%s.txt", senderAcc);

    char senderName[50], senderIC[50], senderType[50], senderPIN[10];
    float senderBalance;
    if (!getAccInfo(senderFile, senderName, senderIC, senderType, senderPIN, &senderBalance)) {
        printf("Error: Unable to load account info. Remitting cancelled.\n");
        return;
    }

    char inputPIN[10];
    printf("Enter 4-digit PIN for account %s: ", senderAcc);
    fgets(inputPIN, sizeof(inputPIN), stdin);
    inputPIN[strcspn(inputPIN, "\n")] = '\0';
    if (checkCancel(inputPIN,"Remittance cancelled.","Cancelled remittance")) return;

    if (strcmp(inputPIN, senderPIN) != 0) {
        printf("Incorrect PIN. Returning to menu...\n");
        return;
    }

    printf("\nChoose the RECEIVER account:\n");

    for (int i = 0, display = 1; i < count; i++) {
        if (i == senderIndex) continue; // skip sender
        printf("%d: %s\n", display++, account[i]);
    }

    int receiverIndex = 0;

    while (true) {
        printf("Receiver account: ");
        fgets(buf, sizeof(buf), stdin);
        buf[strcspn(buf, "\n")] = '\0';
        if (checkCancel(buf, "Remittance cancelled.", "Cancelled remittance")) return;

        int choice = atoi(buf);

        int actualIndex = -1;
        int display = 1;

        for (int i = 0; i < count; i++) {
            if (i == senderIndex) continue;
            if (display == choice) {
                actualIndex = i;
                break;
            }
            display++;
        }

        if (actualIndex != -1) {
            receiverIndex = actualIndex;
            break;
        }

        int found = 0;
        for (int i = 0; i < count; i++) {
            if (i == senderIndex) continue;

            if (strcmp(account[i], buf) == 0) {
                receiverIndex = i;
                found = 1;
                break;
            }
        }
        if (found==1) break;
        printf("Invalid choice. Try again.\n");
    }

    char receiverAcc[50];
    strcpy(receiverAcc, account[receiverIndex]);

    char receiverFile[150];
    snprintf(receiverFile, sizeof(receiverFile), "database/%s.txt", receiverAcc);

    char receiverName[50], receiverIC[50], receiverType[50], receiverPIN[10];
    float receiverBalance;
    if (!getAccInfo(receiverFile, receiverName, receiverIC, receiverType, receiverPIN, &receiverBalance)) {
        printf("Error: Unable to load account info. Remitting cancelled");
        return;
    }

    float amount;
    float fee = 0;
    float rate = 0;

    if (strcmp(senderType, "savings") == 0 && strcmp(receiverType, "current") == 0) {
        rate = 0.02;
    }
    else if (strcmp(senderType, "current") == 0 && strcmp(receiverType, "savings") == 0) {
        rate = 0.03;
    }

    float maxSendable = senderBalance / (1 + rate);

    while (true) {
        printf("Enter remittance amount (Maximum sendable: RM %.2f): RM ", maxSendable);
        fgets(buf, sizeof(buf), stdin);
        buf[strcspn(buf, "\n")] = '\0';
        if (checkCancel(buf, "Remittance cancelled.", "Cancelled remittance")) return;

        amount = atof(buf);
        if (amount > 0 && amount <= maxSendable) break;

        printf("Invalid amount.\n");
    }

    fee = amount * rate;
    float total = amount + fee;

    senderBalance -= total;
    receiverBalance += amount;

    if (!writeAccInfo(senderFile, senderName, senderIC, senderType, senderPIN, &senderBalance) ||
        !writeAccInfo(receiverFile, receiverName, receiverIC, receiverType, receiverPIN, &receiverBalance)) {
        printf("Error: Unable to load account info. Remitting cancelled");
        return;
    }

    printf("\n=== REMITTANCE SUCCESSFUL ===\n");
    printf("From: %s\n", senderAcc);
    printf("To:   %s\n", receiverAcc);
    printf("Amount: RM %.2f\n", amount);
    printf("Fee:    RM %.2f\n", fee);
    printf("Total:  RM %.2f deducted\n", total);
    printf("Sender new balance:   RM %.2f\n", senderBalance);
    printf("Receiver new balance: RM %.2f\n", receiverBalance);

    logAction("Remittance successful");
}

int main() {
    start();
    srand(time(NULL));

    while (true) {
        printf("\n  ==================================================  \n");
        printf("|| Welcome to The Banking System!                   ||\n");
        printf("|| Choose the following options:                    ||\n");
        printf("||                                                  ||\n");
        printf("|| 1: Create a new bank account  (create)           ||\n");
        printf("|| 2: Delete a bank account      (delete)           ||\n");
        printf("|| 3: Deposit an amount          (deposit)          ||\n");
        printf("|| 4: Withdraw an amount         (withdraw)         ||\n");
        printf("|| 5: Remittance                 (remittance)       ||\n");
        printf("|| 6: Exit the program           (exit)             ||\n");
        printf("||                                                  ||\n");
        printf("  ==================================================  \n");

        char userInput[50];
        printf("Enter an option: ");
        fgets(userInput, sizeof(userInput), stdin);
        userInput[strcspn(userInput, "\n")] = 0;

        for (int i = 0; userInput[i] != '\0'; i++) {
            userInput[i] = (char)tolower(userInput[i]);
        }

        if (strcmp(userInput, "1") == 0 || strcmp(userInput, "create") == 0) {
            logAction("Creating account.");
            create();
        } else if (strcmp(userInput, "2") == 0 || strcmp(userInput, "delete") == 0) {
            logAction("Deleting account.");
            delete();
        } else if (strcmp(userInput, "3") == 0 || strcmp(userInput, "deposit") == 0) {
            logAction("Depositing an amount.");
            deposit();
        } else if (strcmp(userInput, "4") == 0 || strcmp(userInput, "withdraw") == 0) {
            logAction("Withdrawing an amount.");
            withdraw();
        } else if (strcmp(userInput, "5") == 0 || strcmp(userInput, "remittance") == 0) {
            logAction("Remitting an amount.");
            remittance();
        } else if (strcmp(userInput, "6") == 0 || strcmp(userInput, "exit") == 0) {
            logAction("Exiting program.");
            printf("Exiting the program.\n");
            return 0;
        } else {
            printf("Invalid option, please try again: ");
        }
    }
}
