# SPDX-License-Identifier: GPL-2.0-or-later

ExternalProject_Add(external_ffi
  URL file://${PACKAGE_DIR}/${FFI_FILE}
  URL_HASH ${FFI_HASH_TYPE}=${FFI_HASH}
  DOWNLOAD_DIR ${DOWNLOAD_DIR}
  PREFIX ${BUILD_DIR}/ffi
  CONFIGURE_COMMAND ${CONFIGURE_ENV} && cd ${BUILD_DIR}/ffi/src/external_ffi/x86_64-pc-linux-gnu && ../${CONFIGURE_COMMAND} --prefix=${LIBDIR}/ffi
    --enable-shared=no
    --enable-static=yes
    --with-pic
    --libdir=${LIBDIR}/ffi/lib64/
  BINARY_DIR ${BUILD_DIR}/ffi/src/external_ffi/x86_64-pc-linux-gnu
  BUILD_COMMAND ${CONFIGURE_ENV} && cd ${BUILD_DIR}/ffi/src/external_ffi/x86_64-pc-linux-gnu && make -j${MAKE_THREADS}
  INSTALL_COMMAND ${CONFIGURE_ENV} && cd ${BUILD_DIR}/ffi/src/external_ffi/x86_64-pc-linux-gnu && make install
  PATCH_COMMAND ${PATCH_CMD} -p 0 -d ${BUILD_DIR}/ffi/src/external_ffi < ${PATCH_DIR}/ffi.diff
  INSTALL_DIR ${LIBDIR}/ffi
)

if(UNIX AND NOT APPLE)
  ExternalProject_Add_Step(external_ffi after_install
    COMMAND ${CMAKE_COMMAND} -E copy ${LIBDIR}/ffi/lib64/libffi.a ${LIBDIR}/ffi/lib/libffi_pic.a
    DEPENDEES install
  )
endif()
