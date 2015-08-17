LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := gotyeapi_static
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libgotyeapi.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../include


LOCAL_WHOLE_STATIC_LIBRARIES := curl_static
LOCAL_WHOLE_STATIC_LIBRARIES += libgotye

include $(PREBUILT_STATIC_LIBRARY)

$(call import-add-path, $(LOCAL_PATH)/third)
$(call import-module,curl/prebuilt/android)
$(call import-module,gotye)
