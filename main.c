#include <stdio.h>
#include <stdlib.h>

int main() {

    int exit = 0;
    int userInput;

    while (exit == 0) {
        printf("  ==================================================  \n");
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

        printf("Enter an option: ");
        scanf("%d",&userInput);
        printf("You entered: %d", userInput);
        exit = 1;
    }
}
