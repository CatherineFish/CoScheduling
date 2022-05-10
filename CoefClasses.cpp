#include <iostream>

#include "CoefClasses.h"



BLackCoef:: BLackCoef(double NeedBand_, double HaveBand_): NeedBand(NeedBand_), HaveBand(HaveBand_), NeedBandInit(NeedBand_)
{
	if (HaveBand - NeedBand < 0)
	{
		//std::cout << "Too much bandwidth need" << std::endl;
		Value = NeedBand / HaveBand;
	} else
	{
		Value = 1.0;
	}
}

void BLackCoef:: ReUpdate()
{
    Value = (NeedBandInit * 1.5) / HaveBand;
    return;
}


void BLackCoef:: Update(bool IsPlanned, double NewBand, int CountNotPlanned, int CountInPeriod) 
{
    if (!IsPlanned && Value > 1.0)
    {
        NeedBand -= NewBand / CountInPeriod;
        Value = std::max(NeedBand / HaveBand, 1.0);
        
    } else if (!IsPlanned)
    {
        NeedBand -= NewBand / CountInPeriod;
        //Eperiment теоритически можно понижать коэффициент с какого-то момента
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
    //std::cout << "REload COEF " << IsBetter << std::endl;
    if (IsBetter)
    {
        Value = CountInPeriod / (CountInPeriod - CountNotPlanned);
        NumOfPlanned = -1;    
    } else 
    {
        CountNotPlanned--;
        //std::cout << CountNotPlanned << " " << CountInPeriod << std::endl; 
        Value = CountInPeriod / (CountInPeriod - CountNotPlanned);  
    }
    //std::cout << "Res Val : " << Value << std::endl;
    return;
}

void StableCoef:: Update(bool IsPlanned) 
{
    //std::cout << "Update COEF " << IsPlanned << " val " << Value << std::endl;
    if (!IsPlanned && Value)
    {
        CountNotPlanned++;
        if (CountNotPlanned == CountInPeriod) {
            Value = 1.0; 	
        } else {
            Value = CountInPeriod / (CountInPeriod - CountNotPlanned); 
        }
        //std::cout << "Count " << CountInPeriod << std::endl;  
    } else {
        NumOfPlanned = CountNotPlanned;
        Value = 0.0;
    }
    //std::cout << "Res Val : " << Value << std::endl;
    return; 
}