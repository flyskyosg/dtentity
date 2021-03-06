#pragma once

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

#include <dtEntity/component.h>
#include <dtEntity/defaultentitysystem.h>
#include <dtEntity/dynamicproperty.h>
#include <dtEntity/property.h>
#include <dtEntityOSG/nodecomponent.h>
#include <osgManipulator/Dragger>
#include <osgParticle/ParticleSystem>
#include <osgParticle/ModularEmitter>
#include <osgParticle/ModularProgram>
#include <osgParticle/ParticleSystemUpdater>

namespace dtEntityOSG
{

   class ParticleComponent
         : public dtEntityOSG::NodeComponent
   {

   public:
      typedef dtEntityOSG::NodeComponent BaseClass;

      static const dtEntity::ComponentType TYPE;     
      
      static const dtEntity::StringId AlphaRangeId;
      static const dtEntity::StringId ColorRangeMinId;
      static const dtEntity::StringId ColorRangeMaxId;
      static const dtEntity::StringId CounterId;
      static const dtEntity::StringId DebugOnId;
      static const dtEntity::StringId EmissiveParticlesId;
      static const dtEntity::StringId EnabledId;
      static const dtEntity::StringId LifeTimeId;
      static const dtEntity::StringId LightingId;
      static const dtEntity::StringId MassId;
      static const dtEntity::StringId PlacerId;
      static const dtEntity::StringId SizeRangeId;
      static const dtEntity::StringId TextureFileId;     
      static const dtEntity::StringId TextureUnitId;      

      static const dtEntity::StringId __SELECTED__Id;
      static const dtEntity::StringId BoxId;       
      static const dtEntity::StringId CompositeId;      
      static const dtEntity::StringId MultiSegmentId;      
      static const dtEntity::StringId PointId;      
      static const dtEntity::StringId SectorId;      
      static const dtEntity::StringId SegmentId;      

      static const dtEntity::StringId XRangeId;      
      static const dtEntity::StringId YRangeId;      
      static const dtEntity::StringId ZRangeId;      
      static const dtEntity::StringId CenterId;

      static const dtEntity::StringId RandomRateCounterId;
      static const dtEntity::StringId ConstantRateCounterId;

      static const dtEntity::StringId RateRangeId;
      static const dtEntity::StringId MinimumNumberOfParticlesToCreateId;
      static const dtEntity::StringId NumberOfParticlesPerSecondToCreateId;

      static const dtEntity::StringId ShooterThetaRangeId;
      static const dtEntity::StringId ShooterPhiRangeId;
      static const dtEntity::StringId ShooterInitialSpeedRangeId;
      static const dtEntity::StringId ShooterInitialRotationalSpeedMinId;
      static const dtEntity::StringId ShooterInitialRotationalSpeedMaxId;

      static const dtEntity::StringId ShapeId;
      static const dtEntity::StringId ShapePointId;
      static const dtEntity::StringId ShapeQuadId;
      static const dtEntity::StringId ShapeQuadTriangeStripId;
      static const dtEntity::StringId ShapeHexagonId;
      static const dtEntity::StringId ShapeLineId;

      static const dtEntity::StringId OperatorsId;

      static const dtEntity::StringId BounceOperatorId;
      static const dtEntity::StringId ForceOperatorId;

      static const dtEntity::StringId FrictionId;
      static const dtEntity::StringId ResilienceId;
      static const dtEntity::StringId CutoffId;
      static const dtEntity::StringId DomainsId;
      static const dtEntity::StringId PlaneDomainId;
      static const dtEntity::StringId SphereDomainId;
      static const dtEntity::StringId ForceId;
      static const dtEntity::StringId NormalId;
      static const dtEntity::StringId DistId;
      static const dtEntity::StringId RadiusId;


      
      ParticleComponent();
      virtual ~ParticleComponent();

      virtual dtEntity::ComponentType GetType() const { return TYPE; }
      virtual bool IsInstanceOf(dtEntity::ComponentType id) const
      { 
         return (id == TYPE); 
      }

      virtual void OnAddedToEntity(dtEntity::Entity &entity);
      virtual void OnRemovedFromEntity(dtEntity::Entity &entity);

      virtual void Finished();

      void SetTextureFile(const std::string& v) { mTextureFile.Set(v); }
      std::string GetTextureFile() const { return mTextureFile.Get(); }

      void SetEmissiveParticles(bool v) { mEmissiveParticles.Set(v); }
      bool GetEmissiveParticles() const { return mEmissiveParticles.Get(); }

      void SetDebugOn(bool v);
      bool GetDebugOn() const { return mDebugOnVal; }

      void SetLighting(bool v) { mLighting.Set(v); }
      bool GetLighting() const { return mLighting.Get(); }

      void SetTextureUnit(int v) { mTextureUnit.Set(v); }
      int GetTextureUnit() const { return mTextureUnit.Get(); }

      void SetLifeTime(double v) { mLifeTime.Set(v); }
      double GetLifeTime() const { return mLifeTime.Get(); }

      void SetMass(float v) { mMass.Set(v); }
      float GetMass() const { return mMass.Get(); }

      void SetSizeRange(const osg::Vec2& v) { mSizeRange.Set(v); }
      osg::Vec2 GetSizeRange() const { return mSizeRange.Get(); }


      void SetCounter(const dtEntity::PropertyGroup&);
      dtEntity::PropertyGroup GetCounter() const;

      void SetPlacer(const dtEntity::PropertyGroup&);
      dtEntity::PropertyGroup GetPlacer() const;

      void SetShooterThetaRange(const osg::Vec2& v);
      osg::Vec2 GetShooterThetaRange() const;

      void SetShooterPhiRange(const osg::Vec2& v);
      osg::Vec2 GetShooterPhiRange() const;

      void SetShooterInitialSpeedRange(const osg::Vec2& v);
      osg::Vec2 GetShooterInitialSpeedRange() const;

      void SetShooterInitialRotationalSpeedMin(const osg::Vec3& v);
      osg::Vec3 GetShooterInitialRotationalSpeedMin() const;

      void SetShooterInitialRotationalSpeedMax(const osg::Vec3& v);
      osg::Vec3 GetShooterInitialRotationalSpeedMax() const;

      void SetOperators(const dtEntity::PropertyArray& v);
      dtEntity::PropertyArray GetOperators() const;

   protected:

      osg::ref_ptr<osg::Geode> mGeode;
      osg::ref_ptr<osgParticle::ParticleSystem> mParticleSystem;
      osg::ref_ptr<osgParticle::ModularEmitter> mModularEmitter;
      osg::ref_ptr<osgParticle::ModularProgram> mProgram;
      
      dtEntity::DynamicBoolProperty mDebugOn;
      bool mDebugOnVal;
      dtEntity::StringProperty mTextureFile;
      dtEntity::BoolProperty mEmissiveParticles;
      dtEntity::BoolProperty mEnabled;
      dtEntity::BoolProperty mLighting;
      dtEntity::IntProperty mTextureUnit;

      dtEntity::DoubleProperty mLifeTime;
      dtEntity::FloatProperty mMass;
      dtEntity::Vec2Property mSizeRange;
      dtEntity::Vec2Property mAlphaRange;
      dtEntity::Vec4Property mColorRangeMin;
      dtEntity::Vec4Property mColorRangeMax;

      dtEntity::DynamicGroupProperty mPlacer;
      dtEntity::GroupProperty mPlacerVal;
      dtEntity::DynamicGroupProperty mCounter;
      dtEntity::GroupProperty mCounterVal;
      
      dtEntity::DynamicVec2Property mShooterThetaRange;
      dtEntity::DynamicVec2Property mShooterPhiRange;
      dtEntity::DynamicVec2Property mShooterInitialSpeedRange;
      dtEntity::DynamicVec3Property mShooterInitialRotationalSpeedMin;
      dtEntity::DynamicVec3Property mShooterInitialRotationalSpeedMax;

      dtEntity::StringIdProperty mShape;

      dtEntity::DynamicArrayProperty mOperators;
      dtEntity::ArrayProperty mOperatorsVal;

   };


   ////////////////////////////////////////////////////////////////////////////////

   class ParticleSystem
      : public dtEntity::DefaultEntitySystem<ParticleComponent>
   {
      typedef dtEntity::DefaultEntitySystem<ParticleComponent> BaseClass;
      
   public:

      static const dtEntity::ComponentType TYPE;

      ParticleSystem(dtEntity::EntityManager& em);
      ~ParticleSystem();

   };
}
