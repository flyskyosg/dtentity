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

#include <dtEntityOSG/texturelabelcomponent.h>

#include <dtEntity/nodemasks.h>
#include <dtEntity/stringid.h>
#include <osg/BlendFunc>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Program>
#include <osg/Image>
#include <osg/LineWidth>
#include <osg/Point>
#include <osg/PointSprite>
#include <osg/PolygonMode>
#include <osg/Texture2D>
#include <osg/StateSet>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osg/Multisample>
#include <osg/PolygonOffset>
#include <osgDB/ReadFile>

namespace dtEntityOSG
{
   

   const dtEntity::StringId TextureLabelComponent::TYPE(dtEntity::SID("TextureLabel"));
   const dtEntity::StringId TextureLabelComponent::OffsetId(dtEntity::SID("Offset"));
   const dtEntity::StringId TextureLabelComponent::ColorId(dtEntity::SID("Color"));
   const dtEntity::StringId TextureLabelComponent::PathId(dtEntity::SID("Path"));
   const dtEntity::StringId TextureLabelComponent::VisibleId(dtEntity::SID("Visible"));
   const dtEntity::StringId TextureLabelComponent::MaxSizeId(dtEntity::SID("MaxSize"));
   const dtEntity::StringId TextureLabelComponent::MinSizeId(dtEntity::SID("MinSize"));
   const dtEntity::StringId TextureLabelComponent::DistanceAttenuationId(dtEntity::SID("DistanceAttenuation"));
   const dtEntity::StringId TextureLabelComponent::AlwaysOnTopId(dtEntity::SID("AlwaysOnTop"));
   const dtEntity::StringId TextureLabelComponent::AddConnectingLineId(dtEntity::SID("AddConnectingLine"));

   ////////////////////////////////////////////////////////////////////////////
   TextureLabelComponent::TextureLabelComponent()
      : NodeComponent(new osg::Geode())
      , mOffset(
         dtEntity::DynamicVec3Property::SetValueCB(this, &TextureLabelComponent::SetOffset),
         dtEntity::DynamicVec3Property::GetValueCB(this, &TextureLabelComponent::GetOffset)
      )
      , mColor(
         dtEntity::DynamicVec4Property::SetValueCB(this, &TextureLabelComponent::SetColor),
         dtEntity::DynamicVec4Property::GetValueCB(this, &TextureLabelComponent::GetColor)
      )
      , mColorVal(osg::Vec4(1,1,1,1))
      , mVisible(
         dtEntity::DynamicBoolProperty::SetValueCB(this, &TextureLabelComponent::SetVisible),
         dtEntity::DynamicBoolProperty::GetValueCB(this, &TextureLabelComponent::GetVisible)
      )
      , mLabelSystem(NULL)
   {
      osg::Geode* geode = static_cast<osg::Geode*>(GetNode());

      geode->setNodeMask(dtEntity::NodeMasks::VISIBLE);

      GetNode()->setName("TextureLabelComponent");
      
      Register(OffsetId, &mOffset);
      Register(PathId, &mPath);
      Register(MaxSizeId, &mMaxSize);
      Register(MinSizeId, &mMinSize);
      Register(DistanceAttenuationId, &mDistanceAttenuation);
      Register(ColorId, &mColor);
      Register(VisibleId, &mVisible);
      Register(AlwaysOnTopId, &mAlwaysOnTop);

      mMaxSize.Set(1024);
      mMinSize.Set(0);
      mDistanceAttenuation.Set(osg::Vec3(0, 0, 0));

      mVisible.Set(true);
      mAlwaysOnTop.Set(true);

      mDotGeometry = new osg::Geometry();
      geode->addDrawable(mDotGeometry);
      
      osg::Vec3Array* verts = new osg::Vec3Array(1);
      (*verts)[0] = mOffset.Get();
      mDotGeometry->setVertexArray(verts);
      mDotGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, verts->size()));

      // have to set bound to be non-zero, otherwise point is not shown
      // I actually make it quite big because otherwise the label is cut off as soon as
      // the center of the actor is offscreen.
      osg::BoundingBox bb(-1000, -1000, -1000, 1000, 1000, 1000);
      mDotGeometry->setInitialBound(bb);

      mColorUniform = new osg::Uniform("texlabelcolor", osg::Vec4(mColor.Get()));
      

   }
    
   ////////////////////////////////////////////////////////////////////////////
   TextureLabelComponent::~TextureLabelComponent()
   {
   }

   ////////////////////////////////////////////////////////////////////////////
   void TextureLabelComponent::SetVisible(bool v)
   {
      osg::Geode* geode = static_cast<osg::Geode*>(GetNode());

      bool visible = GetVisible();
      if(v && !visible)
      {
         geode->addDrawable(mDotGeometry);
         
      }
      else if(!v && visible)
      {
         geode->removeDrawable(mDotGeometry);
      }
   }

   ////////////////////////////////////////////////////////////////////////////
   bool TextureLabelComponent::GetVisible() const
   {
      osg::Geode* geode = static_cast<osg::Geode*>(GetNode());
      return (mDotGeometry.valid() && 
         mDotGeometry->getNumParents() > 0 &&
         mDotGeometry->getParent(0) == geode);
   }

   ////////////////////////////////////////////////////////////////////////////
   void TextureLabelComponent::SetOffset(const osg::Vec3& o)
   {
      mOffsetVal = o;
      osg::Vec3Array* verts = static_cast<osg::Vec3Array*>(mDotGeometry->getVertexArray());
      (*verts)[0] = o;
      mDotGeometry->dirtyDisplayList();
      //mDotGeometry->dirtyBound();
   }

   ////////////////////////////////////////////////////////////////////////////
   void TextureLabelComponent::SetColor(const osg::Vec4& v)
   {
      mColorVal = v;
      mColorUniform->set(v);
   }

   ////////////////////////////////////////////////////////////////////////////
   osg::Geode* TextureLabelComponent::GetGeode() const
   {
      return static_cast<osg::Geode*>(GetNode());
   }

   ////////////////////////////////////////////////////////////////////////////
   void TextureLabelComponent::OnAddedToEntity(dtEntity::Entity& entity)
   {  
      BaseClass::OnAddedToEntity(entity);
      entity.GetEntityManager().GetEntitySystem(TYPE, mLabelSystem);
   }

   ////////////////////////////////////////////////////////////////////////////
   void TextureLabelComponent::Finished()
   {
      assert(mLabelSystem != NULL);
      mLabelSystem->SetupLabel(*this);
      BaseClass::Finished();
      SetVisible(mLabelSystem->GetEnabled());
   }
  
   ////////////////////////////////////////////////////////////////////////////
   ////////////////////////////////////////////////////////////////////////////
   const dtEntity::StringId TextureLabelSystem::TYPE(dtEntity::SID("TextureLabel"));
   const dtEntity::StringId TextureLabelSystem::EnabledId(dtEntity::SID("Enabled"));

   TextureLabelSystem::TextureLabelSystem(dtEntity::EntityManager& em)
      : dtEntity::DefaultEntitySystem<TextureLabelComponent>(em)
      , mEnabled(
         dtEntity::DynamicBoolProperty::SetValueCB(this, &TextureLabelSystem::SetEnabled),
         dtEntity::DynamicBoolProperty::GetValueCB(this, &TextureLabelSystem::GetEnabled)
      )
      , mEnabledVal(true)
   {
      Register(EnabledId, &mEnabled);


      mProgram = new osg::Program();

      mProgram->setName("TextureLabel");
      

      std::string vsrc = ""
      "uniform sampler2D diffuseTexture;\n"
      "uniform vec4 texlabelcolor;\n"
      "\n"
      "void main(void)\n"
      "{\n"
      "   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
      "   gl_FrontColor = texlabelcolor;\n"
      "   gl_TexCoord[0] = gl_MultiTexCoord0;\n"
      "}";

      osg::Shader* vert = new osg::Shader(osg::Shader::VERTEX);
      vert->setShaderSource(vsrc);  
      mProgram->addShader(vert);
      std::string fsrc = ""
      "uniform sampler2D diffuseTexture;\n"
      "\n"
      "void main(void)\n"
      "{\n"
      "   vec4 diffuseColor = texture2D(diffuseTexture, gl_TexCoord[0].st);\n"
      "   gl_FragColor.rgb = diffuseColor.rgb * gl_Color;"
      "   gl_FragColor.a = diffuseColor.a;\n"
      "}";

      osg::Shader* frag = new osg::Shader(osg::Shader::FRAGMENT);
      frag->setShaderSource(fsrc);
      mProgram->addShader(frag);

   }  

   ////////////////////////////////////////////////////////////////////////////
   TextureLabelSystem::~TextureLabelSystem()
   {
   }

   ////////////////////////////////////////////////////////////////////////////
   void TextureLabelSystem::SetEnabled(bool v)
   {
      mEnabledVal = v;
      for(ComponentStore::iterator i = mComponents.begin(); i != mComponents.end(); ++i)
      {
         i->second->SetVisible(v);
      }
   }

   ////////////////////////////////////////////////////////////////////////////
   bool TextureLabelSystem::GetEnabled() const
   {
      return mEnabledVal;
   }

   ////////////////////////////////////////////////////////////////////////////
   void TextureLabelSystem::SetupLabel(TextureLabelComponent& component)
   {
      std::string path = osgDB::findDataFile(component.GetPath()).c_str();
      if(!path.empty())
      {
         component.mDotGeometry->setStateSet(GetSymbolStateset(path, component));      
      }
   }

   ////////////////////////////////////////////////////////////////////////////
   osg::StateSet* TextureLabelSystem::GetSymbolStateset(const std::string& symbolpath, TextureLabelComponent& component)
   {      
      unsigned int width = component.GetMaxSize();
      osg::StateSet* ss = new osg::StateSet();
      


      osg::PointSprite* sprite = new osg::PointSprite();
      sprite->setCoordOriginMode(osg::PointSprite::LOWER_LEFT);
      ss->setTextureAttributeAndModes(0, sprite, osg::StateAttribute::ON);

      osg::Texture2D* texture = new osg::Texture2D();
      texture->setResizeNonPowerOfTwoHint(false);
      texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::NEAREST);
      texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::NEAREST);
      texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
      texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
                 
      ss->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
      ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF);   
      
      osg::BlendFunc* blend = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
      ss->setAttribute(blend, osg::StateAttribute::ON);      
      ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
      ss->setRenderBinDetails(90, "DepthSortedBin");
      ss->setMode(GL_BLEND,osg::StateAttribute::ON);

      if(component.GetAlwaysOnTop())
      {
         ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
      }
      else
      {
         ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
      }

      ss->setAttributeAndModes(mProgram.get());
      ss->setMode(GL_FOG, osg::StateAttribute::OFF);

      osg::ref_ptr<osg::Image> image = osgDB::readImageFile(symbolpath);
      if(!image)
      {
         LOG_ERROR("Could not load image from " + symbolpath);
         return ss;
      }
      texture->setImage(image);
      ss->addUniform(component.GetColorUniform());

      osg::Point* point = new osg::Point();
      point->setSize(image->s());
      point->setMinSize(component.GetMinSize());
      point->setMaxSize(width);
      point->setDistanceAttenuation(component.GetDistanceAttenuation());

      ss->setAttribute(point);

      return ss;
   }
}
