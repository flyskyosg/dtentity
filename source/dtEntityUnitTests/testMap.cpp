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


#include <UnitTest++.h>
#include <dtEntity/init.h>
#include <dtEntity/core.h>
#include <dtEntity/mapcomponent.h>
#include <dtEntity/spawner.h>
#include <dtEntity/entitymanager.h> 
#include <dtEntity/dynamicscomponent.h>
#include <dtEntity/systeminterface.h>
#include <osgDB/FileUtils>

using namespace UnitTest;
using namespace dtEntity;

class MySystemInterface : public dtEntity::SystemInterface
{
public:
    virtual ~MySystemInterface() {}
    virtual void EmitTickMessagesAndQueuedMessages() {}
    virtual void EmitPostUpdateMessage() {}
    virtual float GetDeltaSimTime() const { return 0; }
    virtual float GetDeltaRealTime() const { return 0; }
    virtual float GetTimeScale() const { return 0; }
    virtual void SetTimeScale(float) {}
    virtual double GetSimulationTime() const { return 0; }
    virtual void SetSimulationTime(double) {}
    virtual void SetSimulationClockTime(Timer_t) {}
    virtual Timer_t GetSimulationClockTime() const { return 0; }
    virtual Timer_t GetRealClockTime() { return 0; }
    virtual std::string FindDataFile(const std::string& filename) { return osgDB::findDataFile(filename); }
    virtual bool FileExists(const std::string& filename) { return osgDB::fileExists(filename); }
    virtual int GetArgC() { return 0; }
    virtual const char** GetArgV() { return NULL; }

};

struct MapFixture
{
   MapFixture()
   {
      dtEntity::SetSystemInterface(new MySystemInterface());
      SetupDataPaths(0, NULL, true);
      dtEntity::AddDefaultEntitySystemsAndFactories(0, NULL, mEntityManager);
      mEntityManager.GetEntitySystem(MapComponent::TYPE, mMapSystem);
   }

   ~MapFixture()
   {
      ComponentPluginManager::DestroyInstance();
      delete dtEntity::GetSystemInterface();
   }

   EntityManager mEntityManager;
   MapSystem* mMapSystem;
};

TEST(IsBaseAssetsEnvVariableSet)
{
   CHECK(getenv("DTENTITY_BASEASSETS") != NULL);
}

TEST_FIXTURE(MapFixture, SpawnerCanBeLoaded)
{
   bool found = mMapSystem->LoadMap("TestData/testmap.dtemap");
   CHECK(found);
   dtEntity::Spawner* spawner;
   bool spawnerfound = mMapSystem->GetSpawner("TestSpawner", spawner);
   CHECK(spawnerfound);
}


TEST_FIXTURE(MapFixture, SpawnerHeaderData)
{
   bool found = mMapSystem->LoadMap("TestData/testmap.dtemap");
   CHECK(found);
   dtEntity::Spawner* spawner;
   found = mMapSystem->GetSpawner("TestSpawner", spawner);
   CHECK(found);
   if(found)
   {
      CHECK_EQUAL(spawner->GetAddToSpawnerStore(), true);
      CHECK_EQUAL(spawner->GetIconPath(), "TestIconPath");
      CHECK_EQUAL(spawner->GetGUICategory(), "TestGuiCategory");
      CHECK_EQUAL(spawner->GetName(), "TestSpawner");
   }
}

TEST_FIXTURE(MapFixture, SpawnerHasProperties)
{
   mMapSystem->LoadMap("TestData/testmap.dtemap");
   dtEntity::Spawner* spawner;
   bool found = mMapSystem->GetSpawner("TestSpawner", spawner);
   CHECK(found);
   if(found)
   {
      CHECK(spawner->HasComponent(dtEntity::SID("TestComponent")));
      CHECK(!spawner->HasComponent(dtEntity::SID("NoTestComponent")));
   }
}

bool Has(const dtEntity::Spawner::ComponentProperties& props, const std::string& key)
{
   return props.find(dtEntity::SID(key)) != props.end();
}

const dtEntity::Property* GetProp(const dtEntity::GroupProperty& props, const std::string& key)
{
   dtEntity::PropertyGroup propmap = props.Get();
   if(propmap.find(dtEntity::SID(key)) == propmap.end())
   {
      return NULL;
   }
   return propmap[dtEntity::SID(key)];
}

bool Equals(const dtEntity::GroupProperty& props, const std::string& key, const std::string& value)
{
   const dtEntity::Property* prop = GetProp(props, key);
   if(prop == NULL)
   {
      return false;
   }
   std::string str = prop->StringValue();
   return str == value;
}

TEST_FIXTURE(MapFixture, SpawnerPropertyValuesCorrect)
{
   mMapSystem->LoadMap("TestData/testmap.dtemap");
   dtEntity::Spawner* spawner;
   bool found = mMapSystem->GetSpawner("TestSpawner", spawner);

   CHECK(found);
   if(!found) return;

   dtEntity::Spawner::ComponentProperties props;
   spawner->GetAllComponentProperties(props);
   CHECK(Has(props, "TestComponent"));

   dtEntity::GroupProperty compprops =
      spawner->GetComponentValues(dtEntity::SID("TestComponent"));

   CHECK(Equals(compprops, "StringProperty", "StringValue"));
   CHECK(Equals(compprops, "BoolTrue", "true"));
   CHECK(Equals(compprops, "BoolFalse", "false"));
   CHECK(Equals(compprops, "DoubleProperty", "12345.6789000000"));
   CHECK(Equals(compprops, "IntProperty", "-12345"));
   CHECK(Equals(compprops, "UIntProperty", "12345"));
   
   CHECK(Equals(compprops, "MatrixProperty", "1.0000000000 2.0000000000 3.0000000000 4.0000000000 5.0000000000 6.0000000000 7.0000000000 8.0000000000 9.0000000000 10.0000000000 11.0000000000 12.0000000000 13.0000000000 14.0000000000 15.0000000000 16.0000000000"));

   CHECK(GetProp(compprops, "FloatProperty") != NULL);
   CHECK_EQUAL(GetProp(compprops, "FloatProperty")->StringValue().substr(0, 8), "12345.67");

   CHECK(Equals(compprops, "Vec2Property", "1.0000000000 2.0000000000"));
   CHECK(Equals(compprops, "Vec3Property", "1.0000000000 2.0000000000 3.0000000000"));
   CHECK(Equals(compprops, "Vec4Property", "1.0000000000 2.0000000000 3.0000000000 4.0000000000"));
   CHECK(Equals(compprops, "QuatProperty", "1.0000000000 2.0000000000 3.0000000000 4.0000000000"));

   CHECK(GetProp(compprops, "ArrayProperty") != NULL);
   CHECK(GetProp(compprops, "GroupProperty") != NULL);

   const Property* arrayprop = GetProp(compprops, "ArrayProperty");
   dtEntity::PropertyArray arr = arrayprop->ArrayValue();
   CHECK_EQUAL((unsigned int)2, arr.size());
   CHECK_EQUAL(arr[0]->StringValue(), "ArrayValOne");
   CHECK_EQUAL(arr[1]->StringValue(), "ArrayValTwo");

   const Property* grpprop = GetProp(compprops, "GroupProperty");
   dtEntity::PropertyGroup grp = grpprop->GroupValue();
   CHECK_EQUAL((unsigned int)2, grp.size());
   CHECK(grp.find(dtEntity::SID("GroupNameOne")) != grp.end());
   CHECK(grp.find(dtEntity::SID("GroupNameTwo")) != grp.end());
   CHECK_EQUAL(grp[dtEntity::SID("GroupNameOne")]->StringValue(), "GroupValOne");
   CHECK_EQUAL(grp[dtEntity::SID("GroupNameTwo")]->StringValue(), "GroupValTwo");
}


TEST_FIXTURE(MapFixture, SaveMapTest)
{
   std::string mapname = "TestData/testmap_generated.dtemap";

   CHECK(getenv("DTENTITY_BASEASSETS") != NULL);   
   std::string projectassets = getenv("DTENTITY_BASEASSETS");
   std::string targetpath = projectassets + std::string("/") + mapname;

   {  
      
      mMapSystem->AddEmptyMap(projectassets, mapname);
      dtEntity::Entity* entity;
      mEntityManager.CreateEntity(entity);

      if(!mEntityManager.HasEntitySystem(dtEntity::SID("PositionAttitudeTransform")))
      {
         mEntityManager.AddEntitySystem(*new dtEntity::DynamicsSystem(mEntityManager));
      }
      dtEntity::DynamicsComponent* dyncomp;
      entity->CreateComponent(dyncomp);
      dyncomp->SetVelocity(osg::Vec3(1,2,3));
      
      dtEntity::MapComponent* mapcomp;
      entity->CreateComponent(mapcomp);
      mapcomp->SetMapName(mapname);
      mapcomp->SetUniqueId("TestEntityId");
      mapcomp->SetEntityName("TestEntityName");
      mapcomp->Finished();
      mMapSystem->AddToScene(entity->GetId());
      mMapSystem->SaveMapAs(mapname, targetpath);

      mMapSystem->RemoveFromScene(entity->GetId());
      mEntityManager.KillEntity(entity->GetId());

      mMapSystem->UnloadMap(mapname);
   }

   {
      mMapSystem->LoadMap(mapname);
      dtEntity::Entity* entity;
      bool canFindEntity = mMapSystem->GetEntityByUniqueId("TestEntityId", entity);
      CHECK(canFindEntity);
      if(!canFindEntity) return;

      dtEntity::DynamicsComponent* dyncomp;
      entity->GetComponent(dyncomp);
      osg::Vec3 vel = dyncomp->GetVelocity();
      CHECK_CLOSE(vel[0], 1, 0.1);
      CHECK_CLOSE(vel[1], 2, 0.1);
      CHECK_CLOSE(vel[2], 3, 0.1);    
     
   }

   std::remove(targetpath.c_str());
}

TEST_FIXTURE(MapFixture, SaveSpawner)
{
   std::string mapname = "TestData/testmap_generated.dtemap";
   CHECK(getenv("DTENTITY_BASEASSETS") != NULL);   
   std::string projectassets = getenv("DTENTITY_BASEASSETS");
   std::string targetpath = projectassets + std::string("/") + mapname;
      
   {
      mMapSystem->AddEmptyMap(projectassets, mapname);

      dtEntity::Spawner* spawner1 = new dtEntity::Spawner("TestSpawner1", mapname);
      spawner1->SetAddToSpawnerStore(true);
      spawner1->SetGUICategory("TestGuiCategory");
      spawner1->SetIconPath("TestIconPath");

      GroupProperty props;
      props.Add(dtEntity::SID("StringProp1"), new dtEntity::StringProperty("StringPropValue1"));
      props.Add(dtEntity::SID("StringProp2"), new dtEntity::StringProperty("StringPropValue2"));
      spawner1->AddComponent(dtEntity::SID("TestComponent"), props);

      mMapSystem->AddSpawner(*spawner1);

      mMapSystem->SaveMapAs(mapname, targetpath);
      mMapSystem->UnloadMap(mapname);
   }
   {

      mMapSystem->LoadMap(mapname);

      dtEntity::Spawner* spawner1;
      bool found = mMapSystem->GetSpawner("TestSpawner1", spawner1);
      CHECK(found);
      if(!found) return;
      CHECK_EQUAL(true, spawner1->GetAddToSpawnerStore());
      CHECK_EQUAL("TestGuiCategory", spawner1->GetGUICategory());
      CHECK_EQUAL("TestIconPath", spawner1->GetIconPath());
      GroupProperty props = spawner1->GetComponentValues(dtEntity::SID("TestComponent"));
      CHECK_EQUAL((unsigned int)2, props.Get().size());
      CHECK_EQUAL("StringPropValue1", props.Get(dtEntity::SID("StringProp1"))->StringValue());
      CHECK_EQUAL("StringPropValue2", props.Get(dtEntity::SID("StringProp2"))->StringValue());
   }
   std::remove(targetpath.c_str());
}
