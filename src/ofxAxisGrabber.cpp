/*
 * ofxAxisGrabber.cpp
 *
 *  Created on: Sep 23, 2012
 *      Author: arturo
 */

#include "ofxAxisGrabber.h"
#include "Poco/DOM/DOMParser.h"
#include "Poco/DOM/Document.h"
#include "Poco/DOM/NodeList.h"
#include "Poco/DOM/NamedNodeMap.h"
#include "ofAppRunner.h"

ofxAxisGrabber::ofxAxisGrabber() {
	paramRefreshRateMs = 3000;
	desiredFramerate = 0;
	startTimeOneSecMicros = 0;
	framesInOneSec = 0;
	codec = H264;
	protocol = HTTP;
	updating = false;
	started = false;

	parameters.add(focus.set("focus",100,0,297));
	parameters.add(manualIris.set("manualIris",true));
	parameters.add(exposure.set("exposure",50,0,100));
	parameters.add(irFilterCut.set("irFilterCut",0,0,2));
	parameters.add(compression.set("compression",30,0,100));
	parameters.add(paramRefreshRateMs.set("paramRefreshRateMs",3000,0,10000));
	parameters.add(fps.set("fps",0,0,60));
	parameters.add(cameraConnected.set("cameraConnected",false));
	parameters.add(cameraAuth.set("cameraAuth",false));
	prevFocus = focus;

	compression.addListener(this,&ofxAxisGrabber::compressionChanged);
	exposure.addListener(this,&ofxAxisGrabber::exposureChanged);
	irFilterCut.addListener(this,&ofxAxisGrabber::irFilterCutChanged);
	manualIris.addListener(this,&ofxAxisGrabber::setManualIris);
	focus.addListener(this,&ofxAxisGrabber::focusChanged);
}

//overwritten from ofBaseVideoGrabber
std::vector<ofVideoDevice> ofxAxisGrabber::listDevices(){
	return std::vector<ofVideoDevice>();
}


void ofxAxisGrabber::setCodec(ofxAxisGrabber::AxisCodec _codec){
	codec = _codec;
}

void ofxAxisGrabber::setCameraAddress(string address){
	string parametersName = address;
	ofStringReplace(parametersName,".","_");
	parameters.setName(parametersName);
	cameraAddress = address;
}

string ofxAxisGrabber::getCameraAddress(){
	return cameraAddress;
}

void ofxAxisGrabber::setCompression(int _compression){
	compression = _compression;
}

bool ofxAxisGrabber::initGrabber(int w, int h){
	startTimeOneSecMicros = 0;
	framesInOneSec=0;
	fps=0;

	if(cameraAddress==""){
		ofLogError() << "cannot init axis grabber, must call setCameraAddress before initGrabber" << endl;
	}

	string codecStr = codec==MJPG?"mjpg":"h264";
	stringstream url;
	if(protocol==RTSP){
		url << "rtsp://" << cameraAddress <<
				"/axis-media/media.amp?";
	}else{
		url << "http://" << cameraAddress <<
				"/axis-cgi/mjpg/video.cgi?";
	}
	url << "videocodec=" << codecStr <<
	"&resolution=" << w << "x" << h <<
	"&compression=" << compression <<
	"&mirror=0"
	"&rotation=0"
	"&textposition=top"
	"&textbackgroundcolor=black"
	"&textcolor=white"
	"&text=0"
	"&clock=0"
	"&date=0"
	"&overlayimage=0"
	"&fps=" << desiredFramerate <<
	"&audio=0"
	"&keyframe_interval=8"
	"&videobitrate=0"
	"&maxframesize=0";

	stringstream pipeline;
	if(protocol==HTTP){
		pipeline << "souphttpsrc location=\""<< url.str() << "\"";
	}else{
		pipeline << "rtspsrc location=\""<< url.str() << "\" latency=0";
	}
	pipeline <<" ! decodebin ! videoconvert ! videoscale ";

	ofLogNotice("ofxAxisGrabber") << "pipeline: ";
	ofLogNotice("ofxAxisGrabber") << pipeline.str();
	bool ret = gst.setPipeline(pipeline.str(),24,true,w,h);
//#if OF_VERSION_MINOR>8 || (OF_VERSION_MINOR==0 && OF_VERSION_PATCH>0)
	ret = gst.startPipeline();
//#endif

	//gst.loadMovie("http://10.42.0.23/axis-cgi/mjpg/video.cgi?fps=30&nbrofframes=0&resolution=640x480");
	//gst.loadMovie("rtsp://10.42.0.23:554/axis-media/media.amp?videocodec=h264");
	//player.loadMovie("rtsp://10.42.0.23:554/axis-media/media.amp?videocodec=h264&resolution="+ofToString(w)+"x"+ofToString(h)+"&compression=10&mirror=0&rotation=180&textposition=top&textbackgroundcolor=black&textcolor=white&text=0&clock=0&date=0&overlayimage=0&fps=0&audio=0&keyframe_interval=32&videobitrate=0&maxframesize=0");
	cameraConnected = ret;
	if(ret) {
		gst.play();

		http.setTimeoutSeconds(30);
		http.start();
		if(!getPocoThread().isRunning()) startThread();
	}else{
		ofLogError("ofxAxisGrabber") << "couldn't allocate gstreamer";
	}

	started = ret;
	return ret;
}

void ofxAxisGrabber::newResponse(ofxHttpResponse & response){
	if(response.status==-1){
		cameraConnected = false;
		http.clearQueue();
	}
}

void ofxAxisGrabber::focusChanged(int & focus){
	if(updating || cameraAddress=="") return;
	float step = float(focus - prevFocus)/297.;
	http.addUrl("http://"+cameraAddress+"/axis-cgi/opticssetup.cgi?rfocus="+ofToString(step)+"&timestamp="+ofToString(ofGetSystemTime()));
	prevFocus = focus;
}


string ofxAxisGrabber::getStrPropertyValue(string property){
	if(cameraAddress=="") return "";
	ofxHttpResponse response = http.getUrl("http://"+cameraAddress+"/axis-cgi/param.cgi?action=list&group="+property+"&timestamp="+ofToString(ofGetSystemTime()));
	if(response.status==200){
		return string(response.responseBody).substr(string(property).size()+1);
	}else{
		return "";
	}
}

int ofxAxisGrabber::getIntPropertyValue(string property){
	if(cameraAddress=="") return 0;
	string strValue = getStrPropertyValue(property);
	if(strValue!=""){
		return ofToInt(strValue);
	}else{
		return 0;
	}
}

float ofxAxisGrabber::getFloatPropertyValue(string property){
	if(cameraAddress=="") return 0;
	string strValue = getStrPropertyValue(property);
	if(strValue!=""){
		return ofToFloat(strValue);
	}else{
		return 0;
	}
}

template<typename T>
void ofxAxisGrabber::setPropertyValue(string property, T & value){
	if(cameraAddress=="") return;
	http.addUrl("http://"+cameraAddress+"/axis-cgi/param.cgi?action=update&"+property+"="+ofToString(value)+"&timestamp="+ofToString(ofGetSystemTime()));
}

void ofxAxisGrabber::exposureChanged(int & exposure){
	if(!updating)
		setPropertyValue("ImageSource.I0.Sensor.ExposureValue",exposure);
}

void ofxAxisGrabber::checkExposure(){
	if(cameraAddress=="") return;
	exposure = getIntPropertyValue("ImageSource.I0.Sensor.ExposureValue");
}

void ofxAxisGrabber::irFilterCutChanged(int & irFilterCut){
	if(!updating){
		switch(irFilterCut){
		case 0:
			setPropertyValue("ImageSource.I0.DayNight.IrCutFilter","auto");
			break;
		case 1:
			setPropertyValue("ImageSource.I0.DayNight.IrCutFilter","yes");
			break;
		case 2:
			setPropertyValue("ImageSource.I0.DayNight.IrCutFilter","no");
			break;
		}
	}
}

void ofxAxisGrabber::checkIRFilterCut(){
	string strIRFilter = getStrPropertyValue("ImageSource.I0.DayNight.IrCutFilter");
	if(strIRFilter=="auto"){
		irFilterCut=0;
	}else if(strIRFilter=="yes"){
		irFilterCut=1;
	}if(strIRFilter=="no"){
		irFilterCut=2;
	}
}

void ofxAxisGrabber::compressionChanged(int & compression){
	if(!updating)
		setPropertyValue("Image.I0.Appearance.Compression",compression);
}

void ofxAxisGrabber::checkCompression(){
	compression = getIntPropertyValue("Image.I0.Appearance.Compression");
}

void ofxAxisGrabber::requestFocusWindow(){
	ofxHttpResponse response = http.getUrl("http://"+cameraAddress+"/axis-cgi/param.cgi?action=list&group=ImageSource.I0.Focus.Window.W0&timestamp="+ofToString(ofGetSystemTime()));
	if(response.status==200){
		string focusWindowStr = response.responseBody;
		focusWindowStr = focusWindowStr.substr(string("ImageSource.I0.Focus.Window.W0=").size());
		vector<string> coords = ofSplitString(focusWindowStr,":");

		focusWindow.x = ofToFloat(ofSplitString(coords[0],",")[0]);
		focusWindow.y = ofToFloat(ofSplitString(coords[0],",")[1]);
		focusWindow.width = -(ofToFloat(ofSplitString(coords[1],",")[0])-focusWindow.x);
		focusWindow.height = -(ofToFloat(ofSplitString(coords[2],",")[1])-focusWindow.y);
		focusWindow.y = 1-focusWindow.y;
		focusWindow.x = 1-focusWindow.x;

		focusWindowScaled = focusWindow;
		focusWindowScaled.x*=gst.getWidth();
		focusWindowScaled.y*=gst.getHeight();
		focusWindowScaled.width*=gst.getWidth();
		focusWindowScaled.height*=gst.getHeight();
	}
}

void ofxAxisGrabber::setManualIris(bool & manual){
	http.addUrl("http://"+cameraAddress+"//axis-cgi/irissetup.cgi?automatic=" + string(manual?"no":"yes") + "&timestamp="+ofToString(ofGetSystemTime()));
}

ofRectangle & ofxAxisGrabber::getFocusWindow(){
	return focusWindowScaled;
}

void ofxAxisGrabber::setFocusWindow(const ofRectangle & focusWindow){
	focusWindowScaled = focusWindow;
	setFocusWindow();
}

void ofxAxisGrabber::setFocusWindow(){
	if(gst.getWidth()>0 && gst.getHeight()>0){
		string focusWindowStr = ofToString(1-focusWindowScaled.x/gst.getWidth()) + "," + ofToString(1-focusWindowScaled.y/gst.getHeight()) + ":" +
				ofToString(1-(focusWindowScaled.x + focusWindowScaled.width)/gst.getWidth()) + "," + ofToString(1-focusWindowScaled.y/gst.getHeight()) + ":" +
				ofToString(1-(focusWindowScaled.x + focusWindowScaled.width)/gst.getWidth()) + "," + ofToString(1-(focusWindowScaled.y+focusWindowScaled.height)/gst.getHeight()) + ":" +
				ofToString(1-focusWindowScaled.x/gst.getWidth()) + "," + ofToString(1-(focusWindowScaled.y+focusWindowScaled.height)/gst.getHeight());


		http.getUrl("http://"+cameraAddress+"/axis-cgi/param.cgi?action=update&ImageSource.I0.Focus.Window.W0="+focusWindowStr+"&timestamp="+ofToString(ofGetSystemTime()));
		requestFocusWindow();
		triggerAutoFocus();
	}
}

void ofxAxisGrabber::threadedFunction(){
	while(isThreadRunning()){
		if(http.getQueueLength()==0){
			u_long now = ofGetElapsedTimeMillis();
			updating = true;
			ofxHttpResponse response = http.getUrl("http://"+cameraAddress+"/axis-cgi/opticssetup.cgi?monitor=poll&timestamp="+ofToString(ofGetSystemTime()));
			if(response.status==200){
				cameraConnected = true;
				cameraAuth = true;
				Poco::XML::DOMParser parser;
				Poco::XML::Document * xml = parser.parseString(response.responseBody);
				focus = round(ofToFloat(xml->getElementsByTagName("opticsSetupState")->item(0)->attributes()->getNamedItem("focusPosition")->nodeValue())*297);
				prevFocus = focus;
				xml->getElementsByTagName("opticsSetupState")->item(0)->attributes()->getNamedItem("focusOperationInProgress")->nodeValue();
				focusMeasure = ofToInt(xml->getElementsByTagName("opticsSetupState")->item(0)->attributes()->getNamedItem("focusMeasure")->nodeValue());
				xml->getElementsByTagName("opticsSetupState")->item(0)->attributes()->getNamedItem("focusAssistantRunning")->nodeValue();
			}else{
				if(response.status==-1){
					cameraConnected = false;
				}else if(response.status==401){
					cameraAuth = false;
				}
				ofLogError("ofxAxisGrabber")<< "couldn't update parameters: " << response.status << ": " << response.reasonForStatus;
			}
			requestFocusWindow();
			checkExposure();
			checkIRFilterCut();
			checkCompression();
			updating = false;
		}
		ofSleepMillis(paramRefreshRateMs);
	}
}

void ofxAxisGrabber::update(){
	gst.update();
	if(gst.isFrameNew()){
		framesInOneSec++;
		u_long now = ofGetElapsedTimeMicros();
		double oneSecTime = now-startTimeOneSecMicros;
		fps = double(framesInOneSec*1000000)/oneSecTime;
		if(oneSecTime>=1000000){
			startTimeOneSecMicros = now;
			framesInOneSec = 0;
		}
	}
	if(!getPocoThread().isRunning()) startThread();
}

bool ofxAxisGrabber::isFrameNew(){
	return gst.isFrameNew();
}

unsigned char * ofxAxisGrabber::getPixels(){
	return gst.getPixels();
}

ofPixels_<unsigned char> & ofxAxisGrabber::getPixelsRef(){
	return gst.getPixelsRef();
}

void ofxAxisGrabber::close(){
	if(started){
		gst.close();
		http.stop();
		stopThread();
		started = false;
	}
}

float ofxAxisGrabber::getHeight(){
	return gst.getHeight();
}

float ofxAxisGrabber::getWidth(){
	return gst.getWidth();
}

bool ofxAxisGrabber::setPixelFormat(ofPixelFormat pixelFormat){
	return false;
}

ofPixelFormat ofxAxisGrabber::getPixelFormat(){
	return OF_PIXELS_RGB;
}

void ofxAxisGrabber::setVerbose(bool bTalkToMe){

}

void ofxAxisGrabber::setDeviceID(int _deviceID){

}

void ofxAxisGrabber::setDesiredFrameRate(int framerate){
	desiredFramerate = framerate;
}

void ofxAxisGrabber::videoSettings(){

}


// axisgrabber only
void ofxAxisGrabber::triggerAutoFocus(){
	http.addUrl("http://"+cameraAddress+"/axis-cgi/opticssetup.cgi?autofocus=perform&timestamp="+ofToString(ofGetSystemTime()));
}

void ofxAxisGrabber::setAuth(string user, string pwd){
	http.setBasicAuthentication(user,pwd);
}

void ofxAxisGrabber::setParametersRefreshRate(int ms){
	paramRefreshRateMs = ms;
}

ofGstVideoUtils & ofxAxisGrabber::getGstUtils(){
	return gst;
}
