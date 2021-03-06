# OpenCMISS-Cmgui Application
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

IF (WIN32)
    EXECUTE_PROCESS(COMMAND "${SOURCE_DIR}/cmake/windatetime.bat" OUTPUT_VARIABLE RESULT OUTPUT_STRIP_TRAILING_WHITESPACE)
ELSEIF(UNIX)
    EXECUTE_PROCESS(COMMAND "date" "+%d/%m/%Y %H:%M:%S" OUTPUT_VARIABLE RESULT OUTPUT_STRIP_TRAILING_WHITESPACE)
ELSE (WIN32)
    MESSAGE(SEND_ERROR "date not implemented for this platform")
    SET(RESULT 00/00/0000 00:00:00)
ENDIF (WIN32)

SET( CMGUI_DATETIME_STRING ${RESULT} )

CONFIGURE_FILE( ${SOURCE_DIR}/source/configure/version.h.cmake ${VERSION_HDR} )
