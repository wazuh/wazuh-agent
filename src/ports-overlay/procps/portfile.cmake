vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO warmchang/procps
  REF fadce3f1b25b55d5f22be0d26db1ae2a53018255
  SHA512 c4efb9583be0e2272316b57ab9e574698d78a0bb6b2a84d4e5857e1fde9418e6fb1354892a1fba799a456e03eeb4b995487673d88d2d0b5b19087c6a812cd500
  HEAD_REF v3.3.0
  PATCHES 
    fix_glibc_collision.patch
)

vcpkg_configure_make(
  SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_install_make()

vcpkg_copy_pdbs()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")
set(VCPKG_POLICY_SKIP_COPYRIGHT_CHECK enabled)
