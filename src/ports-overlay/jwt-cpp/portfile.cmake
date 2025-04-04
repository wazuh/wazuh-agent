vcpkg_from_github(
    OUT_SOURCE_PATH
    SOURCE_PATH
    REPO
    Thalhammer/jwt-cpp
    REF
    "v${VERSION}"
    SHA512
    b6fdb93e3f2f065a2eb45fe16cb076a932b8d4bfad2251bd66d2be40d8afaf5c27a9cf17aaea61d8bfa3f5ff9ed3b45f90962dc14d72704ac5b9d717c12cc79f
    HEAD_REF
    master)

vcpkg_configure_cmake(SOURCE_PATH "${SOURCE_PATH}" OPTIONS -DJWT_DISABLE_PICOJSON=ON -DJWT_BUILD_EXAMPLES=OFF)

# Copy the header files
file(GLOB HEADER_FILES ${SOURCE_PATH}/include/jwt-cpp/*)
file(COPY ${HEADER_FILES} DESTINATION ${CURRENT_PACKAGES_DIR}/include/jwt-cpp)
