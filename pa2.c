//---------------------------------------------------------------
//
//  4190.308 Computer Architecture (Fall 2020)
//
//  Project #2: FP12 (12-bit floating point) Representation
//
//  September 28, 2020
//
//  Injae Kang (abcinje@snu.ac.kr)
//  Sunmin Jeong (sunnyday0208@snu.ac.kr)
//  Systems Software & Architecture Laboratory
//  Dept. of Computer Science and Engineering
//  Seoul National University
//
//---------------------------------------------------------------

#include "pa2.h"

/* Convert 32-bit signed integer to 12-bit floating point */
fp12 int_fp12(int n){
	if (!n){
		return 0;
	}

	if (n == -2147483648)		//too hard to manage as -n dosnt work
		return 0xFFC0;
	int result = 0;
	int sign = 0;
	if (n<0){
		sign = 1;
		n = -n;
	}

	int exp = 0;
	int tmp = n;
	while (tmp > 1){
		tmp /= 2;
		exp++;
	}


	int mantissa = 0;
	mantissa = n >> (exp - 5);
	int R = (n >> (exp - 6)) & 0x00000001;
	int L = (n >> (exp - 5)) & 0x00000001;
	int X = n << (32-(exp - 5));
	if ((X>0 && R == 1) || (L == 1 && R == 1)){
		mantissa = 0;
		exp++;
	}

	exp += 31;		//bias

	result = (sign << 15) + (sign << 14) + (sign << 13) + (sign << 12) + (sign << 11);
	if (exp >= 63 || exp <= -1){
		exp = 63;
		mantissa = 0;
	}

	result |= exp << 5;
	result |= mantissa;

	return result;
}


/* Convert 12-bit floating point to 32-bit signed integer */
int fp12_int(fp12 x)
{
	int result = 0;
	int signbit = 0;
	int exp = 0;
	int mant = 0;
	signbit = x >> 15;
	exp = ((x & 0x07FF) >> 5) - 31;
	mant = x & 0x001F;
	float expcalc = 1;



	float mantcalc = 1;
	if (exp == -31)
		mantcalc = 0;
	mantcalc += (float) (mant >> 4) * 1/2 + (float) ((mant >> 3) & 1) * 1/4 + (float) ((mant >> 2) & 1) * 1/8 + (float) ((mant >> 1) & 1) * 1/16 + (float) ((mant) & 1) * 1/32;


	if (signbit){
		for (int i = exp; i < 0; ++i)
		{
			expcalc = expcalc / 2;
		}
		result =  mantcalc * expcalc * -1;
	}
	else{
		for (int i = exp; i > 0; --i)
		{
			expcalc = expcalc * 2;
		}
		result =  mantcalc * expcalc;
	}

	//if -/+ NaN or +/- inf or too large
	if (exp == 32 || mantcalc * expcalc > 0x7FFFFFFF || -(mantcalc * expcalc) > 0x7FFFFFFF){
		return 0x80000000;
	}

	return result;
}

/* Convert 32-bit single-precision floating point to 12-bit floating point */
fp12 float_fp12(float f)
{
	if (!f){
		return 0;
	}

	union origin {
		float f;
		int i;
	} origin;

	union result {
		float f;
		int i;
	} result;

	origin.f = f;


	int sign = 0;
	sign = (origin.i & 0x80000000) >> 31;
	if (sign == 1){
		f = -f;
	}

	result.i = (sign<<15) + (sign<<14) + (sign<<13) + (sign<<12) + (sign<<11);



	int exponent = (origin.i & 0x7F800000) >> 23;



	int mantissa = origin.i & 0x007FFF;

	if (exponent == 255){		//if nan or inf
		if(mantissa > 0){		//if nan
			result.i |= 0x07E1;
			return result.i;
		}
		else{					//if inf
			result.i |= 0x07E0;
			return result.i;
		}

	}

	exponent -= 127;

	int exp = 0;
	int tmp = mantissa;
	while (tmp > 1){
		tmp /= 2;
		exp++;
	}



	int fp12mantissa = mantissa >> (exp - 5);
	if (exp < 5){
		fp12mantissa = mantissa;
	}
	int R = (mantissa >> (exp - 6)) & 0x00000001;
	int L = (mantissa >> (exp - 5)) & 0x00000001;
	int X = mantissa << (32-(exp - 5));
	if ((X>0 && R == 1) || (L == 1 && R == 1)){
		fp12mantissa = 0;
		exponent++;
	}


	if (exponent > 31){
		result.i |= 0x07E0;
	}
	else if (exponent < -31){
		result.i &= 0xF81F;
	}
	else{
		result.i |= (exponent + 31) << 5;
	}
	/* TODO */
	return result.i;
}

/* Convert 12-bit floating point to 32-bit single-precision floating point */
float fp12_float(fp12 x)
{
	union result {
		float f;
		int i;
	} result;

	int signbit = 0;
	int exp = 0;
	int mant = 0;
	float frac = 0;
	signbit = x >> 15;
	exp = ((x & 0x07FF) >> 5) - 31;
	mant = x & 0x001F;



	float expcalc = 1;

	result.i = signbit<<31;


	//if -/+ NaN or +/- inf or too large
	if (exp == 32){
		result.i |= 0xFF<<23;
		result.i |= mant;
		return result.f;
	}

	result.i |= (exp + 127) << 23;



	float mantcalc = 1;
	if (exp == -31)
		mantcalc = 0;
	mantcalc += (float) (mant >> 4) * 1/2 + (float) ((mant >> 3) & 1) * 1/4 + (float) ((mant >> 2) & 1) * 1/8 + (float) ((mant >> 1) & 1) * 1/16 + (float) ((mant) & 1) * 1/32;


	if (signbit){
		for (int i = exp; i < 0; ++i)
		{
			expcalc = expcalc / 2;
		}
		frac =  mantcalc * expcalc * -1;
	}
	else{
		for (int i = exp; i > 0; --i)
		{
			expcalc = expcalc * 2;
		}
		frac =  mantcalc * expcalc;
	}

result.f = frac;




	return result.f;
}
