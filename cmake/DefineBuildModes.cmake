#######################################################
### DEBUG mode

IF(UNIX)
  IF(CMAKE_COMPILER_IS_GNUCC)
    LIST ( APPEND CF_C_FLAGS_DEBUG       "-g -O0" )
    LIST ( APPEND CF_CXX_FLAGS_DEBUG     "-g -O0 -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC" )
    LIST ( APPEND CF_Fortran_FLAGS_DEBUG "-g -O0" )
  ENDIF(CMAKE_COMPILER_IS_GNUCC)
  IF(CF_ENABLE_OMP)
        LIST ( APPEND CF_C_FLAGS_DEBUG       "-fopenmp" )
    	LIST ( APPEND CF_CXX_FLAGS_DEBUG     "-fopenmp" )
	LIST ( APPEND CF_Fortran_FLAGS_DEBUG "-fopenmp" )
  ENDIF(CF_ENABLE_OMP)
ENDIF()

### TESTING THIS MODIFICATION
IF( WIN32 )
    LIST ( APPEND CF_C_FLAGS_DEBUG       "/Zi" )
    LIST ( APPEND CF_CXX_FLAGS_DEBUG     "/Zi" )
    LIST ( APPEND CF_Fortran_FLAGS_DEBUG "" )
ENDIF()

IF(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_ASSERTIONS   ON   )
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_TRACE        ON   )
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_LOGALL       ON   )
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_LOGDEBUG     ON   )
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_DEBUG_MACROS ON   )
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_STATIC       OFF  )
ENDIF()

#######################################################
### OPTIM mode

IF(UNIX)
  IF(CMAKE_COMPILER_IS_GNUCC)
    LIST ( APPEND CF_C_FLAGS_OPTIM       "-g -O2" )
    LIST ( APPEND CF_CXX_FLAGS_OPTIM     "-g -O2" )
    LIST ( APPEND CF_Fortran_FLAGS_OPTIM "-g -O2" )
  ENDIF(CMAKE_COMPILER_IS_GNUCC) 
  IF(CF_ENABLE_OMP)
        LIST ( APPEND CF_C_FLAGS_OPTIM       "-fopenmp" )
    	LIST ( APPEND CF_CXX_FLAGS_OPTIM     "-fopenmp" )
	LIST ( APPEND CF_Fortran_FLAGS_OPTIM "-fopenmp" )
  ENDIF(CF_ENABLE_OMP)
ENDIF(UNIX)


MARK_AS_ADVANCED( CF_CMAKE_CXX_FLAGS_OPTIM  CF_CMAKE_C_FLAGS_OPTIM CF_CMAKE_Fortran_FLAGS_OPTIM )

IF(CMAKE_BUILD_TYPE MATCHES "[Oo][Pp][Tt][Ii][Mm]")
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_ASSERTIONS   ON   )
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_TRACE        ON  )
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_LOGALL       ON   )
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_LOGOPTIM     ON   )
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_OPTIM_MACROS ON   )
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_STATIC       OFF  )
ENDIF()

#######################################################
### RelWithDebInfo mode

IF(UNIX)
  IF(CMAKE_COMPILER_IS_GNUCC)
    LIST ( APPEND CF_C_FLAGS_RelWithDebInfo       "-g -O2" )
    LIST ( APPEND CF_CXX_FLAGS_RelWithDebInfo     "-g -O2" )
    LIST ( APPEND CF_Fortran_FLAGS_RelWithDebInfo "-g -O2" )
  ENDIF(CMAKE_COMPILER_IS_GNUCC)
  IF(CF_ENABLE_OMP)
        LIST ( APPEND CF_C_FLAGS_RelWithDebInfo      "-fopenmp" )
    	LIST ( APPEND CF_CXX_FLAGS_RelWithDebInfo     "-fopenmp" )
	LIST ( APPEND CF_Fortran_FLAGS_RelWithDebInfo "-fopenmp" )
  ENDIF(CF_ENABLE_OMP)
ENDIF(UNIX)


MARK_AS_ADVANCED( CF_CMAKE_CXX_FLAGS_RelWithDebInfo  CF_CMAKE_C_FLAGS_RelWithDebInfo CF_CMAKE_Fortran_FLAGS_RelWithDebInfo )

IF(CMAKE_BUILD_TYPE MATCHES "[Re][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo]")
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_ASSERTIONS   ON   )
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_TRACE        ON  )
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_LOGALL       ON   )
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_LOGOPTIM     ON   )
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_OPTIM_MACROS ON   )
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_STATIC       OFF  )
ENDIF()

#######################################################
### RELEASE mode

IF(UNIX)
  IF(CMAKE_COMPILER_IS_GNUCC)
    LIST ( APPEND CF_C_FLAGS_RELEASE       "-O2 -DNDEBUG -fomit-frame-pointer" )
    LIST ( APPEND CF_CXX_FLAGS_RELEASE     "-O2 -DNDEBUG -fomit-frame-pointer" )
    LIST ( APPEND CF_Fortran_FLAGS_RELEASE "-O2 -DNDEBUG -fomit-frame-pointer" )
  ENDIF(CMAKE_COMPILER_IS_GNUCC)
  IF(CF_ENABLE_OMP)
        LIST ( APPEND CF_C_FLAGS_RELEASE       "-fopenmp" )
    	LIST ( APPEND CF_CXX_FLAGS_RELEASE     "-fopenmp" )
	LIST ( APPEND CF_Fortran_FLAGS_RELEASE "-fopenmp" )
  ENDIF(CF_ENABLE_OMP)
ENDIF(UNIX)


MARK_AS_ADVANCED( CF_CMAKE_CXX_FLAGS_RELEASE  CF_CMAKE_C_FLAGS_RELEASE CF_CMAKE_Fortran_FLAGS_RELEASE )

IF(CMAKE_BUILD_TYPE MATCHES "[Re][Ee][Ll][Ee][Aa][Ss][Ee]")
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_ASSERTIONS   OFF  )
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_TRACE        OFF  )
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_LOGALL       ON   )
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_LOGDEBUG     OFF  )
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_DEBUG_MACROS OFF  )
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_STATIC       OFF  )
ENDIF()

#######################################################
### CUDA mode

IF(UNIX)
   # LIST ( APPEND CF_C_FLAGS_CUDA       "-O2 -g -arch sm_20 -Xcompiler -fPIC" )
   # LIST ( APPEND CF_CXX_FLAGS_CUDA     "-O2 -g -arch sm_20 -Xcompiler -fPIC" )
   # LIST ( APPEND CF_Fortran_FLAGS_CUDA "-O2 -g -fPIC" )
   # LIST ( APPEND CF_CUDAC_FLAGS_CUDA   "-O2 -g -arch sm_20 -Xcompiler -fPIC" )
   LIST ( APPEND CF_C_FLAGS_CUDA       "-O3 -g -fPIC" )
   LIST ( APPEND CF_CXX_FLAGS_CUDA     "-O3 -g -fPIC" )
   LIST ( APPEND CF_Fortran_FLAGS_CUDA "-O3 -g -fPIC" )
   #LIST ( APPEND CF_CUDAC_FLAGS_CUDA   "-O3 -g -arch sm_20 -Xcompiler -fPIC" )
   LIST ( APPEND CF_CUDAC_FLAGS_CUDA   "-O3 -g -arch=sm_30 -Xcompiler -fPIC" )  

#   SET(CMAKE_SHARED_LIBRARY_Fortran_FLAGS "-fPIC")  
#   SET(CMAKE_SHARED_LIBRARY_C_FLAGS "-Xcompiler -fPIC")        
#   SET(CMAKE_SHARED_LIBRARY_CXX_FLAGS "-Xcompiler -fPIC")      
ENDIF(UNIX)

MARK_AS_ADVANCED( CF_CMAKE_CXX_FLAGS_CUDA  CF_CMAKE_C_FLAGS_CUDA CF_CMAKE_Fortran_FLAGS_CUDA CF_CUDAC_FLAGS_CUDA )

### CUDARELEASE mode

IF(UNIX)
   # LIST ( APPEND CF_C_FLAGS_CUDARELEASE       "-O2 -g -arch sm_20 -Xcompiler -fPIC" )
   # LIST ( APPEND CF_CXX_FLAGS_CUDARELEASE     "-O2 -g -arch sm_20 -Xcompiler -fPIC" )
   # LIST ( APPEND CF_Fortran_FLAGS_CUDARELEASE "-O2 -g -fPIC" )
   # LIST ( APPEND CF_CUDAC_FLAGS_CUDARELEASE   "-O2 -g -arch sm_20 -Xcompiler -fPIC" )
   LIST ( APPEND CF_C_FLAGS_CUDARELEASE       "-O3 -DNDEBUG -fPIC" )
   LIST ( APPEND CF_CXX_FLAGS_CUDARELEASE     "-O3 -DNDEBUG -fPIC" )
   LIST ( APPEND CF_Fortran_FLAGS_CUDARELEASE "-O3 -DNDEBUG -fPIC" )
   LIST ( APPEND CF_CUDAC_FLAGS_CUDARELEASE   "-O3 -DNDEBUG -arch sm_30 -Xcompiler -fPIC" )
#   LIST ( APPEND CF_CUDAC_FLAGS_CUDARELEASE   "-O3 -DNDEBUG -code=compute_35 -arch=sm_35 -Xcompiler -fPIC" )

#   SET(CMAKE_SHARED_LIBRARY_Fortran_FLAGS "-fPIC")  
#   SET(CMAKE_SHARED_LIBRARY_C_FLAGS "-Xcompiler -fPIC")        
#   SET(CMAKE_SHARED_LIBRARY_CXX_FLAGS "-Xcompiler -fPIC")      
ENDIF(UNIX)

MARK_AS_ADVANCED( CF_CMAKE_CXX_FLAGS_CUDARELEASE  CF_CMAKE_C_FLAGS_CUDARELEASE CF_CMAKE_Fortran_FLAGS_CUDARELEASE CF_CUDAC_FLAGS_CUDARELEASE )

#######################################################

IF(CMAKE_BUILD_TYPE MATCHES "[Re][Ee][Ll][Ee][Aa][Ss][Ee]")
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_ASSERTIONS   OFF  )
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_TRACE        OFF  )
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_LOGALL       ON   )
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_LOGDEBUG     OFF  )
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_DEBUG_MACROS OFF  )
  SET_VAR_IF_NOT_DEFINED ( CF_ENABLE_STATIC       OFF  )
ENDIF()


IF ( DEFINED CF_C_FLAGS_${CMAKE_BUILD_TYPE} )
	SET( CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE}  "${CF_C_FLAGS_${CMAKE_BUILD_TYPE}}"  CACHE STRING "Flags used by the C compiler during ${CMAKE_BUILD_TYPE} builds." FORCE )
ENDIF()
IF ( DEFINED CF_CXX_FLAGS_${CMAKE_BUILD_TYPE} )
	SET( CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}  "${CF_CXX_FLAGS_${CMAKE_BUILD_TYPE}}"  CACHE STRING "Flags used by the C++ compiler during ${CMAKE_BUILD_TYPE} builds." FORCE )
ENDIF()
IF ( DEFINED CF_Fortran_FLAGS_${CMAKE_BUILD_TYPE} )
	SET( CMAKE_Fortran_FLAGS_${CMAKE_BUILD_TYPE}  "${CF_Fortran_FLAGS_${CMAKE_BUILD_TYPE}}"  CACHE STRING "Flags used by the C compiler during ${CMAKE_BUILD_TYPE} builds." FORCE )
ENDIF()
IF ( DEFINED CF_CUDAC_FLAGS_${CMAKE_BUILD_TYPE} )
        SET( CMAKE_CUDA_FLAGS_${CMAKE_BUILD_TYPE}  "${CF_CUDAC_FLAGS_${CMAKE_BUILD_TYPE}}"  CACHE STRING "Flags used by the CUDA compiler during ${CMAKE_BUILD_TYPE} builds." FORCE )
ENDIF()
