/*
 * CS3600, Fall 2013
 * C Bootcamp, Homework 1, Problem 4
 * (c) 2012 Alan Mislove
 *
 * In this problem, your goal is to explore the use of the printf() function in the
 * do_stuff function.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// the prototype of the function
double do_stuff(int a, float b, char c, double d, int e);

int main(int argc, char *argv[]) {
  // check for the right number of arguments
  if (argc != 6) {
    printf("Error: Usage: ./cp1-problem4 [int] [float] [char] [double] [int]\n");
    return 0;
  }
  
  // interpret the variables
  int a = atoi(argv[1]);
  float b = (float) atof(argv[2]);
  char c = argv[3][0];
  double d = atof(argv[4]);
  int e = atoi(argv[5]);
  
  // calculate the result
  double result = do_stuff(a, b, c, d, e);
  
  // print it out
  if (result >= 0) {
    printf("Success: Completed!\n");
    return 0;
  } else {
    printf("Error: Unknown error.");
    return -1;
  }
}

/**
 * This function should print out (using printf) the following text
 * 
 * Int: <int in lowercase hexadecimal, prefixed with 0x>\n
 * Float: <float, padded to 3 digits before the . and 5 after>\n
 * Char: <the ASCII value of the char, in octal>\n
 * Double: <the double, in scientific notation>\n
 * Short: <the int cast to a short, in decimal>\n
 *
 * As an example, the first line is provided.  The return value from 
 * this function should be 0.
 */
double do_stuff(int a, float b, char c, double d, int e) {
  printf("Int: 0x%x\n", a);

  // Print the values of the variables a, b, c, d, and e; as defined above.
  printf("Float: %09.5f\n", b);
  int charValue = c;
  printf("Char: %o\n", charValue);
  printf("Double: %e\n", d);
  short s = (short) e;
  printf("Short: %d\n", s);
  return 0;
}









