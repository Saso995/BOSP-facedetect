/**
 *       @file  FaceDetect_exc.h
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

#ifndef FACEDETECT_EXC_H_
#define FACEDETECT_EXC_H_

#include "bbque/cpp11/thread.h"
#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <vector>


#include <bbque/bbque_exc.h>
#include <bbque/utils/utility.h>

using bbque::rtlib::BbqueEXC;
using namespace cv;
using namespace std;


class FaceDetect : public BbqueEXC {


string cascadeName;                         
string nestedCascadeName;
bool tryflip;
double scale;
string inputName;

public:

	FaceDetect(std::string const & name,
			std::string cascade, std::string nestedCascade,
			bool tf, double s, std::string input,
			std::string const & recipe,
			RTLIB_Services_t *rtlib);

private:

	RTLIB_ExitCode_t onSetup();
	RTLIB_ExitCode_t onConfigure(int8_t awm_id);
	RTLIB_ExitCode_t onRun();
	RTLIB_ExitCode_t onMonitor();
	RTLIB_ExitCode_t onSuspend();
	RTLIB_ExitCode_t onRelease();

};

#endif // FACEDETECT_EXC_H_
