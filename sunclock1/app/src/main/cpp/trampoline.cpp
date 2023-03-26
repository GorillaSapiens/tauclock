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
                     jint lightdark,
                     jobjectArray monthnames,
                     jobjectArray weekdaynames) {
    const char *ccProvider = env->GetStringUTFChars(provider, nullptr);
    const char *ccTz = env->GetStringUTFChars(tz, nullptr);
    const char *ccTzProvider = env->GetStringUTFChars(tzprovider, nullptr);

    const char *monam[12];
    //jsize mlen[12];
    jint nbrMElements = env->GetArrayLength(monthnames);
    for (int i = 0; i < nbrMElements && i < 12; i++) {
       jstring elem = (jstring) env->GetObjectArrayElement(monthnames, i);
       //mlen[i] = env->GetStringLength(elem);
       monam[i] = env->GetStringUTFChars(elem, nullptr);
    }

    const char *wenam[7];
    //jsize wlen[7];
    jint nbrWElements = env->GetArrayLength(weekdaynames);
    for (int i = 0; (i+1) < nbrWElements && i < 7; i++) {
       jstring elem = (jstring) env->GetObjectArrayElement(weekdaynames, i + 1);
       //wlen[i] = env->GetStringLength(elem);
       wenam[i] = env->GetStringUTFChars(elem, nullptr);
    }

    Canvas *canvas = do_all(lat, lon, offset,
                            width, ccProvider, ccTzProvider, ccTz,
                            lightdark, monam, wenam);


    for (int i = 0; i < nbrMElements && i < 12; i++) {
       jstring elem = (jstring) env->GetObjectArrayElement(monthnames, i);
       env->ReleaseStringUTFChars(elem, monam[i]);
    }

   for (int i = 0; (i + 1) < nbrWElements && i < 7; i++) {
      jstring elem = (jstring) env->GetObjectArrayElement(weekdaynames, i + 1);
      env->ReleaseStringUTFChars(elem, wenam[i]);
   }

    env->ReleaseStringUTFChars(tzprovider, ccTzProvider);
    env->ReleaseStringUTFChars(tz, ccTz);
    env->ReleaseStringUTFChars(provider, ccProvider);
    jintArray ret = env->NewIntArray(2 + canvas->h * canvas->w);
    jint w = canvas->w;
    jint h = canvas->h;
    env->SetIntArrayRegion(ret, 0, 1, &w);
    env->SetIntArrayRegion(ret, 1, 1, &h);
    for (int i = 0; i < w * h; i++) {
        jint tmp = canvas->data[i] | 0xFF000000;
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
    //for (int i = 0; i < w * h; i++) {
    //    jint tmp = canvas->data[i];
    //    env->SetIntArrayRegion(ret, 2 + i, 1, &tmp);
    //}
    env->SetIntArrayRegion(ret, 2, w * h, reinterpret_cast<const jint *>(canvas->data));
    delete_canvas(canvas);
    return ret;
}
