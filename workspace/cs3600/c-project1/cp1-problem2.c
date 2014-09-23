/*
 * CS3600, Fall 2013
 * C Bootcamp, Homework 1, Problem 2
 * (c) 2012 Alan Mislove
 *
 * In this problem, your goal is to three integers as input, and perform a few bitwise
 * calculations.  You will write functions that will do the following:
 * 
 * 1. You will determine the integer that has the highest number of bits set as '1', and
 *    will return it (function most_ones).
 *
 * 2. You will take the exclusive-OR of all three integers, and return it as an integer
 *    (function xor_all).
 *
 * 3. You will reverse the order of the bits of the first integer, and will return the
 *    result as an integer (function reverse_bits).
 */

#include <stdio.h>
#include <stdlib.h>

// the prototype of the first function
int most_ones(int a, int b, int c);

// the prototype of the second function
int xor_all(int a, int b, int c);

// the prototype of the third function
int reverse_bits(int a);

int main(int argc, char *argv[]) {
  // check for the right number of arguments
  if (argc != 4) {
    printf("Error: Usage: ./cp1-problem2 [int1] [int2] [int3]\n");
    return 0;
  }

  // interpret the variables
  int int1 = atoi(argv[1]);
  int int2 = atoi(argv[2]);
  int int3 = atoi(argv[3]);
  
  // calculate the result for the first problem
  int result1 = most_ones(int1, int2, int3);  
  printf("Success: The argument with the most 1s is: %d\n", result1);

  // calculate the result for the second problem
  int result2 = xor_all(int1, int2, int3);  
  printf("Success: The XOR of all three ints is: %d\n", result2);

  // calculate the result for the third problem
  int result3 = reverse_bits(int1);  
  printf("Success: The reversal of the bits of %d is: %d\n", int1, result3);
 
  return 0;
}

/**
 * This function takes in three ints and returns the int that has the most
 * bits set to 1.  If multiple arguments have the same number of bits set to
 * 1, the function should return the *first* such argument.
 */
int most_ones(int a, int b, int c) {
  /** Originally I had this function call a helper function 
      called num_ones which returned the number of bits of 
      value 1 in a given int's binary representation, but I
      re-read the instructions and saw that we were only 
      "supposed" to change the code in the sections with 
      TODO, so I re-designed this function accordingly.  
  */

  // The number of ones in each varable a, b, and c:
  int a_ones = 0;
  int b_ones = 0;
  int c_ones = 0;
  for(int i = 0; i < 32; i++){
	if(((a >> i) & 1) == 1){
		a_ones++;
	}
  }
  for(int i = 0; i < 32; i++){
	if(((b >> i) & 1) == 1){
		b_ones++;
	}
  }
  for(int i = 0; i < 32; i++){
	if(((c >> i) & 1) == 1){
		c_ones++;
	}
  }
  // Find the variable with the lowest number of ones:
  int out;
  if(a_ones >= b_ones && a_ones >= c_ones){
	out = a;
  } else if(b_ones > a_ones && b_ones >= c_ones){
	out = b;
  } else {
	out = c;
  }
  return out;
}
/**
 * This function takes in three ints and calculates the exclusive OR (XOR) of
 * all three ints, and returns the result as an int.
 */
int xor_all(int a, int b, int c) {
  // TODO: Fill in this function
  int out = 0;
  out = a ^ b;
  out = out ^ c; 
  return out;
}

/**
 * This function takes in an integer and reverses the order of the bits, and
 * returns the result.  For example, if 40 is the input value, this is
 *   0000 0028
 * in hex.  Reversed, this would be
 *   1400 0000
 * the decimal value of which would be 335544320.
 */
int reverse_bits(int a) {
  // This function was difficult to get working
  /** 
	This function essentially appends the last bit of the input value to 
  	the beginning of the output value. It accomplishes this by shifting 
 	the input value to the right at each iteration of the loop and the
	output value to the left, so that the position of the "active" bit 
	is the same relative to the last input bit as it is to the first out-
 	put bit, reversing the order of the bits. This function ensures that 
	upon each iteration only the "active" bit can be changed by using 
	bitwise OR with the input AND 1, which will maintain the positions 
	of the bits in the output so far but allow the current last bit in 
	the output to be changed.  
  */
  int out = (a & 1);
  for(int i = 0; i < 31; i++){
        out = out << 1;
        a = a >> 1;
	out = out | (a & 1);
  }
  return out;
}





