#pragma once 

//можнос делать через один общий абстрактный класс для коэффициентов
class BLackCoef
{
public:
    double Value;
    void UpdateFunction() {}
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

