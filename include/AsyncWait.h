/*
 * AsyncWait.h
 *
 *  Created on: 14 sept. 2020
 *      Author: jctejero
 */

#ifndef ASYNCWAIT_H_
#define ASYNCWAIT_H_


#if (ARDUINO >= 100)
#  include <Arduino.h>
#else
#  include <WProgram.h>
#endif

#include <limits.h>


using MilliSec = unsigned long;


class AsyncWait {
  public:
    AsyncWait() {
      reset();
    }

    void startWaiting(uint16_t *waitDuration) {
      this->waitDuration = waitDuration;
      restart();
    }

    bool isWaiting() {
      if (!isAsyncWaiting) {
        return false;
      }

      MilliSec delta = calculateMilliSecDelta(millis());
      if (delta >= ((*waitDuration)*1000)) {    //*1000 porque el dato esta en segundos
        // We have finished waiting for the given duration.
        reset();
        return false;
      }

      // Still waiting.
      return true;
    }

    void cancel() {
      reset();
    }

    void restart(){
    	waitStartMillisec 	= millis();
    	isAsyncWaiting 		= true;
    	Verified			= false;
    }

    bool isVerified(){
    	return Verified;
    }

    void setVerified(){
    	Verified = true;
    }

  private:
    bool		Verified;
    bool 		isAsyncWaiting;
    MilliSec 	waitStartMillisec;
    uint16_t 	*waitDuration;

    void reset() {
      isAsyncWaiting 	= false;
      waitStartMillisec = 0;
    }

    /**
     * Calculate the difference between currentMilliSec and waitStartMillisec
     * taking into account the event when 'millis()' wraps around.
     */
    MilliSec calculateMilliSecDelta(MilliSec currentMilliSec)
    {

        if (currentMilliSec < waitStartMillisec) {
            // Handle the wrap-around of millis().
            return ULONG_MAX - waitStartMillisec + currentMilliSec;
        } else {
            return currentMilliSec - waitStartMillisec;
        }
    }
};



#endif /* ASYNCWAIT_H_ */
