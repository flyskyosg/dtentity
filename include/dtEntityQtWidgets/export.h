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

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
#  ifdef ENTITYQTWIDGETS_LIBRARY
#    define ENTITYQTWIDGETS_EXPORT __declspec(dllexport)
#  else
#     define ENTITYQTWIDGETS_EXPORT __declspec(dllimport)
#   endif
#else
#   ifdef ENTITYQTWIDGETS_LIBRARY
#      define ENTITYQTWIDGETS_EXPORT __attribute__ ((visibility("default")))
#   else
#      define ENTITYQTWIDGETS_EXPORT
#   endif
#endif

#ifdef _WIN32
#   pragma warning (disable: 4251)
#   pragma warning(disable : 4355) // 'this' used in initializer list
#endif
