/*
 * ofxAxisGui.h
 *
 *  Created on: Sep 26, 2012
 *      Author: arturo
 */

#ifndef OFXAXISGUI_H_
#define OFXAXISGUI_H_

#include "ofxAxisGrabber.h"
#include "ofVideoGrabber.h"
#include "ofxGui.h"

class ofxAxisGui {
public:
	ofxAxisGui();

	void setDrawGui(bool bDrawGui);
	void setAuth(string user, string pass);
	void setup(string cameraAddress,string cameraName,int x, int y);
	void update();
	void draw(float x, float y, float w, float h);


	ofxButton autofocus;
	ofParameter<bool> showFocusWindow;
	ofxButton changeIp;
	ofxButton reset;
	ofParameter<int> resolution;
	ofParameter<string> address;
	ofxGuiGroup gui;

	ofVideoGrabber grabber;
	ofPtr<ofxAxisGrabber> axis;

private:
	void resetCamera();
	void autofocusPressed();
	void changeIpPressed();
	void resolutionChanged(int & resolution);
	void resetPressed();

	void mousePressed(ofMouseEventArgs & mouse);
	void mouseReleased(ofMouseEventArgs & mouse);

	ofxLabel addressLabel;
	string cameraName;
	ofVec2f position;
	float w,h;
	bool settingFocusWindow;
	ofRectangle focusWindow;
	bool bDrawGui;
};

#endif /* OFXAXISGUI_H_ */
