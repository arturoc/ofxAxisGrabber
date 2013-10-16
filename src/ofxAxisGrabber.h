/*
 * ofxAxisGrabber.h
 *
 *  Created on: Sep 23, 2012
 *      Author: arturo
 */

#ifndef OFXAXISGRABBER_H_
#define OFXAXISGRABBER_H_

#include "ofBaseTypes.h"
#include "ofParameter.h"
#include "ofParameterGroup.h"
#include "ofxHttpUtils.h"
#include "ofGstUtils.h"

class ofxAxisGrabber: public ofBaseVideoGrabber, public ofThread {
public:
	ofxAxisGrabber();

	//overwritten from ofBaseVideoGrabber
	void	listDevices();
	bool	initGrabber(int w, int h);
	void	update();
	bool	isFrameNew();

	unsigned char 	* getPixels();
	ofPixels_<unsigned char> & getPixelsRef();

	void	close();

	float	getHeight();
	float	getWidth();

	bool setPixelFormat(ofPixelFormat pixelFormat);
	ofPixelFormat getPixelFormat();

	void setVerbose(bool bTalkToMe);
	void setDeviceID(int _deviceID);
	void setDesiredFrameRate(int framerate);
	void videoSettings();


	// axisgrabber only
	enum AxisCodec{
		MJPG,
		H264
	};
	void triggerAutoFocus();
	void setAuth(string user, string pwd); // without authentication parameter changing won't work
	void setParametersRefreshRate(int ms);  //default 0 no refresh
	void setCodec(AxisCodec codec);
	void setCompression(int compression=30);
	void setCameraAddress(string address);
	string getCameraAddress();
	ofRectangle & getFocusWindow();
	void setFocusWindow(const ofRectangle & focusWindow);

	//checks available properties from the camera with http://camera_address/axis-cgi/param.cgi?action=list
	string getStrPropertyValue(string property);
	int getIntPropertyValue(string property);
	float getFloatPropertyValue(string property);
	template<typename T>
	void setPropertyValue(string property, T & value);

	ofGstVideoUtils & getGstUtils();

	ofParameter<int> focus;
	ofParameter<int> focusMeasure;
	ofParameter<bool> manualIris;
	ofParameter<float> fps;
	ofParameter<bool> cameraConnected;
	ofParameter<bool> cameraAuth;
	ofParameter<int> exposure;
	ofParameter<int> irFilterCut;
	ofParameter<int> compression;
	ofParameter<int>  paramRefreshRateMs;
	ofParameterGroup parameters;

private:
	void focusChanged(int & focus);
	void setManualIris(bool & manual);
	void exposureChanged(int & exposure);
	void irFilterCutChanged(int & irFilterCut);
	void compressionChanged(int & compression);

	void checkExposure();
	void requestFocusWindow();
	void checkIRFilterCut();
	void checkCompression();
	void setFocusWindow();
	void threadedFunction();
	void newResponse(ofxHttpResponse & response);

	ofGstVideoUtils gst;
	int prevFocus;
	bool updating;

	ofxHttpUtils http;

	ofRectangle focusWindow;
	ofRectangle focusWindowScaled;

	int startTimeOneSecMicros;
	int framesInOneSec;
	string user,pwd;
	int desiredFramerate;
	AxisCodec codec;

	string cameraAddress;

	bool started;
};

#endif /* OFXAXISGRABBER_H_ */
