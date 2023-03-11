#pragma GCC optimize("Ofast")
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
JNIEXPORT jint JNICALL
Java_com_gorillasapiens_sunclock1_AlarmStorage_doWhenIsIt(JNIEnv *env, jobject thiz, jdouble lat,
                        jdouble lon, jint category, jint type, jint delayMinutes) {
    return do_when_is_it(lat, lon, category, type, delayMinutes);
}

extern "C"
JNIEXPORT jintArray JNICALL
Java_com_gorillasapiens_sunclock1_MainActivity_doAll(JNIEnv *env,
                     jobject thiz,
                     jdouble lat,
                     jdouble lon,
                     jdouble offset,
                     jint width,
                     jstring provider,
                     jstring tzprovider,
                     jstring tz,
                     jint lightdark) {
    jboolean garbage = false;
    const char *ccProvider = env->GetStringUTFChars(provider, &garbage);
    garbage = false;
    const char *ccTz = env->GetStringUTFChars(tz, &garbage);
    garbage = false;
    const char *ccTzProvider = env->GetStringUTFChars(tzprovider, &garbage);
    Canvas *canvas = do_all(lat, lon, offset,
                            width, ccProvider, ccTzProvider, ccTz,
                            lightdark);
    env->ReleaseStringUTFChars(tzprovider, ccTzProvider);
    env->ReleaseStringUTFChars(tz, ccTz);
    env->ReleaseStringUTFChars(provider, ccProvider);
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
Java_com_gorillasapiens_sunclock1_MainActivity_doGlobe(JNIEnv *env, jobject thiz, jdouble lat,
                                                       jdouble lon,
                                                       jdouble spin,
                                                       jint width,
                                                       jstring tzname) {
    jboolean garbage = false;
    const char *str = env->GetStringUTFChars(tzname, &garbage);
    Canvas *canvas = do_globe(lat, lon, spin, width, str);
    env->ReleaseStringUTFChars(tzname, str);
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
