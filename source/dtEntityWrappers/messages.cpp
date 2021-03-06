/*
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

#include <dtEntityWrappers/messages.h>

#include <dtEntity/messagefactory.h>
#include <dtEntity/entity.h>
#include <dtEntity/stringid.h>


namespace dtEntityWrappers
{
   ///////////////////////////////////////////////////////////////////////////////////////////////////////
   void RegisterMessageTypes(dtEntity::MessageFactory& em)
   {    
      em.RegisterMessageType<ExecuteScriptMessage>(ExecuteScriptMessage::TYPE);
   }

   const dtEntity::MessageType ExecuteScriptMessage::TYPE(dtEntity::SID("ExecuteScriptMessage"));
   const dtEntity::StringId ExecuteScriptMessage::PathId(dtEntity::SID("Path"));
   const dtEntity::StringId ExecuteScriptMessage::IncludeOnceId(dtEntity::SID("IncludeOnce"));
  
}
