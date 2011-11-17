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

#include <dtEntityQtWidgets/export.h>
#include <dtEntity/entitymanager.h>
#include <QtGui/QtGui>

namespace dtEntityQtWidgets
{

   ////////////////////////////////////////////////////////////////////////////////
   class SpawnerList : public QListWidget
   {
      Q_OBJECT

   public:

      SpawnerList();
      void mousePressEvent(QMouseEvent *event);
   };

   ////////////////////////////////////////////////////////////////////////////////
   class ENTITYQTWIDGETS_EXPORT SpawnerStoreView
      : public QWidget
   {
      Q_OBJECT

   public:
      
      SpawnerStoreView(QWidget* parent = NULL);
      virtual ~SpawnerStoreView();

   public slots:

      void OnAddSpawner(const QString& name, const QString& category, const QString& iconpath);
      void OnRemoveSpawner(const QString& name);

   private:
      QListWidget* mSpawnerList;

   }; 

   ////////////////////////////////////////////////////////////////////////////////
   class ENTITYQTWIDGETS_EXPORT SpawnerStoreController
      : public QObject
   {
      Q_OBJECT

   public:
      
      SpawnerStoreController(dtEntity::EntityManager*);
      virtual ~SpawnerStoreController();

      void SetupSlots(SpawnerStoreView* view);

      void OnSpawnerAdded(const dtEntity::Message& m);
      void OnSpawnerRemoved(const dtEntity::Message& m);

   signals:
      void AddSpawner(const QString& name, const QString& category, const QString& iconpath);
      void RemoveSpawner(const QString& name);

   public slots:
   
      void Init();
      void OnTextDroppedOntoGLWidget(const QPointF& pos, const QString&);

   private:
      dtEntity::EntityManager* mEntityManager;
      dtEntity::MessageFunctor mSpawnerAddedFunctor;
      dtEntity::MessageFunctor mSpawnerRemovedFunctor;
   };
}