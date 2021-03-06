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

      virtual void OnAddedToEntity(dtEntity::Entity& entity) { mOwner = &entity;}
      virtual void OnRemovedFromEntity(dtEntity::Entity& entity) { mOwner = NULL; }


      // accessors

      dtEntityAudio::Sound* GetCurrentSound() const { return mCurrentSound.get(); }

      std::string GetSoundPath() const { return mSoundPath; }
      void SetSoundPath(const std::string& val);
      bool GetAutoPlay() const { return mAutoPlay; }
      void SetAutoPlay(bool val);
      bool GetLooping() const { return mLooping; }
      void SetLooping(bool val);
      float GetGain() const { return mGain; }
      void SetGain(float val);
      float GetPitch() const { return mPitch; }
      void SetPitch(float val);
      float GetRollOff() const { return mRollOff; }
      void SetRollOff(float val);


      // other utils

      bool IsPlaying() const;
      void PlaySound();
      void StopSound();



   private:

      /// initializes the component as needed
      void Init();
      /// Update properties on the related sound objects
      void UpdateSoundValues();
      /// reset the component to default then initializes it again
      void Reinit();

      /// Updates sound object (normally called at every tick)
      void Update(float dt);
      /// Frees the related sound object
      void FreeSound();

      /// parent entity
      dtEntity::Entity* mOwner;
      /// flag indicating if the component has been initialized
      bool mIsInitialized;
      /// sound object linked to this component
      osg::ref_ptr<dtEntityAudio::Sound> mCurrentSound;

      // dynamic properties
      dtEntity::DynamicStringProperty mSoundPathProp;
      dtEntity::DynamicBoolProperty mAutoPlayProp;
      dtEntity::DynamicFloatProperty mGainProp;
      dtEntity::DynamicFloatProperty mPitchProp;
      dtEntity::DynamicFloatProperty mRollOffProp;
      dtEntity::DynamicBoolProperty mLoopingProp;

      std::string mSoundPath;
      bool mAutoPlay;
      bool mLooping;
      float mGain;
      float mPitch;
      float mRollOff;


      
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

      void OnAddedToEntityManager(dtEntity::EntityManager& em);
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
