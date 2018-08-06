/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "camera_preview/mv_recording_preview_controller.h"
#include <sys/types.h>
#include <android/asset_manager_jni.h>
#include <android/asset_manager.h>
/* Header for class com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler */

#ifndef _Included_com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler
#define _Included_com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler
 * Method:    switchPreviewFilter
 * Signature: (ILjava/lang/Object;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_switchPreviewFilter
  (JNIEnv *, jobject, jint, jobject, jstring);

JNIEXPORT void JNICALL Java_com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_adaptiveVideoQuality
  (JNIEnv *, jobject, jint, jint, jint);

JNIEXPORT void JNICALL
Java_com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_hotConfigQuality(
        JNIEnv *env, jobject instance,jint max, jint avg, jint fps);

/*
 * Class:     com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler
 * Method:    switchCameraFacing
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_switchCameraFacing
  (JNIEnv *, jobject);

JNIEXPORT void JNICALL Java_com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_startEncoding
  (JNIEnv *, jobject, jint, jint, jint, jint, jboolean, jint);

JNIEXPORT void JNICALL Java_com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_stopEncoding
  (JNIEnv *, jobject);
/*
 * Class:     com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler
 * Method:    prepareEGLContext
 * Signature: (Ljava/lang/Object;II)V
 */
JNIEXPORT void JNICALL Java_com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_prepareEGLContext
  (JNIEnv *, jobject, jobject, jint, jint, jint);

/*
 * Class:     com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler
 * Method:    resetRenderSize
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_resetRenderSize
  (JNIEnv *, jobject, jint, jint);

/*
 * Class:     com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler
 * Method:    destroyEGLContext
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_destroyEGLContext
  (JNIEnv *, jobject);

/*
 * Class:     com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler
 * Method:    notifyFrameAvailable
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_notifyFrameAvailable
  (JNIEnv *, jobject);

JNIEXPORT void JNICALL Java_com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_updateTexMatrix
  (JNIEnv *, jobject, jfloatArray);

JNIEXPORT void JNICALL Java_com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_switchPauseRecordingPreviewState
  (JNIEnv* env, jobject obj);

JNIEXPORT void JNICALL Java_com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_switchCommonPreviewState
  (JNIEnv* env, jobject obj);

JNIEXPORT void JNICALL Java_com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_createWindowSurface
  (JNIEnv*, jobject, jobject);

JNIEXPORT void JNICALL Java_com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_destroyWindowSurface
  (JNIEnv*, jobject);

JNIEXPORT void JNICALL Java_com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_setBeautifyParam(JNIEnv* env, jobject, jint, jfloat);
JNIEXPORT void JNICALL Java_com_zx_androidffmpegrecoder_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_hotConfig
        (JNIEnv *env, jobject obj, jint bitRate, jint fps, jint gopSize);

#ifdef __cplusplus
}
#endif
#endif
