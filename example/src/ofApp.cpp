#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	axisGrabber = ofPtr<ofxAxisGrabber>(new ofxAxisGrabber);
	axisGrabber->setCameraAddress("76.10.86.11");
	grabber.setGrabber(axisGrabber);
	grabber.initGrabber(640,480);

}

//--------------------------------------------------------------
void ofApp::update(){
	grabber.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
	grabber.draw(0,0);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
