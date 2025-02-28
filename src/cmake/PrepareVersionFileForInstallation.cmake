# This script is used to prepare the version file for installations.
# It appends the short commit hash to the version file
# and updates the version in case it was overriden by the user
# (i.e. with -DVERSION=x.y.z)

file(READ "${CMAKE_SOURCE_DIR}/../VERSION.json" VERSION_JSON)

execute_process(
    COMMAND git rev-parse --short HEAD
    OUTPUT_VARIABLE GIT_SHORT_COMMIT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REGEX REPLACE "^v" "" CLEAN_VERSION "${VERSION}")
string(REGEX REPLACE "^V" "" CLEAN_VERSION "${CLEAN_VERSION}")
string(JSON MODIFIED_JSON SET "${VERSION_JSON}" version "\"${CLEAN_VERSION}\"")
string(JSON MODIFIED_JSON SET "${MODIFIED_JSON}" commit "\"${GIT_SHORT_COMMIT}\"")

file(WRITE "${CMAKE_BINARY_DIR}/VERSION.json" "${MODIFIED_JSON}")
