#include <stdio.h>

void get_input(char stuff [], int len);

int main(void){
char username[10];
char password[10];


char secret[] = "Secret: If war doesn't change, men must change, and so must their symbols.";

printf("Enter Username: ");
get_input(username, sizeof(username)); 

printf("Enter Password: ");
get_input(password, sizeof(password));

printf("\nYou Entered:\n");
printf("%s", username);
printf("%s\n", password);

return 0;
}
