//
// Created by gorilla on 4/5/2022.
//

#include <jni.h>

extern "C" {
#include "clock.h"
#include "draw.h"
#include "globe.h"
}

extern "C"
JNIEXPORT jintArray JNICALL
Java_com_gorillasapiens_sunclock1_MainActivity_do_1all(JNIEnv *env, jobject thiz, jdouble lat,
                                                          jdouble lng, jdouble offset,
                                                          jint width) {
    Canvas *canvas = do_all(lat, lng, offset, width);
    jintArray ret = env->NewIntArray(2 + canvas->h * canvas->w);
    jint w = canvas->w;
    jint h = canvas->h;
    env->SetIntArrayRegion(ret, 0, 1, &w);
    env->SetIntArrayRegion(ret, 1, 1, &h);
    for (int i = 0; i < w * h; i++) {
        jint tmp = canvas->data[i];
        env->SetIntArrayRegion(ret, 2 + i, 1, &tmp);
    }
    delete_canvas(canvas);
    return ret;
}

extern "C"
JNIEXPORT jintArray JNICALL
Java_com_gorillasapiens_sunclock1_MainActivity_do_1globe(JNIEnv *env, jobject thiz, jdouble lat,
                                                       jdouble lng,
                                                       jint width) {
    Canvas *canvas = do_globe(lat, lng, width);
    jintArray ret = env->NewIntArray(2 + canvas->h * canvas->w);
    jint w = canvas->w;
    jint h = canvas->h;
    env->SetIntArrayRegion(ret, 0, 1, &w);
    env->SetIntArrayRegion(ret, 1, 1, &h);
    for (int i = 0; i < w * h; i++) {
        jint tmp = canvas->data[i];
        env->SetIntArrayRegion(ret, 2 + i, 1, &tmp);
    }
    delete_canvas(canvas);
    return ret;
}