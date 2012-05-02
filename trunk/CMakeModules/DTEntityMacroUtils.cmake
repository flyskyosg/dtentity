MACRO(SETUP_PLUGIN PLUGIN_NAME)

    SET(TARGET_NAME ${PLUGIN_NAME} )
	
	IF (WIN32)
	  ADD_DEFINITIONS(-DNOMINMAX)
	ENDIF (WIN32)

	INCLUDE_DIRECTORIES( 
	  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/
	  ${OSG_INCLUDE_DIRS}
	  ${CMAKE_CURRENT_SOURCE_DIR}
	)

  ADD_LIBRARY(${TARGET_NAME} ${DTENTITY_PLUGINS_DYNAMIC_OR_STATIC}
		${TARGET_H}
		${TARGET_SRC}
	)

  TARGET_LINK_LIBRARIES(${TARGET_NAME} ${TARGET_ADDED_LIBRARIES} ${TARGET_COMMON_LIBRARIES})
	
  INCLUDE(PluginInstall OPTIONAL)

  SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
  
  IF (MSVC)    
    SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES PREFIX "../../plugins/")
    SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES IMPORT_PREFIX "../")
  ENDIF (MSVC) 

ENDMACRO(SETUP_PLUGIN)


MACRO(REPLACE_SIDS_IN_SOURCE SOURCES_OUT)
  
  # Passing a list as macro arg gets expanded to multiple args.
  # catch by accessing ARGN, which is a list hold ing all unexpected
  # macro args.
  SET(SOURCES_IN ${ARGN})
  
  # empty output list if it exists
  SET(${SOURCES_OUT})
  
  IF(DTENTITY_REPLACE_SIDS_WITH_PREPROCESSOR AND NOT DTENTITY_USE_STRINGS_AS_STRINGIDS)
	  foreach(iterator ${SOURCES_IN})
		# read from this path
		SET(SID_ORIGIN ${CMAKE_CURRENT_SOURCE_DIR}/${iterator})
	    
		# write to this path
		SET(SID_TARGET ${CMAKE_CURRENT_BINARY_DIR}/${iterator}.cpp)
		
		get_filename_component(EXTENSION ${SID_ORIGIN} EXT)
		
	    IF(EXTENSION STREQUAL ".cpp")
			# DTENTITY_SID_DB_PATH should hold write location for SID text file
			add_custom_command (
			  OUTPUT ${SID_TARGET}
			  COMMAND HashSids ${SID_ORIGIN} ${SID_TARGET} ${DTENTITY_SID_DB_PATH}
			  DEPENDS HashSids ${SID_ORIGIN}
			)
			
			# write destination to sources out global var
			LIST(APPEND ${SOURCES_OUT} ${SID_TARGET})	  
			set_source_files_properties(${SID_ORIGIN} PROPERTIES HEADER_FILE_ONLY true)
		    set_source_files_properties(${SID_TARGET} PROPERTIES GENERATED true)
		ELSE(EXTENSION STREQUAL ".cpp")
		  LIST(APPEND ${SOURCES_OUT} ${SID_ORIGIN})	  
		ENDIF(EXTENSION STREQUAL ".cpp")
		
		
	  endforeach(iterator)
  ELSE(DTENTITY_REPLACE_SIDS_WITH_PREPROCESSOR AND NOT DTENTITY_USE_STRINGS_AS_STRINGIDS)
    set(SOURCES_OUT ${SOURCES_IN})
  ENDIF(DTENTITY_REPLACE_SIDS_WITH_PREPROCESSOR AND NOT DTENTITY_USE_STRINGS_AS_STRINGIDS)
ENDMACRO(REPLACE_SIDS_IN_SOURCE)