/* -*-c++-*-
* dtEntity Game and Simulation Engine
*
* Copyright (c) 2013 Martin Scheffler
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software
* and associated documentation files (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies 
* or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
* INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
* PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
* FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*/

#include <dtEntityEditor/editorapplication.h>

#include <assert.h>
#include <dtEntity/componentpluginmanager.h>
#include <dtEntity/core.h>
#include <dtEntity/entity.h>
#include <dtEntity/entitymanager.h>
#include <dtEntity/init.h>
#include <dtEntity/mapcomponent.h>
#include <dtEntityOSG/resourcemanager.h>
#include <dtEntity/systemmessages.h>
#include <dtEntityEditor/editormainwindow.h>
#include <dtEntityEditor/motionmodel.h>
#include <dtEntityOSG/osginputinterface.h>
#include <dtEntityOSG/osgdebugdrawinterface.h>
#include <dtEntityOSG/osgsysteminterface.h>
#include <dtEntityOSG/osgwindowinterface.h>
#include <dtEntityOSG/componentfactories.h>
#include <dtEntityOSG/initosgviewer.h>
#include <dtEntityQtWidgets/messages.h>
#include <dtEntityQtWidgets/osggraphicswindowqt.h>
#include <dtEntityQtWidgets/osgadapterwidget.h>
#include <osgViewer/GraphicsWindow>
#include <osgViewer/View>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <iostream>
#include <QtCore/QDir>

namespace dtEntityEditor
{
    // include the plugins we need
    USE_DTENTITYPLUGIN(dtEntityV8Plugin)

   ////////////////////////////////////////////////////////////////////////////////
   EditorApplication::EditorApplication(int argc, char *argv[])
      : mMainWindow(NULL)
      , mEntityManager(new dtEntity::EntityManager())
      , mStartOfFrameTick(osg::Timer::instance()->tick())
      , mTimeScale(1)
      , mFileSystemWatcher(new QFileSystemWatcher())
   {
      dtEntity::SetSystemInterface(new dtEntityOSG::OSGSystemInterface(mEntityManager->GetMessagePump(), argc, (const char**) argv));
      dtEntity::SetWindowInterface(new dtEntityOSG::OSGWindowInterface(*mEntityManager));
      dtEntity::SetInputInterface(new dtEntityOSG::OSGInputInterface(mEntityManager->GetMessagePump()));

      dtEntity::LogManager::GetInstance().AddListener(new dtEntity::ConsoleLogHandler());

      mEntityManager->AddEntitySystem(*new MotionModelSystem(*mEntityManager));
      
      dtEntity::SetupDataPaths(argc,argv, false);

      osgDB::FilePathList oldpaths;
      for(osgDB::FilePathList::iterator i = osgDB::getDataFilePathList().begin(); i!= osgDB::getDataFilePathList().end(); ++i)
      {
         oldpaths.push_back(osgDB::convertFileNameToUnixStyle(*i));
      }
      osgDB::FilePathList newpaths;

      QSettings settings;
      QStringList qtpaths = settings.value("DataPaths", "ProjectAssets").toStringList();

      foreach(QString qtpath, qtpaths)
      {
         if(QFile::exists(qtpath))
         {
            newpaths.push_back(osgDB::convertFileNameToUnixStyle(qtpath.toStdString()));
         }
         else
         {
            LOG_ERROR("Project assets folder does not exist: " << qtpath.toStdString());            
         }      
      }

      for(osgDB::FilePathList::size_type i = 0; i < oldpaths.size(); ++i)
      {
         if(std::find(newpaths.begin(), newpaths.end(), oldpaths[i]) == newpaths.end() && 
            QFile::exists(oldpaths[i].c_str()))
         {
            newpaths.push_back(oldpaths[i]);
         }
      }

      osgDB::setDataFilePathList(newpaths);

      QStringList newpathsqt;
      for(osgDB::FilePathList::size_type i = 0; i < newpaths.size(); ++i)
      {
         newpathsqt.push_back(newpaths[i].c_str());
      }

      settings.setValue("DataPaths", newpathsqt);
      emit DataPathsChanged(newpathsqt);

      dtEntity::AddDefaultEntitySystemsAndFactories(argc, argv, *mEntityManager);
      dtEntityOSG::RegisterStandardFactories(dtEntity::ComponentPluginManager::GetInstance());

      // default plugin dir
      mPluginPaths.push_back("plugins");

      osg::ArgumentParser arguments(&argc,argv);
      mViewer = new osgViewer::Viewer(arguments);
      mViewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
      // don't exit on escape
      mViewer->setKeyEventSetsDone(0);

      static const char* winvar = "OSG_WINDOW=0 0 800 600";
      putenv(const_cast<char*>(winvar));

      dtEntity::MessageFunctor f(this, &EditorApplication::OnResourceLoaded);
      mEntityManager->RegisterForMessages(dtEntity::ResourceLoadedMessage::TYPE, f, "EditorApplication::OnResourceLoaded");
      connect(mFileSystemWatcher, SIGNAL(fileChanged(QString)), this, SLOT(OnFileChanged(QString)));

      dtEntity::MessageFunctor f2(this, &EditorApplication::OnCameraAdded);
      mEntityManager->RegisterForMessages(dtEntity::CameraAddedMessage::TYPE, f2, "EditorApplication::OnCameraAdded");
   }

   ////////////////////////////////////////////////////////////////////////////////
   EditorApplication::~EditorApplication()
   {
      delete mEntityManager;
      mEntityManager = NULL;
   }

   ////////////////////////////////////////////////////////////////////////////////
   dtEntity::EntityManager& EditorApplication::GetEntityManager() const
   {
      return *mEntityManager;
   }

   ////////////////////////////////////////////////////////////////////////////////
   void EditorApplication::SetMainWindow(EditorMainWindow* mw)
   {
      assert(mMainWindow == NULL);
      mMainWindow = mw;
      
      connect(mMainWindow, SIGNAL(Closed(bool)), this, SLOT(ShutDownGame(bool)));
      connect(mMainWindow, SIGNAL(ViewResized(const QSize&)), this, SLOT(ViewResized(const QSize&)));

      connect(this, SIGNAL(EditorSceneLoaded(const QString&)), mMainWindow, SLOT(OnComponentListChanged()));
   }

   ////////////////////////////////////////////////////////////////////////////////
   void EditorApplication::SetAdditionalPluginPath(const QString &path, bool bBeforeDefaultPath)
   {
      if(bBeforeDefaultPath)
         mPluginPaths.insert(mPluginPaths.begin(), path.toStdString());
      else
         mPluginPaths.push_back(path.toStdString());
   }

   ////////////////////////////////////////////////////////////////////////////////
   void EditorApplication::StartGame(const QString& sceneToLoad)
   {
      assert(mMainWindow != NULL);

      // give application system access to viewer
      dtEntityOSG::OSGSystemInterface* iface = static_cast<dtEntityOSG::OSGSystemInterface*>(dtEntity::GetSystemInterface());
      iface->SetViewer(mViewer);


      bool success = dtEntityOSG::DoScreenSetup(0, NULL, *mViewer, GetEntityManager());
      if(!success)
      {
         LOG_ERROR("Error setting up screens! exiting");
         return;
      }

      dtEntityOSG::SetupSceneGraph(*mViewer, GetEntityManager(), new osg::Group());
    
      osgViewer::ViewerBase::Views views;
      mViewer->getViews(views);
      for(osgViewer::ViewerBase::Views::iterator i = views.begin(); i != views.end(); ++i)
      {
         osgViewer::StatsHandler* stats = new osgViewer::StatsHandler();
         stats->setKeyEventTogglesOnScreenStats(osgGA::GUIEventAdapter::KEY_Insert);
         stats->setKeyEventPrintsOutStats(osgGA::GUIEventAdapter::KEY_Undo);
         (*i)->addEventHandler(stats);
      }

      dtEntity::SetDebugDrawInterface(new dtEntityOSG::OSGDebugDrawInterface(*mEntityManager));

      
      ////////////////////
      
      dtEntityQtWidgets::RegisterMessageTypes(dtEntity::MessageFactory::GetInstance());

      osgViewer::ViewerBase::Windows wins;
      mViewer->getWindows(wins);

      dtEntityQtWidgets::OSGGraphicsWindowQt* osgGraphWindow =
            dynamic_cast<dtEntityQtWidgets::OSGGraphicsWindowQt*>(wins.front());


      if(osgGraphWindow->thread() == thread())
      {
         mMainWindow->SetOSGWindow(osgGraphWindow);
      }
      else
      {
         QMetaObject::invokeMethod(mMainWindow, "SetOSGWindow",  Qt::BlockingQueuedConnection,
            Q_ARG(dtEntityQtWidgets::OSGGraphicsWindowQt*, osgGraphWindow));
      }

      try
      { 

         connect(this, SIGNAL(ErrorOccurred(const QString&)),
                 mMainWindow, SLOT(OnDisplayError(const QString&)));

         for(std::vector<std::string>::size_type i = 0; i < mPluginPaths.size(); ++i)
         {
            LOG_DEBUG("Looking for plugins in directory " + mPluginPaths[i]);
            // load and start all entity systems in plugins
            dtEntity::ComponentPluginManager::GetInstance().LoadPluginsInDir(mPluginPaths[i]);
         }
         // add new factories to list of known ones
         mMainWindow->OnComponentListChanged();


      }
      catch(const std::exception& e)
      {
         emit(ErrorOccurred(QString("Error starting application: %1").arg(e.what())));
         LOG_ERROR("Error starting application:" + std::string(e.what()));
      }
      catch(...)
      {
         emit(ErrorOccurred("Unknown error starting application"));
         LOG_ERROR("Unknown error starting application");
      }

      InitializeScripting();

      if(sceneToLoad != "")
      {
         LoadScene(sceneToLoad);
      }

      dtEntity::StartSystemMessage msg;
      mEntityManager->EmitMessage(msg);

      StepGame();

   }

   //////////////////////////////////////////////////////////////////////////
   QStringList EditorApplication::GetDataPaths() const
   {
      QSettings settings;
      return settings.value("DataPaths", "ProjectAssets").toStringList();
   }

   //////////////////////////////////////////////////////////////////////////
   void EditorApplication::SetDataPaths(const QStringList& l)
   {
      QSettings settings;
      settings.setValue("DataPaths", l);

      osgDB::FilePathList osgpaths;
      foreach(QString qtpath, l)
      {
         if(QFile::exists(qtpath))
         {
           osgpaths.push_back(osgDB::convertFileNameToUnixStyle(qtpath.toStdString()));
         }
      }
      osgDB::setDataFilePathList(osgpaths);
   }

   //////////////////////////////////////////////////////////////////////////
   void EditorApplication::AddPluginLibrary(std::string fileName)
   {
      // get Map system
      
      // load plugin, set it to be saved to scene file. Also start all entity systems in it
      std::set<dtEntity::ComponentType> newTypes = 
         dtEntity::ComponentPluginManager::GetInstance().AddPlugin(fileName, true);
      // notify GUI that new types are now available
      mMainWindow->OnComponentListChanged();
   
   }

   ////////////////////////////////////////////////////////////////////////////////
   void EditorApplication::StepGame()
   {
      dtEntity::SystemInterface* iface = dtEntity::GetSystemInterface();

      while(!mViewer->done())
      {

         //LOG_ALWAYS("STEP!" << appsys->GetSimulationTime());
         mViewer->advance(DBL_MAX);

         mViewer->eventTraversal();

         iface->EmitTickMessagesAndQueuedMessages();

         mViewer->updateTraversal();
         iface->EmitPostUpdateMessage();
         mViewer->renderingTraversals();

         QCoreApplication::processEvents();
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void EditorApplication::ShutDownGame(bool)
   {
      mViewer->setDone(true);

      // delete entity manager now before EditorApplication object is moved to main thread.
      mEntityManager = NULL;
      QThread::currentThread()->quit();
   }

   ////////////////////////////////////////////////////////////////////////////////
   void EditorApplication::ViewResized(const QSize& size)
   {
      osgViewer::ViewerBase::Windows wins;
      mViewer->getWindows(wins);
      wins.front()->resized(0, 0, size.width(), size.height());
      wins.front()->getEventQueue()->windowResize(0, 0, size.width(), size.height());

      osgViewer::ViewerBase::Cameras cams;
      mViewer->getCameras(cams);
      cams.front()->setViewport(new osg::Viewport(0, 0, size.width(), size.height()));

      double vfov, aspectRatio, nearClip, farClip;
      cams.front()->getProjectionMatrixAsPerspective(vfov, aspectRatio, nearClip, farClip);
      cams.front()->setProjectionMatrixAsPerspective(vfov, static_cast<double>(size.width()) / static_cast<double>(size.height()), nearClip, farClip);
   }

   ////////////////////////////////////////////////////////////////////////////////
   void EditorApplication::OnCameraAdded(const dtEntity::Message& m)
   {
      const dtEntity::CameraAddedMessage& msg = static_cast<const dtEntity::CameraAddedMessage&>(m);
      unsigned int camid = msg.GetAboutEntityId();

      dtEntityOSG::CameraComponent* camcomp;
      if(mEntityManager->GetComponent(camid, camcomp))
      {
         if(!mEntityManager->HasComponent(camid, MotionModelSystem::TYPE))
         {
            MotionModelComponent* motioncomp;
            mEntityManager->CreateComponent(camid, motioncomp);
            motioncomp->Finished();
         }
      }
      dtEntity::EntitySystem* soundsys = mEntityManager->GetEntitySystem(dtEntity::SID("Sound"));
      if(soundsys)
      {
          soundsys->SetUInt(dtEntity::SID("ListenerEntity"), camid);
          soundsys->Finished();
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void EditorApplication::OnResourceLoaded(const dtEntity::Message& m)
   {
      const dtEntity::ResourceLoadedMessage& msg = static_cast<const dtEntity::ResourceLoadedMessage&>(m);
      QString path = msg.GetPath().c_str();
      if(!mFileSystemWatcher->files().contains(path))
      {
         mFileSystemWatcher->addPath(path);
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void EditorApplication::OnFileChanged(const QString& path)
   {
      
      if(mFileSystemWatcher->files().contains(path))
      {
         mFileSystemWatcher->removePath(path);
         mFileSystemWatcher->addPath(path);
      }

      dtEntityOSG::ResourceManager::GetInstance().TriggerReload(path.toStdString(), *mEntityManager);
   }

   ////////////////////////////////////////////////////////////////////////////////
   void EditorApplication::NewScene()
   {
      dtEntity::MapSystem* mapSystem;
      GetEntityManager().GetES(mapSystem);
      mapSystem->UnloadScene();
   }

   ////////////////////////////////////////////////////////////////////////////////
   void EditorApplication::LoadScene(const QString& path)
   {  
      
      dtEntity::MapSystem* mapSystem;
      GetEntityManager().GetEntitySystem(dtEntity::MapComponent::TYPE, mapSystem);

      mapSystem->UnloadScene();

      mapSystem->LoadScene(path.toStdString());
      
      emit EditorSceneLoaded(path);

   }


   ////////////////////////////////////////////////////////////////////////////////
   void EditorApplication::SaveScene(const QString& path)
   {
      dtEntity::MapSystem* mapSystem;
      GetEntityManager().GetEntitySystem(dtEntity::MapComponent::TYPE, mapSystem);

      std::ostringstream os;
      bool success = mapSystem->SaveScene(path.toStdString(), true);
      if(!success)
      {
         ErrorOccurred(tr("Cannot save scene, please check file permissions!"));
      }
   }


   ////////////////////////////////////////////////////////////////////////////////
   void EditorApplication::InitializeScripting()
   {

      dtEntity::StringId scriptId = dtEntity::SID("Script");

      if(!GetEntityManager().HasEntitySystem(scriptId))
      {
         if(!dtEntity::ComponentPluginManager::GetInstance().FactoryExists(scriptId))
         {
            LOG_ERROR("Cannot start scripting, script plugin not loaded!");
            return;
         }         
            
         bool success = dtEntity::ComponentPluginManager::GetInstance().StartEntitySystem(GetEntityManager(), scriptId);
         if(!success)
         {
            LOG_ERROR("Cannot start scripting, script plugin not loaded!");
            return;
         }
      }

      dtEntity::Message* msg;
      bool success = dtEntity::MessageFactory::GetInstance().CreateMessage(dtEntity::SID("ExecuteScriptMessage"), msg);
      assert(success);
      msg->Get(dtEntity::SID("IncludeOnce"))->SetBool(true);
      dtEntity::Property* pathprop = msg->Get(dtEntity::SID("Path"));

      pathprop->SetString("Scripts/editorautostart.js");
      GetEntityManager().EmitMessage(*msg);

      delete msg;
   }
}
