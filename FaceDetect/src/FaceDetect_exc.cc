/**
 *       @file  FaceDetect_exc.cc
 *      @brief  The FaceDetect BarbequeRTRM application
 *
 * Description: Adaptation of the opencv sample facedetect.cpp to AEM
 *
 *     @author  Salvatore Bova (10499292), salvatore.bova@mail.polimi.it
 *
 *     Company  Politecnico Di Milano
 *   Copyright  Copyright (c) 2020, Salvatore Bova
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#include "FaceDetect_exc.h"

#include <cstdio>
#include <bbque/utils/utility.h>

// My libraries
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <limits>

// Setup logging
#undef  BBQUE_LOG_MODULE
#define BBQUE_LOG_MODULE "aem.facedetect"
#undef  BBQUE_LOG_UID
#define BBQUE_LOG_UID GetChUid()

void detectAndDraw( Mat& img, CascadeClassifier& cascade, CascadeClassifier& nestedCascade, double scale, bool tryflip );
fstream& GotoLine(fstream& file, unsigned int num);

CascadeClassifier cascade, nestedCascade;
VideoCapture capture;                   
Mat frame, image, image2;
bool stopFlag = false;
int countFileLine = 1;
double detectionTime;


FaceDetect::FaceDetect(std::string const & name,
		std::string cascade, std::string nestedCascade,
		bool tf, double s, std::string input, 
		std::string const & recipe,
		RTLIB_Services_t *rtlib) :
		BbqueEXC(name, recipe, rtlib),
		cascadeName(cascade),
		nestedCascadeName(nestedCascade),
		tryflip(tf),
		scale(s),
		inputName(input)
{
	logger->Warn("New FaceDetect::FaceDetect()");
	logger->Info("EXC Unique IDentifier (UID): %u", GetUniqueID());
}

RTLIB_ExitCode_t FaceDetect::onSetup() {

	logger->Warn("FaceDetect::onSetup()");

	if (scale < 0.5)
		scale = 1;
	
	if (!nestedCascade.load(samples::findFileOrKeep(nestedCascadeName))){
        logger->Warn("WARNING: Could not load classifier cascade for nested objects");
        return RTLIB_ERROR;
    }
        

    if (!cascade.load(samples::findFile(cascadeName)))
    {
        logger->Warn("ERROR: Could not load classifier cascade");
        return RTLIB_ERROR;
    }

    //open camera
    if( inputName.empty() || (isdigit(inputName[0]) && inputName.size() == 1) )
    {
        int camera = inputName.empty() ? 0 : inputName[0] - '0';
        if(!capture.open(camera))
        {
            logger->Error("Capture from camera # %d didn't work", camera);
            return RTLIB_ERROR;
        }
    }
    //open image
    else if (!inputName.empty())
    {
        image = imread(samples::findFileOrKeep(inputName), IMREAD_COLOR);
    }
    else
    {
        image = imread(samples::findFile("lena.jpg"), IMREAD_COLOR);
        if (image.empty())
        {
            logger->Warn("Couldn't read lena.jpg");
            return RTLIB_ERROR;
        }
    }

	return RTLIB_OK;
}

RTLIB_ExitCode_t FaceDetect::onConfigure(int8_t awm_id) {

	logger->Warn("FaceDetect::onConfigure(): EXC [%s] => AWM [%02d]",
		exc_name.c_str(), awm_id);

	int32_t proc_quota, proc_nr, mem;
	GetAssignedResources(PROC_ELEMENT, proc_quota);
	GetAssignedResources(PROC_NR, proc_nr);
	GetAssignedResources(MEMORY, mem);
	logger->Notice("MayApp::onConfigure(): "
		"EXC [%s], AWM[%02d] => R<PROC_quota>=%3d, R<PROC_nr>=%2d, R<MEM>=%3d",
		exc_name.c_str(), awm_id, proc_quota, proc_nr, mem);

	return RTLIB_OK;
}

RTLIB_ExitCode_t FaceDetect::onRun() {

    logger->Warn("FaceDetect::onRun()");
    
    if( capture.isOpened() ){           
        logger->Info("Video capturing has been started...");

        capture >> frame;
        if( frame.empty() ){
            logger->Warn("Something bad with your webcam happened!");
            exit(0);
        }
            
        Mat frame1 = frame.clone();
        detectAndDraw( frame1, cascade, nestedCascade, scale, tryflip );
        logger->Notice("Detection time = %g ms", detectionTime);

        char c = (char)waitKey(10);
        if(c == 'q' || c == 'Q') 
            stopFlag = true; 
    }
    else                                
    {
        if( !image.empty() )
        {
            logger->Notice("Detecting face(s) in %s", inputName.c_str());
            detectAndDraw( image, cascade, nestedCascade, scale, tryflip );
            logger->Notice("Detection time = %g ms", detectionTime);
            char c = (char)waitKey(0);
            stopFlag = true;
            
        }
        else if( !inputName.empty() )
        {
            fstream file(inputName);

            GotoLine(file, countFileLine);
            

            string line;
            file >> line;

            if (line != ""){
                image2 = imread(samples::findFileOrKeep(line), IMREAD_COLOR);
                if( !image2.empty() )
                {
                    logger->Notice("Detecting face(s) in %s", line.c_str());
                    detectAndDraw( image2, cascade, nestedCascade, scale, tryflip );
                    logger->Notice("Detection time = %g ms", detectionTime);
                    char c = (char)waitKey(0);
                    countFileLine++;
                    if( c == 'q' || c == 'Q' ){
                        stopFlag = true;
                    }
                }
                else{
                    logger->Error("Could not read %s", inputName.c_str());
                    stopFlag = true;
                }
            }
            else {
                logger->Error("Could not read %s", inputName.c_str());
                stopFlag = true;
            }            
        }
    }
	return RTLIB_OK;
}

RTLIB_ExitCode_t FaceDetect::onMonitor() {	

    if (stopFlag){
        logger->Warn("FaceDetect::onMonitor()  : exit");
        return RTLIB_EXC_WORKLOAD_NONE;
    }
    else{
        if (detectionTime > 1000 && scale < 2){
            scale = 2;
            logger->Warn("FaceDetect::onMonitor()  :Too low, let's increase scale");
        }
        else if ((detectionTime > 500 && detectionTime <= 1000) && scale < 1.5){
            scale = 1.5;
            logger->Warn("FaceDetect::onMonitor()  :Too low, let's increase scale");
        }
        else if ((detectionTime > 300 && detectionTime <= 500) && scale < 1){
            scale = 1;
            logger->Warn("FaceDetect::onMonitor()  :Too low, let's increase scale");
        }
        else if ((detectionTime > 100 && detectionTime <= 300) && scale < 0.8 ){
            scale = 0.8;
            logger->Warn("FaceDetect::onMonitor()  :Too low, let's increase scale");
        }
        else if (detectionTime < 100){
            scale = scale - 0.5;
            logger->Warn("FaceDetect::onMonitor()  :Too fast, let's improve accuracy decreasing scale");
        }
        else{
            ;
        }
    }

 	logger->Warn("FaceDetect::onMonitor()  : exit");
	return RTLIB_OK;
}

RTLIB_ExitCode_t FaceDetect::onSuspend() {

	logger->Warn("FaceDetect::onSuspend()  : suspension...");

	return RTLIB_OK;
}

RTLIB_ExitCode_t FaceDetect::onRelease() {

	logger->Warn("FaceDetect::onRelease()  : exit");

	return RTLIB_OK;
}

void detectAndDraw( Mat& img, CascadeClassifier& cascade, CascadeClassifier& nestedCascade, double scale, bool tryflip )
{
    double t = 0;
    vector<Rect> faces, faces2;
    const static Scalar colors[] =
    {
        Scalar(255,0,0),
        Scalar(255,128,0),
        Scalar(255,255,0),
        Scalar(0,255,0),
        Scalar(0,128,255),
        Scalar(0,255,255),
        Scalar(0,0,255),
        Scalar(255,0,255)
    };
    Mat gray, smallImg;

    cvtColor( img, gray, COLOR_BGR2GRAY );
    double fx = 1 / scale;
    resize( gray, smallImg, Size(), fx, fx, INTER_LINEAR_EXACT );
    equalizeHist( smallImg, smallImg );

    t = (double)getTickCount();
    cascade.detectMultiScale( smallImg, faces,
        1.1, 2, 0
        //|CASCADE_FIND_BIGGEST_OBJECT
        //|CASCADE_DO_ROUGH_SEARCH
        |CASCADE_SCALE_IMAGE,
        Size(30, 30) );
    if( tryflip )
    {
        flip(smallImg, smallImg, 1);
        cascade.detectMultiScale( smallImg, faces2,
                                 1.1, 2, 0
                                 //|CASCADE_FIND_BIGGEST_OBJECT
                                 //|CASCADE_DO_ROUGH_SEARCH
                                 |CASCADE_SCALE_IMAGE,
                                 Size(30, 30) );
        for( vector<Rect>::const_iterator r = faces2.begin(); r != faces2.end(); ++r )
        {
            faces.push_back(Rect(smallImg.cols - r->x - r->width, r->y, r->width, r->height));
        }
    }
    t = (double)getTickCount() - t;
    detectionTime = t*1000/getTickFrequency();
    for ( size_t i = 0; i < faces.size(); i++ )
    {
        Rect r = faces[i];
        Mat smallImgROI;
        vector<Rect> nestedObjects;
        Point center;
        Scalar color = colors[i%8];
        int radius;

        double aspect_ratio = (double)r.width/r.height;
        if( 0.75 < aspect_ratio && aspect_ratio < 1.3 )
        {
            center.x = cvRound((r.x + r.width*0.5)*scale);
            center.y = cvRound((r.y + r.height*0.5)*scale);
            radius = cvRound((r.width + r.height)*0.25*scale);
            circle( img, center, radius, color, 3, 8, 0 );
        }
        else
            rectangle( img, Point(cvRound(r.x*scale), cvRound(r.y*scale)),
                       Point(cvRound((r.x + r.width-1)*scale), cvRound((r.y + r.height-1)*scale)),
                       color, 3, 8, 0);
        if( nestedCascade.empty() )
            continue;
        smallImgROI = smallImg( r );
        nestedCascade.detectMultiScale( smallImgROI, nestedObjects,
            1.1, 2, 0
            //|CASCADE_FIND_BIGGEST_OBJECT
            //|CASCADE_DO_ROUGH_SEARCH
            //|CASCADE_DO_CANNY_PRUNING
            |CASCADE_SCALE_IMAGE,
            Size(30, 30) );
        for ( size_t j = 0; j < nestedObjects.size(); j++ )
        {
            Rect nr = nestedObjects[j];
            center.x = cvRound((r.x + nr.x + nr.width*0.5)*scale);
            center.y = cvRound((r.y + nr.y + nr.height*0.5)*scale);
            radius = cvRound((nr.width + nr.height)*0.25*scale);
            circle( img, center, radius, color, 3, 8, 0 );
        }
    }
    namedWindow("window", WINDOW_NORMAL);
    resizeWindow("window", 600,600);
    imshow( "window", img );
}

fstream& GotoLine(fstream& file, unsigned int num){
    file.seekg(ios::beg);
    for(int i=0; i < num - 1; ++i){
        file.ignore(numeric_limits<streamsize>::max(),'\n');
    }
    return file;
}