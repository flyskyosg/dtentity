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
 */

#include <dtEntityCEGUI/export.h>
#include <osg/ref_ptr>
#include <CEGUI/CEGUISubscriberSlot.h>
#include <CEGUI/CEGUIEvent.h>
#include <osgGA/GUIEventHandler>
#include <dtEntity/entitysystem.h>
#include <dtEntity/scriptaccessor.h>

namespace osg
{
   class Camera;
   class Group;
   class Texture2D;
}

namespace dtEntity
{
   class EntityManager;
}

namespace dtEntityCEGUI
{
   typedef CEGUI::Window Widget;
   typedef CEGUI::ScriptModule BaseScriptModule;

   class ScriptModule;

   /** 
    * Used for rendering and managing on-screen, graphical user interfaces.
    */
   class DTENTITY_CEGUI_EXPORT CEGUISystem
      : public dtEntity::EntitySystem
      , public dtEntity::ScriptAccessor
      , public osgGA::GUIEventHandler
   {
   public:
     
      static const dtEntity::ComponentType TYPE;

      /** 
       * Create the GUI instance.
       * @param camera The Camera the UI will be rendered to
       * @param keyboard The Keyboard to listen to for events
       * @param mouse The Mouse to listen to for events
       */
      CEGUISystem(dtEntity::EntityManager& em);
      virtual ~CEGUISystem();

      virtual dtEntity::ComponentType GetComponentType() const { return TYPE; }

      virtual void OnAddedToEntityManager(dtEntity::EntityManager& em);
      virtual void OnRemoveFromEntityManager(dtEntity::EntityManager& em);

      /** 
       * Sets the Camera to use for rendering to
       * @param camera If NULL, no GUI rendering will take place
       */
      void SetCamera(osg::Camera* camera);

      
      ///returns default sheet (autogenerated root-window)
      Widget* GetRootSheet();

      ///returns default sheet (autogenerated root-window)
      const Widget* GetRootSheet() const;

      /** 
       * Load a Layout file
       * @param fileName The filename of the .layout file
       * @param prefix Optional string prefix that is to be used when creating 
       *        the windows in the layout file, this function allows a layout to be
       *        loaded multiple times without having name clashes.
       * @param resourceGroup Optional string of the resource group this layout belongs to
       * @return The root Widget loaded from the .layout
       */
      Widget* LoadLayout(const std::string& fileName, 
                         const std::string& prefix="",
                         const std::string& resourceGroup = "");

     /** 
       * Load a Layout file and automatically add the loaded root Widget to the supplied parent widget
       * @param parent The parent widget to add the loaded layout to
       * @param fileName The filename of the .layout file
       * @param prefix Optional string prefix that is to be used when creating 
       *        the windows in the layout file, this function allows a layout to be
       *        loaded multiple times without having name clashes.
       * @param resourceGroup Optional string of the resource group this layout belongs to
       * @return The root Widget loaded from the .layout
       */
      Widget* LoadLayout(Widget* parent, const std::string& fileName, 
                         const std::string& prefix="", const std::string& resourceGroup = "" );

      /** 
        * Create a Widget and add it to the internal root Widget
        * @param typeName The type of widget to create (e.g., "WindowsLook/FrameWindow")
        * @param name The name to apply to the widget
        * @return The created Widget (could be NULL if an error occurred)
        */
      Widget* CreateWidget(const std::string& typeName, const std::string& name="");

      /** 
        * Create a Widget and add it to the supplied parent Widget.
        * @param parent The parent widget to add newly created widget to
        * @param typeName The type of widget to create (e.g., "WindowsLook/FrameWindow")
        * @param name The name to apply to the widget
        * @return The created Widget (could be NULL if an error occurred)
        */
      Widget* CreateWidget(Widget* parent, const std::string& typeName, const std::string& name="");

      /** 
        * Remove widget from its parent and destroy it
        * @param widget the widget to destroy
        */
      void DestroyWidget(Widget* widget);

      /** 
        * Find and return the widget by widget name.
        * @param name the name of the widget to find
        * @return The found widget (or NULL, if not found)
        */
      Widget* GetWidget(const std::string& name);

      /** 
        * Searches and return all Widgets that have the supplied text as part
        * of their name.
        * @param subName Part of the name of the Widgets to find
        * @param toFill The found Widgets with subName in their names (or empty, if none found)
        */
      void FindWidgets(const std::string& subName, std::vector<Widget*>& toFill);

      /** 
        * Searches and return the first Widget that has the supplied text as part
        * of its  name.
        * @param subName Part of the name of the Widget to find
        * @return The first found Widget (or NULL, if not found)
        */
      Widget* FindWidget(const std::string& subName);

      /** 
        * Set the directory for the supplied resource type.
        * @code
        * dtEntity::GUI::SetResourceGroupDirectory("layouts", "c:\temp\layouts");
        * @endcode
        * @param resourceType The type of resource to set the directory for.  Could be
        * "imagesets", "looknfeels", "layouts", "lua_scripts", "schemes", or "fonts"
        * @param directory The directory containing the resource types
        */
      static void SetResourceGroupDirectory(const std::string& resourceType, const std::string& directory);

      /** 
        * Set a directory for the supplied named resourceGroup by finding a 
        * file or directory using the Delta3D data file search paths.  If the
        * resourceToFind is found, its full path will be used as the ResourceGroup's
        * path.
        * @code
        * std::string path = dtEntity::GUI::SetResourceGroupFromResource("MyResourceGroup", "testImage.png");
        * if (!path.empty()) dtEntity::GUI::CreateImageset("MyImageset", "testImage.png", "MyResourceGroup");
        * @endcode
        * @param resourceGroup The name of a ResourceGroup to set the directory on
        * @param resourceToFind The file name or directory to find, using the data file search paths
        * @return The path used for the ResourceGroup or empty() if the resourceToFind wasn't found.
        * @see SetResourceGroupDirectory()
        * @see dtUtil::SetDataFilePathList()
        */
      static std::string SetResourceGroupFromResource(const std::string& resourceGroup, const std::string& resourceToFind);

      typedef CEGUI::SubscriberSlot Subscriber;
      /** 
        * Subscribe a callback to the supplied widget's event.  
        * @param widgetName The name of an existing widget
        * @param event The name of an event generated by the widget
        * @param subscriber The callback functor to receive the event
        * @return The Connection object, used this to manage the connection
        */
      CEGUI::Event::Connection SubscribeEvent(const std::string& widgetName, 
                                              const std::string& event,
                                              Subscriber subscriber);

      /** 
        * Subscribe a callback to the supplied widget's event.  
        * @param window An existing widget 
        * @param event The name of an event generated by the widget
        * @param subscriber The callback functor to receive the event
        * @return The Connection object, used this to manage the connection
        */
      CEGUI::Event::Connection SubscribeEvent(Widget& window, 
                                              const std::string& event, 
                                              Subscriber subscriber);

      /** 
        * Is the supplied widget present in the system?
        * @param widgetName The name of widget
        * @return true if the widget exists, false otherwise
        */
      static bool IsWindowPresent(const std::string& widgetName);

      /** 
        * Is the supplied Imageset present in the system?
        * @param imagesetName The name of ImageSet
        * @return true if the ImageSet exists, false otherwise
        */
      static bool IsImagesetPresent(const std::string& imagesetName);

      /** 
        * Create an ImageSet based on the supplied Image file. The Imageset will 
        * initially have a single image defined named "full_image" which is an image 
        * that represents the entire area of the loaded image. 
        * @param imagesetName 
        * @param fileName The name of the Image file to load
        * @param resourceGroup Optional resource group this imageset belongs to
        */
      static void CreateImageset(const std::string& imagesetName,
                                 const std::string& fileName,
                                 const std::string& resourceGroup = "");

      /** 
        * Destroy the supplied imageset if it exists
        * @param imagesetName The name of ImageSet
        */
      static void DestroyImageset(const std::string& imagesetName);

      /** 
        * Sets whether an Imageset should be auto scaled or not
        * @param imagesetName The name of the Imageset
        * @param autoScale Whether we want to autoscale the imageset or not
        */
      static void AutoScaleImageset(const std::string& imagesetName, bool autoScale);

      /** 
        * Define an image on the given imageset with the given properties
        * @param imagesetName The name of the Imageset
        * @param image The name of the Image to define
        * @param position The position of the image
        * @param size The size of the image
        * @param offset The amount to offset rendering from the position
        */
      static void DefineImage(const std::string& imagesetName, const std::string& image,
         osg::Vec2 position, osg::Vec2 size, osg::Vec2 offset);

      /** 
        * Is the supplied Scheme present in the system?
        * Note: Do not include ".scheme" in the name
        * @param schemeName The name of Scheme
        * @return true if the Scheme exists, false otherwise
        */
      static bool IsSchemePresent(const std::string& schemeName);

      /**
       * Shortcut to the CEGUI::SchemeManager
       * @param fileName The filename of the .scheme file to load
       * @param resourceGroup The name of the optional resource group this belongs to
       */
      static void LoadScheme(const std::string& fileName, const std::string& resourceGroup = "schemes");

      /**
       * Set the image for the rendered mouse cursor.
       * @param imagesetName The ImageSet name
       * @param imageName The name of the Image defined in the ImageSet
       */
      static void SetMouseCursor(const std::string& imagesetName, const std::string& imageName);

      /**
       * Show the CEGUI cursor
       */
      static void ShowCursor();

      /**
       * Hide the CEGUI cursor
       */
      static void HideCursor();

      /**
       * Set the ScriptModule to use with CEGUI.  This is typically used to 
       * map callbacks to CEGUI Events when loading from a .layout file.
       * @param scriptModule : The script module to set on the CEGUI::System
       */
      static void SetScriptModule(BaseScriptModule* scriptModule);

      /** 
       * Get the currently assigned ScriptModule used by CEGUI.
       * @return The currently assigned ScriptModule, could be NULL
       */
      static BaseScriptModule* GetScriptModule();

      /**
       * Get the mapping of all layouts loaded into the GUI.
       */
      const std::map<std::string, CEGUI::Window*>& GetLayoutMap() const {return mLayoutMap;}

      /**
       * Get the internal root node to change any stateset attributes, add layers
       * to the UI or to perform extra tweaking not anticipated by this class.
       * @param The root node that's in the scene graph
       */
      osg::Group& GetRootNode();

      /**
       * Set the parser to be used for parsing UI scheme and layout files.
       * @param parserName Name of the preferred XML parser.
       *        For CEGUI, the name should be one of those listed in the documentation
       *        for method CEGUISystem::setDefaultXMLParserName.
       * @return TRUE if the specified parser was successfully found and set.
       */
      static bool SetDefaultParser(const std::string& parserName);

      /**
       * Get the const internal root node to change any stateset attributes, add layers
       * to the UI or to perform extra tweaking not anticipated by this class.
       * @param The const root node that's in the scene graph
       */
      const osg::Group& GetRootNode() const;

      /**
       * Convenience method for creating render-to-texture texture for a specified widget.
       * @param widget The widget to be modified.  Will create a new Imageset and set the Image
       *               on the Property, referenced by propertyName.
       * @param dimensions The dimensions to set for the created texture. By default the method
       *        will use the dimensions of the widget if this argument is left NULL.
       * @param newImagesetName Use a unique image set name to represent the texture independently.
       *        WARNING, do not use the name of an existing imageset or problems may occur.
       * @param propertyName The optional name of the Property to apply the new Image to.  Defaults to "Image".
       * @param newImageName Name of the image area on the imageset texture.
       *        Not specifying this name will default to the same name as the image set,
       *        since it should be unique.
       * @return A fully qualified render-to-texture Texture2D.  Anything rendered to this texture
       *         will be seen on the supplied widget.
       */
      osg::ref_ptr<osg::Texture2D> CreateRenderTargetTexture(Widget& widget,
                                                               const osg::Vec2* dimensions,
                                                               const std::string& newImagesetName,
                                                               const std::string& propertyName = "Image",
                                                               const std::string& newImageName = "");

      /**
       * Convenience method for creating a Camera to render to a specified texture.
       * @param renderTargetTexture Texture to be the render target on the camera that is created.
       * @param viewDimensions The created camera's view port area dimensions. Normally this
       *        is the dimension of the widget being rendered to or the dimensions of the texture itself.
       * @return New Camera that has been set up to render to the specified texture.
       */
      osg::ref_ptr<osg::Camera> CreateCameraForRenderTargetTexture(osg::Texture2D& renderTargetTexture,
                                                                        const osg::Vec2& viewDimensions);

      /**
       * Add an alternate sub-directory (relative to any project context)
       * to search when trying to find a file in a specified resource group.
       * @param resourceGroup The resource group for the alternate search suffix.
       * @param searchSuffix The sub-directory to append to existing project search
       *        paths. This suffix will be used in case a file is not found for the
       *        specified resource group.
       * @return TRUE if the suffix was successfully added.
       */
      bool AddSearchSuffix(const std::string& resourceGroup, const std::string& searchSuffix);

      /**
       * Remove a previously added search suffix for a specified resource group.
       * @param resourceGroup The resource group for the alternate search suffix.
       * @param searchSuffix The sub-directory search suffix that was previously added
       *        for the specified resource group. This should match exactly as it was added.
       * @return TRUE if the suffix was successfully found and removed.
       */
      bool RemoveSearchSuffix(const std::string& resourceGroup, const std::string& searchSuffix);

      /**
       * Remove all previously added search suffixes for a specified resource group.
       * @param resourceGroup The resource group for the alternate search suffixes.
       * @return The number of suffixes that were successfully removed.
       */
      unsigned RemoveSearchSuffixes(const std::string& resourceGroup);

      /**
       * Remove all previously added search suffixes for all resource groups.
       * @return The number of suffixes that were successfully removed.
       */
      unsigned ClearSearchSuffixes();

      /**
       * React to OSG input events
       */
      virtual bool handle(const osgGA::GUIEventAdapter& ea, 
                          osgGA::GUIActionAdapter& aa, osg::Object*,
                          osg::NodeVisitor *);
   	
   private:
      void _SetupInternalGraph();
      void _SetupDefaultUI();

      dtEntity::Property* ScriptLoadScheme(const dtEntity::PropertyArgs& args);      
      dtEntity::Property* ScriptSetMouseCursor(const dtEntity::PropertyArgs& args);      
      dtEntity::Property* ScriptShowCursor(const dtEntity::PropertyArgs& args);      
      dtEntity::Property* ScriptHideCursor(const dtEntity::PropertyArgs& args);     
      dtEntity::Property* ScriptCreateWidget(const dtEntity::PropertyArgs& args);      
      dtEntity::Property* ScriptDestroyWidget(const dtEntity::PropertyArgs& args);   
      dtEntity::Property* ScriptSetWidgetProperty(const dtEntity::PropertyArgs& args);  
      dtEntity::Property* ScriptSetCallback(const dtEntity::PropertyArgs& args);

      static void _SetupSystemAndRenderer();
      static bool SystemAndRendererCreatedByHUD;

      osg::ref_ptr<osg::Group>                   mInternalGraph; ///osg graph used to render the gui
      CEGUI::Window*                               mRootSheet; ///auto-generated panel-window

      osg::ref_ptr<osg::Camera>        mCamera; ///camera, whose viewport is used to draw the gui
      
      std::map<std::string, CEGUI::Window*> mLayoutMap;
      double mLastFrameTime;
   };
}
