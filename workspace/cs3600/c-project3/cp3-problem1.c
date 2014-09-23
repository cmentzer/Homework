/*
 * CS3600, Fall 2013
 * C Bootcamp, Homework 3, Problem 1
 * (c) 2012 Alan Mislove
 *
 * In this problem, your goal is to fill in the itoaaa function.
 * This function will take in a 32-bit signed integer, and will
 * return a malloc'ed char * containing the English representation
 * of the number.  A few examples are below:
 *
 * 0 -> "zero"
 * 9 -> "nine"
 * 45 -> "forty five"
 * -130 -> "negative one hundred thirty"
 * 11983 -> "eleven thousand nine hundred eighty three"
 *
 * Do not touch anything outside of the itoaaa function (you may,
 * of course, define any helper functions you wish).  You may also
 * use any of the functions in <string.h>. 
 *
 * Finally, you must make sure to free() any intermediate malloced()
 * memory before you return the result.  You should also return a 
 * char* that is malloced to be as small as necessary (the script 
 * checks for this).  For example, if you returned "forty five", you
 * should put this into a malloced space of 12 bytes (11 + '\0').
 *
 */

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

char *itoaaa(int i);

int main(int argc, char **argv) {
  // check for the right number of arguments
  if (argc != 2) {
    printf("Error: Usage: ./cp3-problem1 [int]\n");
    return 0;
  }

  // create the time structure
  long long arg = atoll(argv[1]);
  if (arg == (int) arg) {
    // call the function
    char *result = itoaaa((int) arg);

    // print out the result
    printf("%d is: %s\n", (int) arg, result);
  } else {
    printf("Error: Number out of range.\n");
  }

  return 0;
}

/**
 * This function should print out the English full representation
 * of the passed-in argument.  See above for more details.
 */
char *itoaaa(int arg) {
  // TODO:  Fill in this function
  char *billion = "billion ";
  char *million = "million ";
  char *thousand = "thousand ";
  char *hundred = "hundred ";
  const char *tens[10];
	tens[0] = "";
	tens[1] = "one ";
  	tens[9] = "ninety ";
  	tens[8] = "eighty ";
  	tens[7] = "seventy ";
  	tens[6] = "sixty ";
  	tens[5] = "fifty ";
  	tens[4] = "forty ";
  	tens[3] = "thirty ";
  	tens[2] = "twenty ";
  const char *teens[10];
	teens[0] = "ten ";
	teens[9] = "nineteen ";
	teens[8] = "eighteen ";
	teens[7] = "seventeen ";
	teens[6] = "sixteen ";
	teens[5] = "fifteen ";
	teens[4] = "fourteen ";
	teens[3] = "thirteen ";
	teens[2] = "twelve ";
  	teens[1] = "eleven ";
  const char *ones[10];
	ones[0] = "";
  	ones[9] = "nine ";
  	ones[8] = "eight ";
  	ones[7] = "seven ";
  	ones[6] = "six ";
  	ones[5] = "five ";
  	ones[4] = "four ";
  	ones[3] = "three ";
  	ones[2] = "two ";
  	ones[1] = "one ";
  const char *negative = "negative ";

  char *output = (char *) calloc (300, sizeof(char));
  int firstnum = 0;
  int secondnum = 0;
  int rem = 0;
  int arg_mils = 0; 
  if(arg < 0){
	strcpy(output, negative);
	arg = arg * -1;
  } else if(arg == 0){
	strcpy(output, "zero ");
  } else {
	strcpy(output, "");
  }
  if(arg >= 1000000){
	arg_mils = arg;
  }
  if(arg_mils >= 1000000000){
	firstnum = arg_mils / 1000000000;
	strcat(output, ones[firstnum]);
	strcat(output, billion);
	arg_mils = arg_mils - (firstnum * 1000000000);
  }
  if(arg_mils >= 100000000){
	firstnum = arg_mils / 100000000;
	strcat(output, ones[firstnum]);
	strcat(output, hundred);
	arg_mils = arg_mils - (firstnum * 100000000);
  } 
  if(arg_mils >= 1000000){
	firstnum = arg_mils / 1000000;
	if(firstnum < 20){
		firstnum = firstnum - 10;
		strcat(output, teens[firstnum]);
		strcat(output, million);
		arg_mils = arg_mils - ((firstnum + 10) *1000000);
	} else if(firstnum >= 20){
		secondnum = firstnum / 10;
		firstnum = firstnum - (secondnum * 10);
		strcat(output, tens[secondnum]);
		strcat(output, ones[firstnum]);
		strcat(output, million);
		arg_mils = arg_mils - ((secondnum * 10 + firstnum) * 1000000);
	}
  } else if(firstnum >= 1000000){
	firstnum = firstnum / 1000000;
	strcat(output, ones[firstnum]);
	strcat(output, million);
	arg_mils = arg_mils - (firstnum * 1000000);
  } else if(arg >= 1000000 && arg < 1000000000) {
	strcat(output, million);
  }
  if(arg >= 1000000){
	arg = arg_mils;
  } 	
  if(arg >= 100000){	
	firstnum = arg / 100000;
	strcat(output, ones[firstnum]);
	strcat(output, hundred);
	firstnum = arg - (firstnum *100000);
	rem = (firstnum);
  }
  if(arg < 100000){ 
	firstnum = arg; 
	rem = arg;
  }
  if(firstnum >= 10000){ 
	firstnum = firstnum / 1000;
	if(firstnum < 20){
		firstnum = firstnum - 10;
		strcat(output, teens[firstnum]);
		strcat(output, thousand);
		rem = rem - ((firstnum + 10) *1000);
	} else if(firstnum >= 20){
		secondnum = firstnum / 10;
		firstnum = firstnum - (secondnum * 10);
		strcat(output, tens[secondnum]);
		strcat(output, ones[firstnum]);
		strcat(output, thousand);
		rem = rem - ((secondnum * 10 + firstnum) * 1000);
	}
  } else if(firstnum >= 1000){
	firstnum = firstnum / 1000;
	strcat(output, ones[firstnum]);
	strcat(output, thousand);
	rem = rem - (firstnum * 1000);
  } else if(arg >= 1000 && arg < 1000000) {
	strcat(output, thousand);
  }
  if(arg < 1000){ 
	rem = arg;
  }
  if(rem >= 100){
	firstnum = rem /100; 
	strcat(output, ones[firstnum]);
	strcat(output, hundred);
	rem = rem - (firstnum * 100);
  } 

  if(rem < 20){
	firstnum = rem - 10;
	strcat(output, teens[firstnum]);
  } else if(rem >= 20){
	secondnum = rem / 10;
	firstnum = rem - (secondnum * 10);
	strcat(output, tens[secondnum]);
	strcat(output, ones[firstnum]);
  }
  int i = 0;
  while(output[i] != '\0'){
	i++;
  }
  i--; 
  output[i] = '\0';
  char *output_resized = (char *) malloc(strlen(output)+1);
  strcpy(output_resized, output);
  free(output);	
  return output_resized;
}
