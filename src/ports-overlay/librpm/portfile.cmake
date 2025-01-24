vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO rpm-software-management/rpm
  REF 9fc611e38004b1ddaa6668297408649ed556c157
  SHA512 847e55ec10462df517094385c8a565e30119bf860e5932c9230046e9f9a79595b7f67b7c9743264ef809ee52d9ee9bc80a1ab6999a905d7642db4e0f48fdcf0b
  HEAD_REF rpm-4.18.2-release
)

vcpkg_replace_string("${SOURCE_PATH}/configure.ac"
  "RPMCONFIGDIR=\"`echo \${usrprefix}/lib/rpm`\""
  "RPMCONFIGDIR=\"/usr/lib/rpm\""
)

# RPM looks for "popt" but Vcpkg generates a poptd library file
set(COMMON_PATH "${CURRENT_INSTALLED_DIR}/debug/lib/${VCPKG_TARGET_STATIC_LIBRARY_PREFIX}")
set(LIBPOPTD_FILE "${COMMON_PATH}poptd${VCPKG_TARGET_STATIC_LIBRARY_SUFFIX}/")
set(LIBPOPT_FILE "${COMMON_PATH}popt${VCPKG_TARGET_STATIC_LIBRARY_SUFFIX}")
file(COPY ${LIBPOPTD_FILE} DESTINATION ${LIBPOPT_FILE})

vcpkg_configure_make(
  SOURCE_PATH "${SOURCE_PATH}"
  OPTIONS
  "--with-crypto=openssl"
  "--enable-sqlite=yes"
  "--disable-openmp"
  "--disable-plugins"
  "--enable-ndb"
  "--enable-bdb-ro"
)

vcpkg_install_make()
vcpkg_fixup_pkgconfig()
vcpkg_copy_pdbs()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share"
                    "${CURRENT_PACKAGES_DIR}/debug/lib/rpm-plugins"
                    "${CURRENT_PACKAGES_DIR}/debug/lib/rpm/lua"
                    "${CURRENT_PACKAGES_DIR}/debug/lib/rpm/macros.d"
                    "${CURRENT_PACKAGES_DIR}/debug/var/tmp"
                    "${CURRENT_PACKAGES_DIR}/lib/rpm-plugins"
                    "${CURRENT_PACKAGES_DIR}/lib/rpm/lua"
                    "${CURRENT_PACKAGES_DIR}/lib/rpm/macros.d"
                    "${CURRENT_PACKAGES_DIR}/var/tmp"
                    "${CURRENT_PACKAGES_DIR}/debug/var"
                    "${CURRENT_PACKAGES_DIR}/var")

set(VCPKG_POLICY_SKIP_COPYRIGHT_CHECK enabled)
