#pragma once 
#include <memory>

class Message;

class BLackCoef
{
public:
    double Value;
    double NeedBand, HaveBand, NeedBandInit;
    void Update(bool IsPlanned, double NewBand, int CountNotPlanned, int CountInPeriod);
    BLackCoef(double NeedBand_ = 0.0, double HaveBand_ = 0.0);
    ~BLackCoef() = default;
    void Reload(bool IsBetter, 
                double NewBand, 
                int CountNotPlanned, 
                int CountInPeriod);
    void ReUpdate(); 

};

class StableCoef
{
public:
    int CountInPeriod;
    int NumOfPlanned = -1;
    int CountNotPlanned = 0;
    double Value = 1.0;
    void Update(bool IsPlanned);
    StableCoef(int CountInPeriod_ = 0): CountInPeriod(CountInPeriod_) {}
    ~StableCoef() = default;
    void Reload(bool IsBetter);
};

