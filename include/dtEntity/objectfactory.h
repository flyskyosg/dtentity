#pragma once

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

#include <map>
#include <vector>

namespace dtEntity
{
   /**
    * Templated function to provide a generic object construction
    * utility.
    */
   template<typename BaseType, typename DerivedType>
   BaseType *construct()
   {
      return new DerivedType();
   }

   /**
    * This class is a template object factory.  It allows one to
    * create any type of object as long as there is a common base
    * class.  The common base class is defined on a per-factory
    * basis using the templated parameter <code>BaseType</code>.
    * @note
    *   The ObjectFactory implementation only supports objects with
    *   a default constructor.  It will not work with objects that
    *   only have named constructors.
    */
   template<typename UniqueIdTypeClass,typename BaseTypeClass,typename ltCmpClass=std::less<UniqueIdTypeClass> >
   class ObjectFactory
   {
   public:

      typedef UniqueIdTypeClass UniqueIdType;
      typedef BaseTypeClass BaseType;
      typedef ltCmpClass ltCmp;

      typedef BaseType *(*createObjectFunc)(); /// Function pointer type for functions creating objects.
      typedef std::map<UniqueIdType,createObjectFunc,ltCmp> ObjectMap;
      typedef typename ObjectMap::iterator ObjTypeItor;
      typedef typename ObjectMap::const_iterator ObjTypeItorConst;
      ObjectFactory() {}  // constructor


      /**
       * Registers a new type of object with the factory.
       * @return false if the type is a duplicate.
       */
      template<typename DerivedType>
      bool RegisterType(UniqueIdType id)
      {
         if (this->objectTypeMap.find(id) != this->objectTypeMap.end())
         {
            return false;
         }

         this->objectTypeMap[id] = &construct<BaseType,DerivedType>;
         return true;
      }

      /**
       * Removes an existing object type from the factory's known list
       * of object types.
       */
      void RemoveType(UniqueIdType id) {
         this->objectTypeMap.erase(id);
      }

      /**
       * Checks to see if the factory can create objects of the given type.
       * @param id The type of object to check for.
       * @return True if the type is supported, false otherwise.
       */
      bool IsTypeSupported(UniqueIdType id) const
      {
         ObjTypeItorConst itor(this->objectTypeMap.find(id));
         if (itor != this->objectTypeMap.end())
         {
            return true;
         }
         else
         {
            return false;
         }
      }

      /**
       * Gets a list of types that this factory knows how to create.
       */
      void GetSupportedTypes(std::vector<UniqueIdType> &types) const
      {
         types.clear();
         for (ObjTypeItorConst itor=this->objectTypeMap.begin();
            itor != this->objectTypeMap.end(); ++itor)
         {
            types.push_back(itor->first);
         }
      }

      /**
       * Creates a new object.
       * @param id - Type of object to create.
       * @return Returns a pointer to the newly created object or NULL if the given id has not been registered.
       * @throw Exception is thrown if the factory does not know how to create
       *  the requested type.
       */
      BaseType* CreateObject(const UniqueIdType id) const
      {
         ObjTypeItorConst itor(this->objectTypeMap.find(id));

         // We cannot create a new object if we do not know what type it is
         // so throw an exception.
         if (itor == this->objectTypeMap.end())
         {
            return 0;
         }

         return (itor->second)();
      }

      const ObjectMap& GetMap() const { return objectTypeMap; }

   private:
      ///Maps a unique id to a function pointer that when called creates an
      ///object of the appropriate type.
      ObjectMap objectTypeMap;
   };
}
