/* 
 * hello.c - the quintessential hello world program
 */
#include <stdio.h>
#include <string.h>

int main () {
  printf("1. Dor Karter\n");
  printf("2. A20238543\n");
  
  char str[1024];
  strcpy(str,"3. A man experiencing erectile dysfunction goes to see a\n");
  strcat(str,"   doctor. The doctor asks him about his eating habits,\n");
  strcat(str,"   whether he exercises or not, and what he does for a\n");
  strcat(str,"   living. After considering everything he has heard,\n");
  strcat(str,"   he asks to see a picture of the man's wife. The man\n");
  strcat(str,"   opens his wallet and shows him. The doctor nods, takes a\n");
  strcat(str,"   small bottle from a cabinet in the room, and hands it to\n");
  strcat(str,"   the man. \"Here is some free samples. Take one of these\n");
  strcat(str,"   any time you want to have an erection.\" The man reads\n");
  strcat(str,"   the bottle aloud, \"May cause temporary blindness?!\"\n");
  printf(str);
  
  return 0;
}
