vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO rpm-software-management/rpm
  REF 9fc611e38004b1ddaa6668297408649ed556c157
  SHA512 847e55ec10462df517094385c8a565e30119bf860e5932c9230046e9f9a79595b7f67b7c9743264ef809ee52d9ee9bc80a1ab6999a905d7642db4e0f48fdcf0b
  HEAD_REF rpm-4.18.2-release
)

vcpkg_configure_make(
  SOURCE_PATH "${SOURCE_PATH}"
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
