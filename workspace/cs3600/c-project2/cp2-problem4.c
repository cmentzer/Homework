/*
 * CS3600, Fall 2013
 * C Bootcamp, Homework 2, Problem 4
 * (c) 2012 Alan Mislove
 *
 * In this problem, your goal is to learn about pointers.
 * Note that in this exercise, you are allowed to redefine the
 * swap functions (i.e., change the type of arguments that it takes),
 * and you may change the call to swap() in main().  Do not change
 * anything else in main().
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// you may need to change this definition as well
void swap(char *a, char *b);

int main(int argc, const char **argv) {
  // check for the right number of arguments
  if (argc != 3) {
    printf("Error: Usage: ./cp2-problem3 [str1] [str2]\n");
    return 0;
  }

  char *str1 = (char *) malloc(strlen(argv[1])+1);
  strcpy(str1, argv[1]);

  char *str2 = (char *) malloc(strlen(argv[2])+1);
  strcpy(str2, argv[2]);

  // you may change the arguments to this call
  // you may not add any additional lines of code to
  // main(), though
  swap(str1, str2);

  printf("The swapped values are: %s %s\n", str1, str2);
}

/**
 * This function should take in two strings and swap the values.  You 
 * are not guaranteed that the strings will be the same length.
 * You may, however, slightly modify the arguments to this function (hint).
 * As a result, you may change the call to this function (and only the
 * call to this function).
 */
void swap(char *a, char *b) {
  // TODO: Modify this function so that a and b are swapped.
  char *temp = malloc(sizeof(a) + sizeof(b));
  int a_len = 0;
  int b_len = 0;
  while(*a !='\0'){
	*temp = *a;
	temp++;
	a++;
	a_len++;
  }
  temp++;
  *temp = '\0';
  temp = temp - (a_len + 1);
  a = a - a_len;
  while(*b != '\0'){
	*a = *b;
	a++;
	b++;
	b_len++;
  } 
  *a  = '\0';
  a = a - (b_len); 
  b = b - (b_len);
  while(*temp != '\0'){
	*b = *temp;
	b++;
	temp++;
  }
  *b = '\0'; 
}

