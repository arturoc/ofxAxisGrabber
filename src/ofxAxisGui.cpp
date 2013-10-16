/*
 * ofxAxisGui.cpp
 *
 *  Created on: Sep 26, 2012
 *      Author: arturo
 */

#include "ofxAxisGui.h"
#include "ofSystemUtils.h"
#include "ofGraphics.h"

ofxAxisGui::ofxAxisGui() {
	axis = ofPtr<ofxAxisGrabber>(new ofxAxisGrabber);
	settingFocusWindow = false;
	bDrawGui = false;
	w=640, h=480;
}

void ofxAxisGui::setDrawGui(bool _bDrawGui){
	bDrawGui = _bDrawGui;
}

void ofxAxisGui::setAuth(string user, string pass){
	axis->setAuth(user, pass);
}

void ofxAxisGui::setup(string cameraAddress,string _cameraName,int x, int y){
	cameraName = _cameraName;
	settingFocusWindow = false;
	position.set(x,y);

	axis->setCameraAddress(cameraAddress);
	axis->setParametersRefreshRate(5000);
	axis->setDesiredFrameRate(30);
	axis->setCodec(ofxAxisGrabber::H264);
	grabber.setGrabber(axis);

	address.set("address",cameraAddress);

	gui.setup(cameraName,"settings.xml",x+650,y);
	gui.add(addressLabel.setup("ip:",address));
	gui.add(changeIp.setup("changeIp"));
	gui.add(reset.setup("reset"));
	gui.add(resolution.set("resolution",1,0,3));

	gui.add(showFocusWindow.set("showFocusWindow",false));
	gui.add(autofocus.setup("autofocus"));
	gui.add(axis->focus);
	gui.add(axis->manualIris);
	gui.add(axis->exposure);
	gui.add(axis->irFilterCut);
	gui.add(axis->compression);
	gui.add(axis->paramRefreshRateMs);

	gui.getIntSlider("focus").setUpdateOnReleaseOnly(true);
	gui.getIntSlider("exposure").setUpdateOnReleaseOnly(true);
	gui.getIntSlider("irFilterCut").setUpdateOnReleaseOnly(true);
	gui.getIntSlider("compression").setUpdateOnReleaseOnly(true);
	gui.getIntSlider("resolution").setUpdateOnReleaseOnly(true);

	autofocus.addListener(this,&ofxAxisGui::autofocusPressed);
	changeIp.addListener(this,&ofxAxisGui::changeIpPressed);
	resolution.addListener(this,&ofxAxisGui::resolutionChanged);
	reset.addListener(this,&ofxAxisGui::resetPressed);

	ofAddListener(ofEvents().mousePressed,this,&ofxAxisGui::mousePressed);
	ofAddListener(ofEvents().mouseReleased,this,&ofxAxisGui::mouseReleased);

	resetCamera();
}

void ofxAxisGui::update(){
	grabber.update();
	if(settingFocusWindow  && showFocusWindow && ofGetMousePressed(0)){
		focusWindow.width = (ofGetMouseX() - position.x - focusWindow.x)*grabber.getWidth()/w;
		focusWindow.height = (ofGetMouseY() - position.y - focusWindow.y)*grabber.getHeight()/h;
	}else if(showFocusWindow){
		focusWindow = axis->getFocusWindow();
	}
}



void ofxAxisGui::draw(float x, float y, float _w, float _h){
	ofPushStyle();
	position.set(x,y);
	w = _w;
	h = _h;
	if(bDrawGui){
		gui.setPosition(position.x+w+10,position.y);
		gui.draw();
	}
	grabber.draw(position,w,h);
	if(showFocusWindow){
		ofRectangle scaledWindow = focusWindow;
		scaledWindow.scale(w/grabber.getWidth(),h/grabber.getHeight());
		scaledWindow = scaledWindow + position;
		ofNoFill();
		ofSetColor(0,255,0);
		ofRect(scaledWindow);
		ofFill();
	}
	ofPushMatrix();
	ofTranslate(position);
	ofEnableAlphaBlending();
	ofSetColor(0,180);
	ofFill();
	ofRect(0,0,w,20);
	ofSetColor(255);
	string cameraStr = cameraName + ": " + (string)address + " " + ofToString((int)axis->fps) + "fps";
	if(!axis->cameraConnected){
		cameraStr += " not connected";
		ofSetColor(255,0,0);
	}else if(!axis->cameraAuth){
		cameraStr += " wrong passwd";
		ofSetColor(255,0,0);
	}
	ofDrawBitmapString(cameraStr,5,15);
	ofPopMatrix();
	ofPopStyle();
}

void ofxAxisGui::resetPressed(bool & pressed){
	if(!pressed)
		resetCamera();
}

void ofxAxisGui::autofocusPressed(bool & pressed){
	if(!pressed)
		axis->triggerAutoFocus();
}

void ofxAxisGui::changeIpPressed(bool & pressed){
	if(!pressed){
		string newAddress = ofSystemTextBoxDialog("ip", address);
		if(string(address) != newAddress){
			address = newAddress;
			resetCamera();
		}
	}
}

void ofxAxisGui::resolutionChanged(int & resolution){
	resetCamera();
}

void ofxAxisGui::resetCamera(){
	grabber.close();
	axis->setCameraAddress(address);
	switch (resolution) {
		case 0:
			w=320, h=240;
			break;
		case 1:
			w=640, h=480;
			break;
		case 2:
			w=800, h=600;
			break;
		case 3:
			w=1280, h=720;
			break;
		default:
			break;
	}
	grabber.initGrabber(w,h);
}


//--------------------------------------------------------------
void ofxAxisGui::mousePressed(ofMouseEventArgs & mouse){
	ofRectangle grabberRect(position,w,h);
	if(mouse.button == 0 && showFocusWindow && grabberRect.inside(mouse.x,mouse.y)){
		focusWindow.x = mouse.x-position.x;
		focusWindow.y = mouse.y-position.y;
		focusWindow.width = 0;
		focusWindow.height = 0;
		settingFocusWindow = true;
	}

}

//--------------------------------------------------------------
void ofxAxisGui::mouseReleased(ofMouseEventArgs & mouse){
	if(settingFocusWindow && mouse.button==0 && showFocusWindow){
		axis->setFocusWindow(focusWindow);
		settingFocusWindow = false;
	}
}
