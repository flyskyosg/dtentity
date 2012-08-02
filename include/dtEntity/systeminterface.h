#pragma once

/* -*-c++-*-
 * dtEntity Game and Simulation Engine
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Martin Scheffler
 */

#include <dtEntity/nodemasks.h>
#include <dtEntity/property.h>

namespace dtEntity
{     
   #if defined(_MSC_VER)
       typedef __int64 Timer_t;
   #else
       typedef unsigned long long Timer_t;
   #endif

   class SystemInterface
   {
   public:

      /**
       * Send tick messages to main message pump, also emit
       * all queued messages
       */
      virtual void EmitTickMessagesAndQueuedMessages() = 0;

      /**
       * Get simulation time delta since last tick in seconds
       */
      virtual float GetDeltaSimTime() const = 0;

      /**
       * Get real time delta since last tick in seconds
       */
      virtual float GetDeltaRealTime() const = 0;

      /**
       * Get scaling value from real time to simulation time
       */
      virtual float GetTimeScale() const = 0;

      /**
       * Set scaling value from real time to simulation time.
       * A value of 2 means simulation runs twice as fast as real time
       * Causes a TimeChangedMessage to be sent
       */
      virtual void SetTimeScale(float) = 0;

      /**
       * Get number of seconds since start of simulation
       */
      virtual double GetSimulationTime() const = 0;

      /**
       * Set number of seconds since start of simulation
       * Causes a TimeChangedMessage to be sent
       */
      virtual void SetSimulationTime(double) = 0;


      /**
       * Number of microseconds since 1.1.1970
       * Causes a TimeChangedMessage to be sent
       */
      virtual void SetSimulationClockTime(Timer_t) = 0;

      /**
       * Get number of microseconds since 1/1/1970
       */
      virtual Timer_t GetSimulationClockTime() const = 0;

      /**
       * Get current system time in number of microseconds since 1/1/1970
       */
      virtual Timer_t GetRealClockTime() = 0;

      /**
       * Holds intersection info
       */
      struct Intersection
      {
         Vec3d mPosition;
         Vec3f mNormal;
         EntityId mEntityId;
      };

      typedef std::vector<Intersection> Intersections;
      /**
       * Get intersections between start and end and add them to isects vector
       */
      virtual bool GetIntersections(const Vec3d& start, const Vec3d& end, 
         std::vector<Intersection>& isects, 
         unsigned int nodemask = NodeMasks::PICKABLE | NodeMasks::TERRAIN
         ) const = 0;

      /**
       * Get absolute path from a relative file path.
       * Returns empty string if file is not found
       */
      virtual std::string FindDataFile(const std::string& filename) = 0;

      /**
        * return true if a file exists
        */
      virtual bool FileExists(const std::string& filename) = 0;

   };
  
}
