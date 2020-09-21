/* 
 * CS:APP Data Lab 
 * 
 * <STEVEN CHU     UID: 905-094-800>  * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* 
 * bitAnd - x&y using only ~ and | 
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) {
  /* Utilize deMorgan's theorem */
  return ~((~x) | (~y));
}
/* 
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int getByte(int x, int n) {
/* shift x right such that the byte we want to extract is the LSB
 * zero out all other bytes using the & operator*/ 
  n = n << 3;
  x = x >> n; 
  x = x & 255;
  return x;
}
/* 
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3 
 */
int logicalShift(int x, int n) {
/* Generate a bit vector that will replace the appropriate number of 1 bits
 * with 0 bits.
 * Use arithmetic right shift on x, and apply & operator to zero out MSBs. */  
  int i = ~(1 << 31); 
  i = i >> n;
  i = i << 1;
  i = i | 1;
  x = x >> n;
  x = x & i;
  return x;
}
/* 
 * rotateRight - Rotate x to the right by n
 *   Can assume that 0 <= n <= 31
 *   Examples: rotateRight(0x87654321,4) = 0x76543218
 *   Legal ops: ~ & ^ | + << >> !
 *   Max ops: 25
 *   Rating: 3 
 */
int rotateRight(int x, int n) {
/* create bit vector j that will grab the LSBs that are rotated out
 * left shift j so that the LSBs are moved to MSBs position 
 * logical right shift x the appropriate number of bits 
 * using the | operator, insert the MSBs present in j */
  int j = 0;
  int i = ~0 << n;
  int k = ~(1 << 31); 
  k = k >> n;
  k = k << 1;
  k = k | 1;
  i = i ^ ~0;
  j = x & i;
  j = j << (32+((~n)+1));
  x = x >> n;
  x = x & k;
  x = x | j;
return x;
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
/* compute a bit array that is all 1 if x is zero, and all 0 if x is
 * nonzero. 
 * utilize the ^ operator: if x is zero, XOR z will be applied twice,
 * and cancel out, returning y. 
 * if x is nonzero, the inside statement will return a zero vector, and XOR
 * will yield z, which is then returned. */
  int i = (!x) + (~0);
  return z ^ ((y ^ z) & i);
}
/* 
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int bang(int x) {
/* Using the | operator on x and its complement will yield a bit vector
 * where the MSB is 1 if x is nonzero, and 0 if it is zero. 
 * Arithmetic right shift until the original MSB is in the LSB position. 
 * Adding 1 will then either yield 0 by overflow, or 1 if x is the zero bit
 * vector. */  
  x = ((~x)+1) | x;
  x = x >> 31;
  x = x + 1;
  return x;
}
/*
 * bitParity - returns 1 if x contains an odd number of 0's
 *   Examples: bitParity(5) = 0, bitParity(7) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 4
 */
int bitParity(int x) {
/* Repeatedly XOR the first half of the bit sequence with the second half
 * If there is an odd number of zeros, it is impossible for XOR to yield a 0.
 * If there is an even number, XOR on the halves will always converge to 0.
 * Zero out the sequence except for the LSB. */
  x = x ^ (x >> 16);
  x = x ^ (x >> 8);
  x = x ^ (x >> 4);
  x = x ^ (x >> 2);
  x = x ^ (x >> 1);
  return x & 1;
}
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
/* Compute if x is equal to -1 
 * Adding 1 to x and XORing with itself will result in a zero-bit vector if x is
 * INT_MAX. It will also occur if x is -1. Check to make sure x is not -1.
 * Use bang operator to swap values and return. */
  int i = !(x ^ ~0);
  return !((~(x+1) ^ x) ^ i);
}
/* 
 * fitsBits - return 1 if x can be represented as an 
 *  n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int fitsBits(int x, int n) { 
/* Compute 32 - n. 
 * Left shift 32 - n bits so that the n - 1 bit is shifted to sign bit.
 * Right shift back to original position. 
 * XOR will return 0 only if x fits in n bits */ 
  int i = 32 + (~n + 1);
  int j = x << i;
  j = j >> i;
  return !(j ^ x);
}
/* 
 * divpwr2 - Compute x/(2^n), for 0 <= n <= 30
 *  Round toward zero
 *   Examples: divpwr2(15,1) = 7, divpwr2(-33,4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int divpwr2(int x, int n) {
/* Generate a 1 vector or 0 vector if x is negative or positive.
 * Compute the proper bias utilize & operator to zero out the bias if x is
 * positive.
 * Add bias and perform division using right shift */
  int i = x >> 31;
  int j = i & ((1 << n) + ~0);
  return (x + j) >> n;
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
/* Use 2's-complement negation technique:
 * Take the complement of x, and add 1 to LSB. */
  return ~x + 1;
}
/* 
 * isPositive - return 1 if x > 0, return 0 otherwise 
 *   Example: isPositive(-1) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 3
 */
int isPositive(int x) {
/* Generate a 0 vector or a 1 vector based on the sign bit of x. 
 * Using the OR operator, account for case where x = 0. 
 */
  int i = x >> 31;
  return !((!x) | i);
}
/* 
 * isGreater - if x > y  then return 1, else return 0 
 *   Example: isGreater(4,5) = 0, isGreater(5,4) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isGreater(int x, int y) {
/* Compute both the sum and difference of x and y. 
 * Compute the sign of the difference and whether or not x and y are the same
 * sign.
 * Compute whether or not overflow has occurred (x and y are same sign, and sum
 * is different sign). 
 * If x is not negative while y is positive, and the difference is positive 
 * without overflow, x is greater than y. */ 
  int i = y + (~x + 1);
  int j = x + y;
  int k = ((x ^ y) >> 31) & 1;
  i = !(i >> 31);
  j = k & ((x ^ j) >> 31);
  x = (x >> 31) & 1;
  return !(((i & ~j) & !k) | (x & k));
}
/* 
 * subOK - Determine if can compute x-y without overflow
 *   Example: subOK(0x80000000,0x80000000) = 1,
 *            subOK(0x80000000,0x70000000) = 0, 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int subOK(int x, int y) {
/* Compute the difference between x and y.
 * Determine if x and y are the same sign
 * If the difference is a different sign than x, and x and y are the same sign:
 * overflow has occured. */  
  int i = x + (~y + 1);
  int j = ((x ^ y) >> 31) & 1;
  return !(j & ((x ^ i) >> 31));
}
/*
 * ilog2 - return floor(log base 2 of x), where x > 0
 *   Example: ilog2(16) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 90
 *   Rating: 4
 */
int ilog2(int x) {
/* Computing the base 2 log is equivalent to finding the index of the most
 * significant bit.
 * Using 5 check bits at 16, 8, 4, 2, and 1, all possible indices can be accounted
 * for.
 * For each check: If MSB is to the left of the check bit, right shift x
 * until the check bit is the LSB, and add the number to the total.
 * If MSB is to the right, do nothing. After all 5 checks, return the total. */
  int i = !(x >> 16);
  int j = 0;
  i = i << 31;
  i = i >> 31;
  x = x >> (~i & 16);
  j = j + (~i & 16);
  i = !(x >> 8);
  i = i << 31;
  i = i >> 31;
  x = x >> (~i & 8);
  j = j + (~i & 8);
  i = !(x >> 4);
  i = i << 31;
  i = i >> 31;
  x = x >> (~i & 4);
  j = j + (~i & 4);
  i = !(x >> 2);
  i = i << 31;
  i = i >> 31;
  x = x >> (~i & 2);
  j = j + (~i & 2);
  i = !(x >> 1);
  i = i << 31;
  i = i >> 31;
  x = x >> (~i & 1);
  j = j + (~i & 1);
  return j; 
}
