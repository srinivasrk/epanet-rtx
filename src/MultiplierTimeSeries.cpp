#include "MultiplierTimeSeries.h"

using namespace RTX;
using namespace std;



MultiplierTimeSeries::MultiplierTimeSeries() {
  _multiplierBasis = TimeSeries::_sp(new TimeSeries());
}

void MultiplierTimeSeries::setMultiplier(TimeSeries::_sp ts) {
  _multiplierBasis = ts;
  this->invalidate();
}
TimeSeries::_sp MultiplierTimeSeries::multiplier() {
  return _multiplierBasis;
}



Point MultiplierTimeSeries::filteredWithSourcePoint(Point sourcePoint) {
  if (!this->multiplier()) {
    return Point();
  }
  set<time_t> t;
  t.insert(sourcePoint.time);
  vector<Point> multiplierPoint = this->multiplier()->resampled(t).points;
  
  if (multiplierPoint.size() < 1) {
    return Point();
  }
  
  Point x = multiplierPoint.front();
  
  if (!x.isValid) {
    return Point();
  }
  
  Point multiplied = sourcePoint * x.value;
  Point outPoint = Point::convertPoint(multiplied, this->source()->units() * this->multiplier()->units(), this->units());
  
  return outPoint;
}

bool MultiplierTimeSeries::canSetSource(TimeSeries::_sp ts) {
  
  return true;
  
}

void MultiplierTimeSeries::didSetSource(TimeSeries::_sp ts) {
  if (!this->source() || !this->multiplier()) {
    return;
  }
  
  Units nativeDerivedUnits = this->source()->units() * this->multiplier()->units();
  if (!this->units().isSameDimensionAs(nativeDerivedUnits)) {
    this->setUnits(nativeDerivedUnits);
  }
  
}

bool MultiplierTimeSeries::canChangeToUnits(Units units) {
  if (!this->source() || !this->multiplier()) {
    return true; // if the inputs are not fully set, then accept any units.
  }
  else if (units.isSameDimensionAs(this->source()->units() * this->multiplier()->units())) {
    return true;
  }
  else {
    return false;
  }
}

