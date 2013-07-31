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

#ifndef DT_ENTITY_SOUNDCOMPONENT
#define DT_ENTITY_SOUNDCOMPONENT

#include <osg/ref_ptr>
#include <dtEntityAudio/export.h>
#include <dtEntity/defaultentitysystem.h>
#include <dtEntity/dynamicproperty.h>
#include <dtEntity/component.h>
#include <dtEntity/message.h>
#include <dtEntity/stringid.h>
#include <dtEntityAudio/sound.h>


namespace dtEntityAudio
{ 
   ////////////////////////////////////////////////////////////////////////////////

   class SoundSystem;

   class DTENTITY_AUDIO_EXPORT SoundComponent : public dtEntity::Component
   {
   
      friend class SoundSystem;
      typedef Component BaseClass;

   public:

      static const dtEntity::ComponentType TYPE;
      static const dtEntity::StringId SoundPathId;
      static const dtEntity::StringId AutoPlayId;
      static const dtEntity::StringId GainId;
      static const dtEntity::StringId PitchId;
      static const dtEntity::StringId RollOffId;
      static const dtEntity::StringId LoopingId;
      
      SoundComponent();
     
      virtual ~SoundComponent();

      virtual dtEntity::ComponentType GetType() const { 
         return TYPE; 
      }

      virtual bool IsInstanceOf(dtEntity::ComponentType id) const
      { 
         return (id == TYPE); 
      }

      virtual void Finished();
      virtual void OnAddedToEntity(dtEntity::Entity& entity) { mOwner = &entity;}
      virtual void OnRemovedFromEntity(dtEntity::Entity& entity) { mOwner = NULL; }

      dtEntityAudio::Sound* GetCurrentSound() const { return mCurrentSound.get(); }

      void SetSoundPath(const std::string& p);
      std::string GetSoundPath() const { return mSoundPath.Get(); }

      void SetAutoPlay(bool p) { mAutoPlay.Set(p); }
      bool GetAutoPlay() const { return mAutoPlay.Get(); }

      bool IsPlaying() const;
      void PlaySound();
      void StopSound();

      void Update(float dt);
      void FreeSound();

   private:

      dtEntity::StringProperty mSoundPath;
      dtEntity::BoolProperty mAutoPlay;
      dtEntity::FloatProperty mGain;
      dtEntity::FloatProperty mPitch;
      dtEntity::FloatProperty mRollOff;
      dtEntity::BoolProperty mLooping;
      osg::ref_ptr<dtEntityAudio::Sound> mCurrentSound;
      dtEntity::Entity* mOwner;
   };

   
   ////////////////////////////////////////////////////////////////////////////////

   /// Manages sound effects
   /**
      The Sound system relies on the AudioManager, a singleton object which acts
      as the main interface to OpenAL.
      This systems is in charge of updating all sound components at each frame, via
      the OnTick method.
   */
   class DTENTITY_AUDIO_EXPORT SoundSystem : public dtEntity::DefaultEntitySystem<SoundComponent>
   {
      typedef DefaultEntitySystem<SoundComponent> BaseClass;

   public:

      static const dtEntity::ComponentType TYPE;
      
      SoundSystem(dtEntity::EntityManager& em);
      ~SoundSystem();

      static const dtEntity::StringId ListenerGainId;
      static const dtEntity::StringId ListenerEntityId;

      //void OnAddedToEntityManager(dtEntity::EntityManager& em)
      void OnRemoveFromEntityManager(dtEntity::EntityManager& em);
      void OnEnterWorld(const dtEntity::Message&);
      void OnLeaveWorld(const dtEntity::Message&);
      void OnTick(const dtEntity::Message& msg);

      /// Override base class behavior to save system properties to file
      virtual bool StorePropertiesToScene() const { return true; }


      virtual void Finished();

      void SetSoundPath(dtEntity::EntityId eid, const std::string& p);
      void PlaySound(dtEntity::EntityId eid);
      void StopSound(dtEntity::EntityId eid);

      void SetListenerEntity(dtEntity::EntityId id);
      dtEntity::EntityId GetListenerEntity() const;
      
   private:

      /// Internal util that copies current camera position and orientation to listener
      void CopyEntityTransformToListener();

      dtEntity::MessageFunctor mEnterWorldFunctor;
      dtEntity::MessageFunctor mLeaveWorldFunctor;
      dtEntity::MessageFunctor mTickFunctor;
      dtEntity::MessageFunctor mWindowClosedFunctor;
      dtEntity::FloatProperty mListenerGain;
      dtEntity::DynamicUIntProperty mListenerEntity;
      dtEntity::Component* mListenerEntityTrans;
      dtEntity::EntityId mListenerEntityVal;
   };
}  //dtEntityAudio

#endif // DT_ENTITY_SOUNDCOMPONENT 