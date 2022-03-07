#pragma once 
#include <memory>

class Message;
//можнос делать через один общий абстрактный класс для коэффициентов
class BLackCoef
{
public:
    double Value;
    double NeedBand, HaveBand;
    void Update(bool IsPlanned, double NewBand, int CountNotPlanned, int CountInPeriod);
    BLackCoef(double NeedBand_ = 0.0, double HaveBand_ = 0.0);
    ~BLackCoef() = default;
};

class StableCoef
{
public:
    int CountInPeriod;
    int CountNotPlanned = 0;
    double Value = 1.0;
    void Update(bool IsPlanned);
    StableCoef(int CountInPeriod_ = 0): CountInPeriod(CountInPeriod_) {}
    ~StableCoef() = default;
        
};

