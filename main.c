#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <direct.h>
#include <time.h>
#include <ctype.h>

void logAction(char *action) {
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
    char *accountTypes[] = {"savings","current"};
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
    int num;
    char buffer[20];

    while (true) {
        int length = (rand() % 3) + 7;
        int min = 1;
        for (int i = 1; i < length; i++) {
            min *= 10;
        }
        int max = (min * 10) - 1;

        num = (rand() % (max - min + 1)) + min;
        sprintf(buffer, "%d", num);

        if (!checkExists(buffer)) {
            strcpy(output, buffer);
            return;
        }
    }
}

bool checkCreationCancel(char *input) {
    char temp[50];
    for (int i = 0; i < strlen(input); i++) {
        temp[i] = (char)tolower(input[i]);
    }
    temp[strlen(input)] = '\0';
    if (strcmp(temp, "cancel") == 0 || strcmp(temp, "exit") == 0) {
        printf("Account creation cancelled.\n");
        logAction("Cancelled account creation");
        return true;
    }
    return false;
}

bool checkDeletionCancel(char *input) {
    char temp[50];
    for (int i = 0; i < strlen(input); i++) {
        temp[i] = (char)tolower(input[i]);
    }
    temp[strlen(input)] = '\0';
    if (strcmp(temp, "cancel") == 0 || strcmp(temp, "exit") == 0) {
        printf("Account deletion cancelled.\n");
        logAction("Cancelled account deletion");
        return true;
    }
    return false;
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
    if (checkCreationCancel(name)) return;

    while (true) {
        printf("Enter your Identification Card(IC) number (12 digits): ");
        fgets(ic, sizeof(ic), stdin);
        ic[strcspn(ic, "\n")] = 0;
        if (checkCreationCancel(ic)) return;
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
        if (checkCreationCancel(accountType)) return;
        if (validateAccount(accountType)) {
            break;
        }
    }

    while (true) {
        printf("Enter your 4-digit PIN: ");
        fgets(pin, sizeof(pin), stdin);
        pin[strcspn(pin, "\n")] = 0;
        if (checkCreationCancel(pin)) return;
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
    printf("Account successfully created!\n");
}

void start() {
    if (_mkdir("database") == 0) {
        printf("New directory created successfully.\n");
    } else {
        printf("Directory found.\n");
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
}

void delete() {
    FILE *file = fopen("database/index.txt", "r");
    if (!file) {
        printf("No accounts found.\n");
        return;
    }

    char account[100][50];
    int count = 0;

    while (fgets(account[count], sizeof(account[count]), file)) {
        account[count][strcspn(account[count], "\n")] = '\0';
        count++;
    }

    fclose(file);

    if (count == 0) {
        printf("No accounts available.\n");
        return;
    }

    printf("Type 'cancel' or 'exit' to cancel account deletion at any point.\n");
    printf("Choose one of the following account(s) to delete:\n");

    for (int i = 0; i < count; i++) {
        printf("%d: %s\n", i+1 ,account[i]);
    }

    int choice = 0;
    char buf[20];

    while (true) {
        printf("Enter number to delete: ");
        if (!fgets(buf, sizeof(buf), stdin)) {
            printf("Input error.\n");
            return;
        }
        buf[strcspn(buf, "\n")] = '\0';
        if (checkDeletionCancel(buf)) return;
        choice = atoi(buf);
        if (choice >= 1 && choice <= count) {
            break;
        }
        printf("Invalid choice. Try again.\n");
    }

    char *accDelete = account[choice - 1];
    char accFile[150];
    snprintf(accFile, sizeof(accFile), "database/%s.txt", accDelete);

    FILE *accToDelete = fopen(accFile, "r");
    if (accToDelete == NULL) {
        printf("Error opening account file.\n");
        return;
    }
    char ic[50], pin[10], line[50];

    while (fgets(line, sizeof(line), accToDelete) != NULL) {
        if (strncmp(line, "IC:", 3) == 0)
            sscanf(line, "IC: %49s", ic);
        else if (strncmp(line, "PIN:", 4) == 0) {
            sscanf(line, "PIN: %9s", pin);
        }
    }
    fclose(accToDelete);

    const char *userID = ic + strlen(ic) - 4;

    char inputAcc[50], inputIC[10], inputPIN[10];

    printf("--- Account Confirmation Required ---\n");

    printf("Reenter account number: ");
    fgets(inputAcc, sizeof(inputAcc), stdin);
    inputAcc[strcspn(inputAcc, "\n")] = '\0';
    if (checkDeletionCancel(inputAcc)) return;

    printf("Enter last 4 characters of IC number: ");
    fgets(inputIC, sizeof(inputIC), stdin);
    inputIC[strcspn(inputIC, "\n")] = '\0';
    if (checkDeletionCancel(inputIC)) return;

    printf("Enter 4-digit PIN: ");
    fgets(inputPIN, sizeof(inputPIN), stdin);
    inputPIN[strcspn(inputPIN, "\n")] = '\0';
    if (checkDeletionCancel(inputPIN)) return;

    if (strcmp(inputAcc, accDelete) == 0 &&
        strcmp(inputIC, userID) == 0 &&
        strcmp(inputPIN, pin) == 0)
    {
        printf("Verification successful. Account will be deleted.\n");
    }
    else {
        printf("Verification failed. Account will NOT be deleted.");
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
        if (i != choice - 1)
            fprintf(out, "%s\n", account[i]);
    }

    fclose(out);

    printf("Account '%s' deleted successfully.\n", accDelete);
    logAction("Account deleted successfully");
}

int main() {
    start();
    srand(time(NULL));

    int exit = 0;
    while (exit == 0) {
        printf("\n  ==================================================  \n");
        printf("|| Welcome to The Banking System!                   ||\n");
        printf("|| Choose the following options:                    ||\n");
        printf("||                                                  ||\n");
        printf("|| 1: Create a new bank account                     ||\n");
        printf("|| 2: Delete a bank account                         ||\n");
        printf("|| 3: Deposit an amount                             ||\n");
        printf("|| 4: Withdraw an amount                            ||\n");
        printf("|| 5: Remittance                                    ||\n");
        printf("|| 6: Exit the program                              ||\n");
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
            printf("deposit account\n");
        } else if (strcmp(userInput, "4") == 0 || strcmp(userInput, "withdraw") == 0) {
            logAction("Withdrawing an amount.");
            printf("withdraw account\n");
        } else if (strcmp(userInput, "5") == 0 || strcmp(userInput, "remittance") == 0) {
            logAction("Remitting an amount.");
            printf("remittance account\n");
        } else if (strcmp(userInput, "6") == 0 || strcmp(userInput, "exit") == 0) {
            logAction("Exiting program.");
            printf("Exiting the program\n");
            exit = 1;
        } else {
            printf("Invalid option, please try again: ");
        }
    }
}