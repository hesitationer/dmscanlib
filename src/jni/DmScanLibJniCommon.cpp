/*
 * Contains JNI code common to both MS Windows and Linux.
 */

#include "DmScanLibJni.h"
#include "DmScanLibJniInternal.h"
#include "DmScanLib.h"
#include "geometry.h"
#include "decoder/DecodeOptions.h"
#include "decoder/WellDecoder.h"

#include <iostream>
#include <vector>
#include <memory>
#include <glog/logging.h>

using namespace dmscanlib;

jobject createScanResultObject(JNIEnv * env, int resultCode, int value) {
    jclass scanLibResultClass = env->FindClass(
                    "edu/ualberta/med/scannerconfig/dmscanlib/ScanLibResult");

    // run the following command to obtain method signatures from a class.
    // javap -s -p edu.ualberta.med.scannerconfig.dmscanlib.ScanLibResult
    jmethodID cons = env->GetMethodID(scanLibResultClass, "<init>",
                                      "(IILjava/lang/String;)V");

    std::string msg;
    getResultCodeMsg(resultCode, msg);

    jvalue data[3];
    data[0].i = resultCode;
    data[1].i = value;
    data[2].l = env->NewStringUTF(msg.c_str());

    return env->NewObjectA(scanLibResultClass, cons, data);
}

jobject createDecodeResultObject(JNIEnv * env, int resultCode) {
    jclass resultClass = env->FindClass(
                    "edu/ualberta/med/scannerconfig/dmscanlib/DecodeResult");

    // run the following command to obtain method signatures from a class.
    // javap -s -p edu.ualberta.med.scannerconfig.dmscanlib.DecodeResult
    jmethodID cons = env->GetMethodID(resultClass, "<init>",
                                      "(IILjava/lang/String;)V");

    std::string msg;
    getResultCodeMsg(resultCode, msg);

    jvalue data[3];
    data[0].i = resultCode;
    data[1].i = 0;
    data[2].l = env->NewStringUTF(msg.c_str());

    return env->NewObjectA(resultClass, cons, data);
}

jobject createDecodeResultObject(JNIEnv * env, int resultCode,
                const std::vector<dmscanlib::WellDecoder *> & wellDecoders) {
    jobject resultObj = createDecodeResultObject(env, resultCode);

    jclass resultClass = env->FindClass(
                    "edu/ualberta/med/scannerconfig/dmscanlib/DecodeResult");

    if (wellDecoders.size() > 0) {
        jmethodID setCellMethod = env->GetMethodID(resultClass, "addWell",
        		"(Ljava/lang/String;Ljava/lang/String;)V");

        for (unsigned i = 0, n = wellDecoders.size(); i < n; ++i) {
        	dmscanlib::WellDecoder & wellDecoder = *wellDecoders[i];
            jvalue data[3];

            data[0].l = env->NewStringUTF(wellDecoder.getLabel().c_str());
            data[1].l = env->NewStringUTF(wellDecoder.getMessage().c_str());

            env->CallObjectMethodA(resultObj, setCellMethod, data);
        }
    }

    return resultObj;
}

/*
 * Class:     edu_ualberta_med_scannerconfig_dmscanlib_ScanLib
 * Method:    decodeImage
 * Signature: (JLjava/lang/String;Ledu/ualberta/med/scannerconfig/dmscanlib/DecodeOptions;[Ledu/ualberta/med/scannerconfig/dmscanlib/Well;)Ledu/ualberta/med/scannerconfig/dmscanlib/DecodeResult;
 */
JNIEXPORT jobject JNICALL Java_edu_ualberta_med_scannerconfig_dmscanlib_ScanLib_decodeImage(
		JNIEnv * env, jobject obj, jlong _verbose, jstring _filename,
		jobject _decodeOptions, jobjectArray _wellRects) {

	if ((_verbose == 0) || (_filename == 0) || (_decodeOptions == 0)
			|| (_wellRects == 0)) {
		return createDecodeResultObject(env, SC_FAIL);
	}

    DmScanLib::configLogging(static_cast<unsigned>(_verbose), false);

    const char *filename = env->GetStringUTFChars(_filename, 0);

    DecodeOptions decodeOptions(env, _decodeOptions);

    std::vector<std::unique_ptr<WellRectangle<double>  > > wellRects;

    jobject wellRectJavaObj;
    jclass wellRectJavaClass = NULL;
    jmethodID wellRectGetLabelMethodID = NULL;
    jmethodID wellRectGetCornerXMethodID = NULL;
    jmethodID wellRectGetCornerYMethodID = NULL;

    jsize numWells = env->GetArrayLength(_wellRects);

    bool invalidWellRectFound = false;

	VLOG(3) << "decodeImage: numWells/" << numWells;

	// TODO check for max well rectangle objects
    for (int i = 0; i < static_cast<int>(numWells); ++i) {
    	wellRectJavaObj = env->GetObjectArrayElement(_wellRects, i);

    	// if java object pointer is null, skip this array element
    	if (wellRectJavaObj == NULL) {
    		invalidWellRectFound = true;
        	break;
    	}

    	if (wellRectJavaClass == NULL) {
    		wellRectJavaClass = env->GetObjectClass(wellRectJavaObj);

    		wellRectGetLabelMethodID = env->GetMethodID(wellRectJavaClass,
    	    		"getLabel", "()Ljava/lang/String;");
    	    if(env->ExceptionOccurred()) {
    	    	return NULL;
    	    }

    	    wellRectGetCornerXMethodID = env->GetMethodID(wellRectJavaClass,
    	    		"getCornerX", "(I)D");
    	    if(env->ExceptionOccurred()) {
    	    	return NULL;
    	    }

    	    wellRectGetCornerYMethodID = env->GetMethodID(wellRectJavaClass,
    	    		"getCornerY", "(I)D");
    	    if(env->ExceptionOccurred()) {
    	    	return NULL;
    	    }
    	}

    	jobject labelJobj = env->CallObjectMethod(wellRectJavaObj, wellRectGetLabelMethodID);
    	const char * label = env->GetStringUTFChars((jstring) labelJobj, NULL);

    	double x1 = env->CallDoubleMethod(wellRectJavaObj, wellRectGetCornerXMethodID, 0);
    	double y1 = env->CallDoubleMethod(wellRectJavaObj, wellRectGetCornerYMethodID, 0);

    	double x2 = env->CallDoubleMethod(wellRectJavaObj, wellRectGetCornerXMethodID, 1);
    	double y2 = env->CallDoubleMethod(wellRectJavaObj, wellRectGetCornerYMethodID, 1);

    	double x3 = env->CallDoubleMethod(wellRectJavaObj, wellRectGetCornerXMethodID, 2);
    	double y3 = env->CallDoubleMethod(wellRectJavaObj, wellRectGetCornerYMethodID, 2);

    	double x4 = env->CallDoubleMethod(wellRectJavaObj, wellRectGetCornerXMethodID, 3);
    	double y4 = env->CallDoubleMethod(wellRectJavaObj, wellRectGetCornerYMethodID, 3);

    	std::unique_ptr<WellRectangle<double> > wellRect(
    			new WellRectangle<double>(label, x1, y1, x2, y2, x3, y3, x4, y4));

    	VLOG(3) << *wellRect;

    	wellRects.push_back(std::move(wellRect));

    	env->ReleaseStringUTFChars((jstring) labelJobj, label);
    	env->DeleteLocalRef(wellRectJavaObj);
    }

    jobject resultObj = 0;

    if (invalidWellRectFound || (wellRects.size() == 0)) {
    	// invalid rects or zero rects passed from java
    	resultObj = createDecodeResultObject(env, SC_INVALID_NOTHING_TO_DECODE);
    } else {
        dmscanlib::DmScanLib dmScanLib(1);
        int result = dmScanLib.decodeImageWells(filename, decodeOptions, wellRects);

        if (result == SC_SUCCESS) {
        	resultObj = createDecodeResultObject(env,result, dmScanLib.getDecodedWells());
        } else {
        	resultObj = createDecodeResultObject(env, result);
        }
    }

    env->ReleaseStringUTFChars(_filename, filename);

    return resultObj;
}

