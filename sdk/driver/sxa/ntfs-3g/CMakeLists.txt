#SET(CMAKE_C_FLAGS_DEBUG   "-O1 ${CMAKE_C_FLAGS_DEBUG} -O1")
#SET(CMAKE_C_FLAGS_RELEASE "-O1 ${CMAKE_C_FLAGS_RELEASE} -O1")
#SET(CMAKE_C_FLAGS_DEBUG   "-O0 -g ${CMAKE_C_FLAGS_DEBUG} -O0 -g ")
#SET(CMAKE_C_FLAGS_RELEASE "-O0 -g ${CMAKE_C_FLAGS_RELEASE} -O0 -g ")

include_directories(
    ${SMEDIA_INC_DIRS}
	${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos
	${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/inc
	${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src
	${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/include/ntfs-3g
	${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/include/fuse-lite
	${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libfuse-lite
	${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g
	${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/src
	${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/port
    )


ADD_DEFINITIONS(
    -DXDEBUG
    -DHAVE_CONFIG_H
    -D__svr4__
    -DIGNORE_MTAB
    -D__BYTE_ORDER=__BIG_ENDIAN
    -D__BIG_ENDIAN=0x4321
    -D__LITTLE_ENDIAN=0x1234
    -DCONFIG_READDIR2=1
    -DCONFIG_HAVE_NTFSPOS=1
#   -DCONFIG_SAVENI=1
#   -DCONFIG_SXA_DEBUG_PERFORMANCE=1
#   -DCONFIG_SXA_DEBUG_MEMORY=1
    -DCONFIG_NTFS_DISABLE_RESET_LOGFILE=1
    -DXCONFIG_HIGHLEVEL)

add_library(
    libsxa_ntfs3g STATIC

    #=== source files =========================================#
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/src/fnmatch.c
#   ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/src/ntfs-3g.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/src/lowntfs-3g.c

    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libfuse-lite/fuse.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libfuse-lite/fusermount.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libfuse-lite/fuse_kern_chan.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libfuse-lite/fuse_loop.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libfuse-lite/fuse_lowlevel.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libfuse-lite/fuse_opt.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libfuse-lite/fuse_session.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libfuse-lite/fuse_signals.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libfuse-lite/helper.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libfuse-lite/mount.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libfuse-lite/mount_util.c

    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/acls.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/attrib.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/attrlist.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/bitmap.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/bootsect.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/cache.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/collate.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/compat.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/compress.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/debug.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/device.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/dir.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/efs.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/index.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/inode.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/lcnalloc.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/logfile.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/logging.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/mft.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/misc.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/mst.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/object_id.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/reparse.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/runlist.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/security.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/unistr.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/unix_io.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/volume.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/xattrs.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/libntfs-3g/freertos_io.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/port/port.c
    ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/port/sxa_ntfs.c


    #== header files (private) ================================#

    #== header files (public) =================================#
    #${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/inc/sxa.h
    )