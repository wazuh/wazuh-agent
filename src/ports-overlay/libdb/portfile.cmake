vcpkg_from_github(
    OUT_SOURCE_PATH
    SOURCE_PATH
    REPO
    yasuhirokimura/db18
    REF
    40d05f5ef418119213b84a6355165b330d030b1b
    SHA512
    a560253317bbd7d8f7efc682cb71873efb6c4a3fd17c69b7c1db16a567d4e7a99342b859855b8599b2ae72e96c48bee35221ffbd93f3aa6e0f256c8ec493cbbd
    HEAD_REF
    master)

vcpkg_configure_make(
    SOURCE_PATH
    "${SOURCE_PATH}/dist"
    OPTIONS
    ${OPTIONS}
    --with-cryptography=no
    --disable-queue
    --disable-heap
    --disable-partition
    --disable-mutexsupport
    --disable-replication
    --disable-verify
    --disable-statistics
    ac_cv_func_pthread_yield=no)

vcpkg_install_make()

vcpkg_copy_pdbs()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")
set(VCPKG_POLICY_SKIP_COPYRIGHT_CHECK enabled)
