/*
 * CS3600, Fall 2013
 * C Bootcamp, Homework 4, Problem 2
 *
 * In this problem, your goal is to learn about debugging C programs.
 * The expected output of the following C program is to print the 
 * elements in the array. But when actually run, it doesn't do so.
 *
 * Fix it.
 */
#include<stdio.h>

#define TOTAL_ELEMENTS (sizeof(array) / sizeof(array[0]))
int array[] = {23,34,12,17,204,99,16};

int main() {
  int d;

  for (d=0;d <= (TOTAL_ELEMENTS-1); d++)
    printf("%d\n",array[d]);

  return 0;
}
