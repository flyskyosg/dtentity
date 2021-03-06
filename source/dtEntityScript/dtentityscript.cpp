/* -*-c++-*-
* testEntity - testEntity(.h & .cpp) - Using 'The MIT License'
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* Martin Scheffler
*/

#include <dtEntity/core.h>
#include <dtEntity/dtentity_config.h>
#include <dtEntity/entity.h>
#include <dtEntityOSG/initosgviewer.h>
#include <dtEntity/mapcomponent.h>
#include <dtEntity/componentpluginmanager.h>
#include <dtEntity/systeminterface.h>
#include <dtEntityOSG/componentfactories.h>
#include <iostream>
#include <osgDB/FileUtils>
#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>

#if DTENTITY_PROFILING_ENABLED
#include <dtEntity/profile.h>
#endif

// include the plugins we need
USE_DTENTITYPLUGIN(dtEntityRocket)
USE_DTENTITYPLUGIN(dtEntityV8Plugin)

int main(int argc, char** argv)
{
    std::string script = "Scripts/autostart.js";
#if DTENTITY_PROFILING_ENABLED
    bool profiling_enabled = false;
#endif
    int curArg = 1;

    while (curArg < argc)
    {
       std::string curArgv = argv[curArg];
       if (!curArgv.empty())
       {
          if (curArgv == "--script")
          {
             ++curArg;
             if (curArg < argc)
             {
                script = argv[curArg];
             }
          }
#if DTENTITY_PROFILING_ENABLED
          else if (curArgv == "--enable-profiling")
          {
             profiling_enabled = true;
          }
#endif
        }
       ++curArg;
    }

   osg::ArgumentParser arguments(&argc,argv);   
   osgViewer::CompositeViewer viewer(arguments);

   dtEntity::EntityManager entityManager;
   
   if(!dtEntityOSG::InitOSGViewer(argc, argv, viewer, entityManager, true, true))
   {
      LOG_ERROR("Error setting up dtEntity!");
      return 0;
   }


   dtEntity::ComponentPluginManager& pm = dtEntity::ComponentPluginManager::GetInstance();
   pm.AddPlugin("plugins/", "dtEntityV8Plugin", true);
   
   dtEntity::StringId scriptId = dtEntity::SID("Script");
      
   if(!entityManager.HasEntitySystem(scriptId))
   {
      if(!pm.FactoryExists(scriptId))
      {
         LOG_ERROR("Cannot start scripting, script plugin not loaded!");
         return 1;
      }         
         
      bool success = pm.StartEntitySystem(entityManager, scriptId);
      if(!success)
      {
         LOG_ERROR("Cannot start scripting, script plugin not loaded!");
         return 1;
      }
   }

   dtEntity::Message* msg;
   bool success = dtEntity::MessageFactory::GetInstance().CreateMessage(dtEntity::SID("ExecuteScriptMessage"), msg);
   assert(success);
   msg->Get(dtEntity::SID("IncludeOnce"))->SetBool(true);
   dtEntity::Property* pathprop = msg->Get(dtEntity::SID("Path"));

   pathprop->SetString(script); 
   entityManager.EmitMessage(*msg);
   delete msg;

   dtEntity::SystemInterface* iface = dtEntity::GetSystemInterface();

#if DTENTITY_PROFILING_ENABLED
   if(profiling_enabled)
   {
      static dtEntity::StringId frameId = dtEntity::SID("Frame");
      static dtEntity::StringId frameAdvanceId = dtEntity::SID("Frame_Advance");
      static dtEntity::StringId frameEvTrId = dtEntity::SID("Frame_EventTraversal");
      static dtEntity::StringId frameUpTrId = dtEntity::SID("Frame_UpdateTraversal");
      static dtEntity::StringId frameRenderTrId = dtEntity::SID("Frame_RenderingTraversals");

      unsigned int framecount = 0;
      while (!viewer.done()) 
      {
         CProfileManager::Increment_Frame_Counter();
			CProfileManager::Start_Profile(frameId);
         
         CProfileManager::Start_Profile(frameAdvanceId);
         viewer.advance();
         CProfileManager::Stop_Profile();

         CProfileManager::Start_Profile(frameEvTrId);
         viewer.eventTraversal();
         CProfileManager::Stop_Profile();

         CProfileManager::Start_Profile(frameUpTrId);
         iface->EmitTickMessagesAndQueuedMessages();
         viewer.updateTraversal();
         CProfileManager::Stop_Profile();

         iface->EmitPostUpdateMessage();

         CProfileManager::Start_Profile(frameRenderTrId);
         viewer.renderingTraversals();
         CProfileManager::Stop_Profile();

         CProfileManager::Stop_Profile();
         if(++framecount > 999)
         {
            CProfileManager::dumpAll();
			   fflush(stdout);
            framecount = 0;
            CProfileManager::Reset();
         }
      }
   }
   else
#endif
   {
      while (!viewer.done())
      {
         viewer.advance(DBL_MAX);
         viewer.eventTraversal();
         iface->EmitTickMessagesAndQueuedMessages();
         viewer.updateTraversal();
         iface->EmitPostUpdateMessage();
         viewer.renderingTraversals();
      }
   }

   return 0;
}
