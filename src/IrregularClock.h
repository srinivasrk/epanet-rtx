//
//  IrregularClock.h
//  epanet-rtx
//
//  Created by the EPANET-RTX Development Team
//  See README.md and license.txt for more information
//  

#ifndef epanet_rtx_IrregularClock_h
#define epanet_rtx_IrregularClock_h

#include "Clock.h"
#include "PointRecord.h"

namespace RTX {
  
  class IrregularClock : public Clock {
  public:
    RTX_SHARED_POINTER(IrregularClock);
    IrregularClock(PointRecord::_sp pointRecord, std::string name);
    virtual ~IrregularClock();
    
    virtual bool isCompatibleWith(Clock::_sp clock);
    virtual bool isValid(time_t time);
    virtual time_t timeAfter(time_t time);
    virtual time_t timeBefore(time_t time);
    virtual std::set< time_t > timeValuesInRange(time_t start, time_t end);
    virtual std::ostream& toStream(std::ostream &stream);
    
  private:
    PointRecord::_sp _pointRecord;
    std::string _name;
  };
  
}

#endif
