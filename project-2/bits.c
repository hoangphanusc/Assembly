/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
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
/* Copyright (C) 1991-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */
/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */
/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */
/* wchar_t uses Unicode 8.0.0.  Version 8.0 of the Unicode Standard is
   synchronized with ISO/IEC 10646:2014, plus Amendment 1 (published
   2015-05-15).  */
/* We do not support C11 <threads.h>.  */
/* 
 * absVal - absolute value of x
 *   Example: absVal(-1) = 1.
 *   You may assume -TMax <= x <= TMax
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 10
 *   Rating: 4
 */
int absVal(int x) {
  //Create a mask with last bit of x (all 1s if x is negative, else all 0s)
  //xor x with mask to get ~x when x is negative, x when x is positive
  //Add negation of mask to result to +1 when mask is all 1, +0 when mask is
  //all 0s.
  int mask = x>>31;
  x = (x ^ mask) + ((~mask) + 1);
  return x;
}
/*
 * ezThreeFourths - multiplies by 3/4 rounding toward 0,
 *   Should exactly duplicate effect of C expression (x*3/4),
 *   including overflow behavior.
 *   Examples: ezThreeFourths(11) = 8
 *             ezThreeFourths(-9) = -6
 *             ezThreeFourths(1073741824) = -268435456 (overflow)
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 3
 */
int ezThreeFourths(int x) {
  //First multiply x by 3 by calc-ing 2x + x
  //if 3x is positive then 3x<<2 will be automatically rounded down.
  //We need to add 1 to x/4 if the 1st and 2nd bit of 3x are not both 0,
  //and the last bit is 1 (meaning x is negative; 3x/4 needs to be round up)
  //last & (first | second) evals to 1 if true. Add this to 3x/4.
  int threex = (x << 1) + x;
  int firstbit = threex & 0x1;
  int secondbit = (threex & 0x2) >> 1;
  int lastbit = threex>>31;
  x = (threex >> 2) + ((firstbit | secondbit) & lastbit);
  return x;
}
/* 
 * float_abs - Return bit-level equivalent of absolute value of f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument..
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned float_abs(unsigned uf) {
  //To check for NaN, check for the first 31 bits of uf > 0x7F800000 (discussion)
  //To get absolute value, change the last bit to 0 (so uf & first 31 bits)
  int first31 = uf & 0x7fffffff;
  if (first31 > 0x7F800000){
    return uf;
  }
  return uf & first31;
}
/* 
 * float_half - Return bit-level equivalent of expression 0.5*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_half(unsigned uf) {
  //Check uf for NaN or infinite by first31bits >= 0x07800000
  //if NaN or infinite, return argument
  //Check uf for denormalized value by <= 0x007fffff
  
  //Denormal: if uf is 0, return uf 
  //Else right shift fraction by 1. take care of rounding by +1
  //if the lost bit is 1 and the new first bit is 1

  //Normal: minus 1 from exponent part
  //If the resulting number is still normal, return uf
  //Else right shift fraction by 1 and make the last fraction bit 1
  //to simulate moving the binary point in front of the significant 1.
  //Take care of rounding the fraction part again then return.


  int lastbit = uf & 0x80000000;
  int first31 = uf & 0x7fffffff;
  int first23 = uf & 0x007fffff;
  int last9;
  if (first31 >= 0x7f800000){ //Check NaN or infinite
    return uf;
  }
  if (first31 <= 0x007fffff){ //Check denormalized
    if (!first23) {
      return uf;
    }
    if (first23 & 2){
      return (lastbit | ((first23/2) + (first23&1)) );
    }
    return (lastbit | (first23/2));
  }
  uf = uf - 0x00800000;
  first31 = uf & 0x7fffffff;
  last9 = uf & 0xFF800000;
  if (first31 <= 0x007fffff) {
    if (first23 & 2){
      return (last9 |  (first23/2 + (first23&1)) | 0x00400000);
    }
    return (last9 | ((first23/2) | 0x00400000));
  }
  return uf;
}
/*
 * satMul2 - multiplies by 2, saturating to Tmin or Tmax if overflow
 *   Examples: satMul2(0x30000000) = 0x60000000
 *             satMul2(0x40000000) = 0x7FFFFFFF (saturate to TMax)
 *             satMul2(0x60000000) = 0x80000000 (saturate to TMin)
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int satMul2(int x) {
  //sum = 2x
  //if last bits of x and sum are the same, there wasn't an overflow
  //  so check with lastbitx ^ lastbitsum
  //  make a mask out of this => all 1s if diff signs, 0s if same.
  //else
  //  if last bit of x is 0 (+) and of sum is 1 (-), return Tmax
  //  else return Tmin.
  //  but Tmax + 1 = Tmin. So just return (Tmax + lastbitx)
  //To decide which to return based on the sign difference,
  //(sum & ~mask) + (tmax & mask) evals to: sum if same sign,
  //tmax if diff sign. Add lastbitx if the signs were different.


  int tmax = 0x7F << 24 | 0xFF << 16 | 0xFF << 8 | 0xFF;

  int sum = x + x;
  int lastbitsum = sum >> 31 & 1;
  int lastbitx = x >> 31 & 1;
  int diffsign = lastbitsum ^ lastbitx;
  int mask = (diffsign << 31) >> 31;

  return (sum & ~mask) + (tmax & mask) + (diffsign & lastbitx);
}
