LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := lib_a
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/a.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := lib_b
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/b.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := lib_c
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/c.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := lib_d
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/d.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := lib_e
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/e.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := lib_f
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/f.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := lib_g
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/g.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := lib_h
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/h.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := lib_i
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/i.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libgotye
LOCAL_WHOLE_STATIC_LIBRARIES :=  lib_a lib_b lib_c lib_d lib_e lib_f lib_g lib_h lib_i
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/j.a
include $(PREBUILT_STATIC_LIBRARY)

