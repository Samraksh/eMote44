#ifndef _FIXED_POINT_H_
#define _FIXED_POINT_H_

#define _EMBEDDED_ 1

# ifdef _EMBEDDED_
	typedef UINT32 fixed;
#else
	typedef unsigned int fixed;
#endif

#define FIX_SHIFT 8

inline fixed int2Fixed(int d)
{
	return d << FIX_SHIFT;
}

inline unsigned int fixed2Int(fixed d)
{
	return d >> FIX_SHIFT;
}


inline fixed fpDiv(fixed a, fixed b)
{
	if( a < b)
	{
//		printf("Check Point 1\n");
		return (a * (1 << 8) / b);
	}
	else
	{
//		printf("Check Point 2\n");
		return (a  / b * (1 << 8));
	}
}


/*
fixed fpDiv(fixed d1, fixed d2);
#pragma aux fpDiv = \
    "xor    edx, edx" \
    "shld   edx, eax, 14" \
    "sal    eax,14" \
    "idiv   ebx" \
    parm [eax] [ebx]\
    modify [edx] \
    value [eax];
*/

#endif
