vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO warmchang/procps
  REF 21f6017b6156d2ed44ca3dcfdc2a82889218f696
  SHA512 13e8176aad44a89123c3e8b44e4f772fa05c45601e83b2b9180e592b40cf86f2d0faa3911ebdd87c74226ead5d5cad2322284e141ebbc147f41ccc89e3b1e183
  HEAD_REF master
  PATCHES
    set_cmake_project.patch
)

vcpkg_cmake_configure(
  SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

vcpkg_copy_pdbs()
