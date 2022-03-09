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
        Value = std::max((NeedBand - (NewBand / CountInPeriod)) / HaveBand, 1.0);
    } else if (!IsPlanned)
    {
        //TODO теоритически можно понижать коэффициент с какого-то момента
    } else {
        Value = (NeedBand + NewBand * (CountNotPlanned / CountInPeriod)) / HaveBand;
    }
    return;
}

void BLackCoef:: Reload(bool IsBetter, double NewBand, int CountNotPlanned, int CountInPeriod) 
{
    if (IsBetter && Value > 1.0)
    {
        Value = std::max((NeedBand - NewBand * (CountNotPlanned / CountInPeriod)) / HaveBand, 1.0);
        
    } else if (IsBetter)
    {
    
    } else {
        Value = (NeedBand + (NewBand / CountInPeriod)) / HaveBand;
    }
    return;
}



void StableCoef:: Reload(bool IsBetter)
{
    if (IsBetter)
    {
        Value = CountInPeriod / (CountInPeriod - CountNotPlanned);
        NumOfPlanned = -1;    
    } else 
    {
        CountNotPlanned--;
        Value = CountInPeriod / (CountInPeriod - CountNotPlanned);  
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
        NumOfPlanned = CountNotPlanned; // TODO +-1
        Value = 0.0;
    }
    return; 
}