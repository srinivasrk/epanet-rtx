//
//  MovingAverage.cpp
//  epanet-rtx
//
//  Created by the EPANET-RTX Development Team
//  See README.md and license.txt for more information
//  

#include "MovingAverage.h"
#include <boost/foreach.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/circular_buffer.hpp>
#include <math.h>

#include <iostream>

using namespace RTX;
using namespace std;
using namespace boost::accumulators;


MovingAverage::MovingAverage() {
  _windowSize = 5;
}


#pragma mark - Added Methods

void MovingAverage::setWindowSize(int numberOfPoints) {
  this->invalidate();
  _windowSize = numberOfPoints;
}

int MovingAverage::windowSize() {
  return _windowSize;
}


#pragma mark - Delegate Overridden Methods



TimeSeries::PointCollection MovingAverage::filterPointsInRange(TimeRange range) {
  vector<Point> filteredPoints;
  
  TimeRange rangeToResample = range;
  if (this->willResample()) {
    // expand range
    rangeToResample.first = this->source()->pointBefore(range.first + 1).time;
    rangeToResample.second = this->source()->pointAfter(range.second - 1).time;
  }
  
  TimeRange queryRange = rangeToResample;
  queryRange.first  = queryRange.first  > 0 ? queryRange.first  : range.first;
  queryRange.second = queryRange.second > 0 ? queryRange.second : range.second;
  
  int margin = this->windowSize() / 2;
  
  // expand source lookup bounds, counting as we go and ignoring invalid points.
  for (int leftSeek = 0; leftSeek <= margin; ) {
    Point p = this->source()->pointBefore(queryRange.first);
    if (p.isValid && p.time != 0) {
      queryRange.first = p.time;
      ++leftSeek;
    }
    if (p.time == 0) {
      // end of valid points
      break; // out of for(leftSeek)
    }
  }
  for (int rightSeek = 0; rightSeek <= margin; ) {
    Point p = this->source()->pointAfter(queryRange.second);
    if (p.isValid && p.time != 0) {
      queryRange.second = p.time;
      ++rightSeek;
    }
    if (p.time == 0) {
      // end of valid points
      break; // out of for(rightSeek)
    }
  }
  
  // get the source's points, but
  // only retain valid points.
  vector<Point> sourcePoints;
  {
    // use our new righ/left bounds to get the source data we need.
    PointCollection sourceRaw = this->source()->points(queryRange);
    BOOST_FOREACH(Point p, sourceRaw.points) {
      if (p.isValid) {
        sourcePoints.push_back(p);
      }
    }
  }
  
  // set up some iterators to start scrubbing through the source data.
  typedef std::vector<Point>::const_iterator pVec_cIt;
  pVec_cIt vecBegin = sourcePoints.cbegin();
  pVec_cIt vecEnd = sourcePoints.cend();
  
  
  // simple approach: take each point, and search through the vector for where
  // to place the left/right cursors, then take the average.
  
  BOOST_FOREACH(const Point& sPoint, sourcePoints) {
    time_t now = sPoint.time;
    if (now < rangeToResample.first || now > rangeToResample.second) {
      continue; // out of bounds.
    }
    
    // initialize the cursors
    pVec_cIt seekCursor = sourcePoints.cbegin();
    
    // maybe the data doesn't support this particular time we want
    if (seekCursor->time > now) {
      continue; // go to the next time
    }
    
    // seek to this time value in the source data
    while (seekCursor != vecEnd && seekCursor->time < now ) {
      ++seekCursor;
    }
    // have we run off the end?
    if (seekCursor == vecEnd) {
      break; // no more work here.
    }
    
    // back-off to the left
    pVec_cIt leftCurs = seekCursor;
    for (int iLeft = 0; iLeft < margin; ) {
      --leftCurs;
      ++iLeft;
      // did we stumble upon the vector.begin?
      if (leftCurs == vecBegin) {
        break;
      }
    }
    
    // back-off to the right
    pVec_cIt rightCurs = seekCursor;
    for (int iRight = 0; iRight < margin; ) {
      ++rightCurs;
      // end?
      if (rightCurs == vecEnd) {
        --rightCurs; // shaky standard?
        break;
      }
      ++iRight;
    }
    
    
    // at this point, the left and right cursors should be located properly.
    // take an average between them.
    accumulator_set<double, stats<tag::mean> > meanAccumulator;
    accumulator_set<double, stats<tag::mean> > confidenceAccum;
    int nAccumulated = 0;
    Point meanPoint(now);
    
    seekCursor = leftCurs;
    while (seekCursor != rightCurs+1) {
      
      Point p = *seekCursor;
      meanAccumulator(p.value);
      confidenceAccum(p.confidence);
      meanPoint.addQualFlag(p.quality);
      ++nAccumulated;
      ++seekCursor;
    }
    
    // done with computing the average. Save the new value.
    meanPoint.value = mean(meanAccumulator);
    meanPoint.confidence = mean(confidenceAccum);
    meanPoint.addQualFlag(Point::averaged);
//    cout << "accum: " << nAccumulated << endl;
    
    filteredPoints.push_back(meanPoint);
  }
  
  
  PointCollection outData = PointCollection(filteredPoints, source()->units());
  
  
  bool dataOk = false;
  dataOk = outData.convertToUnits(this->units());
  if (dataOk && this->willResample()) {
    set<time_t> timeValues = this->timeValuesInRange(range);
    dataOk = outData.resample(timeValues);
  }
  
  if (dataOk) {
    return outData;
  }
  
  return PointCollection(vector<Point>(),this->units());
}

