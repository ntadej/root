# - Find CppUnit
# This module finds an installed CppUnit package.
#
# It sets the following variables:
#  CPPUNIT_FOUND       - Set to false, or undefined, if CppUnit isn't found.
#  CPPUNIT_INCLUDE_DIR - The CppUnit include directory.
#  CPPUNIT_LIBRARY     - The CppUnit library to link against.

FIND_PATH(CPPUNIT_INCLUDE_DIR cppunit/Test.h)
FIND_LIBRARY(CPPUNIT_LIBRARY NAMES cppunit)

IF (CPPUNIT_INCLUDE_DIR AND CPPUNIT_LIBRARY)
   SET(CPPUNIT_FOUND TRUE)
ENDIF (CPPUNIT_INCLUDE_DIR AND CPPUNIT_LIBRARY)

IF (CPPUNIT_FOUND)

   # show which CppUnit was found only if not quiet
   IF (NOT CPPUNIT_FIND_QUIETLY)
      MESSAGE(STATUS "Found CppUnit: ${CPPUNIT_LIBRARY}")
   ENDIF (NOT CPPUNIT_FIND_QUIETLY)

ELSE (CPPUNIT_FOUND)

   # fatal error if CppUnit is required but not found
   IF (CPPUNIT_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find CppUnit")
   ENDIF (CPPUNIT_FIND_REQUIRED)

ENDIF (CPPUNIT_FOUND)
