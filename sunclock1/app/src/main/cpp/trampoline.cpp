//
// Created by gorilla on 4/5/2022.
//

#include <jni.h>

extern "C" {
#include "clock.h"
}

JNIEXPORT jintArray JNICALL Java_libnova_do_all(JNIEnv *env, jobject obj,
                                           jdouble lat, jdouble lng, jdouble offset) {
    Canvas *canvas = do_all(lat,lng,offset);
    jintArray ret = env->NewIntArray(2 + canvas->h * canvas->w);
    jint w = canvas->w;
    jint h = canvas->h;
    env->SetIntArrayRegion(ret, 0, 1, &w);
    env->SetIntArrayRegion(ret, 1, 1, &h);
    for (int i = 0; i < w * h; i++) {
        jint tmp = canvas->data[i];
        env->SetIntArrayRegion(ret, 2 + i, 1, &tmp);
    }
    return ret;
}