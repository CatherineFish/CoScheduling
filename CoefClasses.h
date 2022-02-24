class Coefficien
{
public:
    double Value;
public:
    virtual void UpdateFunction();
};

class BLackCoef: public Coefficien
{
public:
    override UpdateFunction();
    BLackCoef(double InitBand);
    ~BLackCoef() = default;
}

class StableCoef: public Coefficien
{
    override UpdateFunction();
    StableCoef();
    ~StableCoef() = default;
}

