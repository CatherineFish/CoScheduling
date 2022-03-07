#include <iostream>

#include "CoefClasses.h"



BLackCoef:: BLackCoef(double NeedBand_, double HaveBand_): NeedBand(NeedBand_), HaveBand(HaveBand_)
{
	if (HaveBand - NeedBand < 0)
	{
		std::cout << "Too much bandwidth need" << std::endl;
		Value = NeedBand / HaveBand;
	} else
	{
		Value = 1.0;
	}
}


void BLackCoef:: Update(bool IsPlanned, double NewBand, int CountNotPlanned, int CountInPeriod) 
{
    if (!IsPlanned && Value > 1.0)
    {
        Value = (NeedBand - (NewBand / CountInPeriod)) / HaveBand;
    } else if (!IsPlanned)
    {
        //TODO теоритически можно понижать коэффициент с какого-то момента
    } else {
        Value = (NeedBand + NewBand * (CountNotPlanned / CountInPeriod)) / HaveBand;
    }
    return;
}


void StableCoef:: Update(bool IsPlanned) 
{
    if (!IsPlanned && Value)
    {
        CountNotPlanned++;
        if (CountNotPlanned == CountInPeriod) {
        	return;
        }
        std::cout << "Count " << CountInPeriod << std::endl;
        Value = CountInPeriod / (CountInPeriod - CountNotPlanned);   
    } else {
        Value = 0.0;
    }
    return; 
}