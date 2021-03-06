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

#include <dtEntityOSG/matrixtransformcomponent.h>
#include <osg/MatrixTransform>

namespace dtEntityOSG
{
   ////////////////////////////////////////////////////////////////////////////
   ////////////////////////////////////////////////////////////////////////////
   const dtEntity::StringId MatrixTransformComponent::TYPE(dtEntity::SID("MatrixTransform"));
   const dtEntity::StringId MatrixTransformComponent::MatrixId(dtEntity::SID("Matrix"));
   
   ////////////////////////////////////////////////////////////////////////////
   MatrixTransformComponent::MatrixTransformComponent()
      : BaseClass(new osg::MatrixTransform())
      , mMatrix(
           dtEntity::DynamicMatrixProperty::SetValueCB(this, &MatrixTransformComponent::SetMatrix),
           dtEntity::DynamicMatrixProperty::GetValueCB(this, &MatrixTransformComponent::GetMatrix)
        )
   {
      Register(MatrixId, &mMatrix);
      GetNode()->setName("MatrixTransformComponent");
   }

   ////////////////////////////////////////////////////////////////////////////
   MatrixTransformComponent::MatrixTransformComponent(osg::MatrixTransform* trans)
      : BaseClass(trans)
      , mMatrix(
           dtEntity::DynamicMatrixProperty::SetValueCB(this, &MatrixTransformComponent::SetMatrix),
           dtEntity::DynamicMatrixProperty::GetValueCB(this, &MatrixTransformComponent::GetMatrix)
        )
   {
      Register(MatrixId, &mMatrix);
      GetNode()->setName("MatrixTransformComponent");
   }

   ////////////////////////////////////////////////////////////////////////////
   MatrixTransformComponent::~MatrixTransformComponent()
   {
   }

   ////////////////////////////////////////////////////////////////////////////
   osg::MatrixTransform* MatrixTransformComponent::GetMatrixTransform()
   {
      return static_cast<osg::MatrixTransform*>(GetNode());
   }

   ////////////////////////////////////////////////////////////////////////////
   const osg::MatrixTransform* MatrixTransformComponent::GetMatrixTransform() const
   {
      return static_cast<osg::MatrixTransform*>(GetNode());
   }

   ////////////////////////////////////////////////////////////////////////////
   dtEntity::Matrix MatrixTransformComponent::GetMatrix() const
   {
      return GetMatrixTransform()->getMatrix();
   }

   ////////////////////////////////////////////////////////////////////////////
   void MatrixTransformComponent::SetMatrix(const dtEntity::Matrix& m)
   {
      GetMatrixTransform()->setMatrix(m);
   }
}
