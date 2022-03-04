#pragma once 
#include <memory>

class Message;
//можнос делать через один общий абстрактный класс для коэффициентов
class BLackCoef
{
public:
    double Value;
    void Update(bool IsGood, std::shared_ptr<Message> CurMes) {}
    
    double TakeBand(double BandWant)
    {
        return BandWant;
    }
    BLackCoef(double InitBand = 0.0);
    ~BLackCoef() = default;
};

class StableCoef
{
public:
    double Value;
    void Update(double val) {}
    StableCoef() {}
    ~StableCoef() = default;
};

