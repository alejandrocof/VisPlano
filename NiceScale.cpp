#include "NiceScale.hpp"
/**
* Calculate and update values for tick spacing and nice
* minimum and maximum data points on the axis.
*/
void NiceScale::calculate() 
{
    range = niceNum(maxPoint - minPoint, false);
    tickSpacing = niceNum(range / (maxTicks - 1), true);
    niceMin = floor(minPoint / tickSpacing) * tickSpacing;
    niceMax = ceil(maxPoint / tickSpacing) * tickSpacing;
}

/**
* Returns a "nice" number approximately equal to range 
  Rounds the number if round = true Takes the ceiling if round = false.
*
* @param range the data range
* @param round whether to round the result
* @return a "nice" number to be used for the data range
*/
double NiceScale::niceNum(double range, bool round) 
{   double exponent; /** exponent of range */
    double fraction; /** fractional part of range */
    double niceFraction; /** nice, rounded fraction */

    exponent = floor(log10(range));
    fraction = range / pow(10.f, exponent);

    if (round) 
    {   if (fraction < 1.5)
            niceFraction = 1;
        else if (fraction < 3)
            niceFraction = 2;
        else if (fraction < 7)
            niceFraction = 5;
        else
            niceFraction = 10;
    } 
    else 
    {   if (fraction <= 1)
            niceFraction = 1;
        else if (fraction <= 2)
            niceFraction = 2;
        else if (fraction <= 5)
            niceFraction = 5;
        else
            niceFraction = 10;
    }

    return niceFraction * pow(10, exponent);
}

/**
* Sets the minimum and maximum data points for the axis.
*
* @param minPoint the minimum data point on the axis
* @param maxPoint the maximum data point on the axis
*/
void NiceScale::setMinMaxPoints(double minPoint, double maxPoint) 
{
    this->minPoint = minPoint;
    this->maxPoint = maxPoint;
    calculate();
}

/**
* Sets maximum number of tick marks we're comfortable with
*
* @param maxTicks the maximum number of tick marks for the axis
*/
void NiceScale::setMaxTicks(double maxTicks) 
{
    this->maxTicks = maxTicks;
    calculate();
}

// minimum number of decimals in tick labels
// use in sprintf statement:
// sprintf(buf, "%.*f", decimals(), tickValue);
int NiceScale::decimals(void)
{
    double logTickX = log10(tickSpacing);
    if(logTickX >= 0)
        return 0;
    return (int)(fabs(floor(logTickX)));
}

int NiceScale::N(void){
  return (this->niceMax-this->niceMin)/this->tickSpacing+1;
}
