#include <iostream>

#include "CoefClasses.h"



BLackCoef:: BLackCoef(double InitBand)
{
	if (InitBand < 0)
	{
		std::cout << "Here" << std::endl;
	} else
	{
		Value = 0.0;
	}
}