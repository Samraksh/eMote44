
#define PRECISION 1
#define SUCCESS 1
#define FAIL 0

#include "fixedPoint.h"

#define _EMBEDDED_ 1

class testMath
{
	
public:
	int multiplier;
	int increment;
	int modulus;
	int seedValue;
	bool prngInitialized;
	unsigned int randomValue;

	// Initializes the prng gives the ability to initialize multiplier, increment, modulus and seedValue
	void prng_init(int Multiplier, int Increment, int Modulus, int SeedValue)
	{
		prngInitialized = true;
		multiplier = Multiplier;
		increment = Increment;
		modulus = Modulus;
		seedValue = SeedValue;
	}

	// Uses preinitialized multiplier, increment and modulus values
	void prng_init(int SeedValue)
	{
		prngInitialized = true;
		multiplier = 1140671485; // 2 ^ 24
		increment = 12820163;
		modulus = 16777216;
		seedValue = SeedValue;
		randomValue = (double) SeedValue;
	}

	unsigned int prng()
	{
		//float randomValuecdf = 0.0;

		double intermedResult = 0;
		double randomValueFP = 0;
		double sqrrootValue = 0;
		double resultFP = 0;
		int paretoRandomNumber = 0;
		int result = 0;


		double sqrrootValueFP = 0;

		if(!prngInitialized)
		{
#ifdef _EMBEDDED_
			hal_printf("\nRandom Number Generator Not Initialized");
#else
			printf("\nRandom Number Generator Not Initialized");
#endif
			return -1;
		}
		
		//printf("\n%ld",multiplier * randomValue);

		intermedResult = (1140671485 * randomValue + increment) % (modulus);

		randomValue = (int) intermedResult;

		// Divide by 2^24 and multiply by 2^24 cancel out
		//randomValueFP = randomValue;

		//sqrrootValue = sqrroot((int) (((1 << 24) - (int) randomValueFP) >> 24));
		//sqrrootValue = sqrroot((1 << 24) - randomValue);

		//printf("SQUAREROOT = %lf", sqrrootValue);

		// Converting to fixed point arithmetic beyond this point. 

		// Dividing by 2^24 to make the number less than 1 and then need to multiply by 2^24 to make it fixed point, retaining number as is
		// as the shifts cancel out
		//sqrrootValueFP = sqrrootValue;

		//resultFP = (((500 << 24) << 24) / (int) sqrrootValueFP));

		//resultFP = (((1 << 24) / (int) sqrrootValueFP) * 500);

		//printf("Result = %d", (int) resultFP);

		//randomValue = (long) (( multiplier * randomValue + increment) % (modulus));

		/* Need to figure out overflow issue, using 11 for now, seems to generate a good sequence*/
		/* Using Fixed point arithmetic, this number is assumed as 8.24 with 8 whole parts and 24 decimal parts*/
		//randomValue = (( 11 * randomValue + increment) % (modulus));

		//intermedResult = sqrroot(16777216 - randomValue);
		//randomValue = 16777216 - randomValue;

		//intermedResult = sqrroot(randomValue);

		//printf("%lf",intermedResult);
		//printf("###############%d",sizeof(intermedResult));
		//randomValue = 500 * 1/(sqrroot(1 - randomValue));
	//	if(randomValue < 0)
	//		return prng();
//n		randomValuecdf = pow((float)(100 / randomValue),2);
		//randomValue = 1 / randomValuecdf;
		return (int) randomValue;

	}



	UINT16 pareto_prng()
	{
		UINT32 sqrrootValueFP = 0;
		UINT32 resultFP = 0;

		unsigned int randomValue = prng();

		//sqrrootValueFP = (sqrroot_fp((1 << 24) - randomValue))/ (1 << 8);
		//resultFP = (((1 << 24) / (int) sqrrootValueFP) * 500);
#ifdef _EMBEDDED_
#ifdef _DEBUG_ON_
		hal_printf("Input Value %d\n",(1 << 24) - randomValue);
#endif
#else
		printf("Input Value %d\n",(1 << 24) - randomValue);
#endif
		unsigned int squareRoot = (sqrroot_fp((1 << 24) - randomValue) / (1 << 8));
		//sqrrootValueFP = sqrroot((1 << 24) - randomValue);
		resultFP = (((1 << 24) / (int) squareRoot) * 500);		

		return resultFP % (1 << 16);
	}

	int wienbull_prng()
	{
		unsigned int x = 0;

		unsigned int diff = 0;

		// Using lambda = 1 and k = 1 in the distribution

		diff = (1 << 24) - prng();

		//printf("Difference %d\n",diff);

		x = ln((1 << 24) - prng());

		//printf("X %d\n",x);

		return 1;

	}

	// computes the natural when numbers lie between 0 and 2
	float ln(float x)
	{
		if( x > 2)
			x = 1 / x;

		int n = 1;
		float lnx = 0;
		float oldlnx = 0;
		while(true)
		{
			lnx += pow((float) -1,n + 1) * pow((x - 1),n) / n;
			n = n+ 1;
			if(oldlnx > lnx)
			{
				if(oldlnx - lnx < 0.00001)
					break;
				else
				{
					oldlnx = lnx;
					continue;
				}
			}
			else
			{
				if(lnx - oldlnx < 0.00001)
					break;
				else
				{
					oldlnx = lnx;
					continue;
				}
			}
		}
		if(lnx < 0)
			return (-1) * lnx;
		else
			return lnx;
	}
	
	// Generates the sqrroot using the newtons law
	static float sqrroot(int number)
	{
		float initial = 1;
		float currValue = initial;
		float newValue = 0;

		while(true)
		{
			newValue = currValue - ((currValue * currValue) - number)/(2 * currValue);
			if(newValue > currValue)
			{
				if((newValue - currValue) < PRECISION)
					return newValue;
				else
					currValue = newValue;
			}
			else if(currValue > newValue)
			{
				if((currValue - newValue) < PRECISION)
					return currValue;
				else
					currValue = newValue;
			}
		}
	}

	static fixed sqrroot_fp(int number)
	{
		// Making an initial guess of half the number of bits
		//unsigned int initial = number >> 16;

#ifdef _EMBEDDED_
		UINT32 initial = 1;
#else
		unsigned int initial = 1;
#endif

//		initial >>= 16;
#ifdef _EMBEDDED_
		UINT32 currValue = initial;
#else
		unsigned int currValue = initial;
#endif

#ifdef _EMBEDDED_
		UINT32 newValue = 0;
#else
		unsigned int newValue = 0;
#endif

		// Converting to fixed point arithmetic beyond this point
		//unsigned int currValueFP = (1 << 24) * currValue;
		//unsigned int newValueFP = 0;
		fixed numberFP = (fixed) (1 << 8) * number;
		//unsigned int intermedResult = 0;

		fixed root;
		fixed next;
		fixed diff;
		fixed divRes;

		if (numberFP < int2Fixed(1))
		{
	        return 0;
		}

		next = numberFP >> 2;
		//next = (1 << 24);

		do
		{
			root = next;
			
			//divRes = fpDiv(numberFP,next);

			//if(divRes == 0)
				//printf("Crashing here\n");

			next = (next + fpDiv(numberFP, next)) >> 1;
			
			
			if((root - next) > 0)
				diff = root - next;
			else
				diff = next - root;
			
				
		} while (diff > 1000);


		return root;
		

		//printf("SIZE OF %d\n",sizeof(test));

		/*

		while(true)
		{
			//currValueFP = (1 << 24) * currValue;
			//newValueFP = 0;
			//numberFP = (double) (1 << 24) * number;

			
			//newValue = currValue - ((currValue * currValue) - number)/(2 * currValue);
			// newValue = currValue - (currValue / 2) - number / (2 * currValue);

			//intermedResult = (((currValueFP * currValueFP) / ( 1 << 24)) - numberFP);
			//newValueFP = currValueFP - (((currValueFP * currValueFP) / ( 1 << 24)) - numberFP)/(2 * currValueFP) * (1 << 24);
			//newValueFP = currValueFP - (currValueFP / 2) - (numberFP * ( 1 << 24)/ (2 * currValueFP)) ;

			//newValue = newValueFP / ( 1 << 24);
			//currValue = currValueFP / ( 1 << 24);
			//printf("%lld",newValueFP - currValueFP);
			if(newValueFP > currValueFP)
			{
				if((newValueFP - currValueFP) < (1 << 24))
					return newValueFP;
				else
					currValueFP = newValueFP;
			}
			else if(currValueFP > newValueFP)
			{
				if((currValueFP - newValueFP) < (1 << 24))
					return currValueFP;
				else
					currValueFP = newValueFP;
			}
		}
		*/

	}
};


