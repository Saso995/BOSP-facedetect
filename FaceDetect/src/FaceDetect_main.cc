/**
 *       @file  FaceDetect_main.cc
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

#include <cstdio>
#include <iostream>
#include <random>
#include <cstring>
#include <memory>

#include <libgen.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "version.h"
#include "FaceDetect_exc.h"
#include <bbque/utils/utility.h>
#include <bbque/utils/logging/logger.h>

// Setup logging
#undef  BBQUE_LOG_MODULE
#define BBQUE_LOG_MODULE "FaceDetect"

namespace bu = bbque::utils;
namespace po = boost::program_options;

/**
 * @brief A pointer to an EXC
 */
std::unique_ptr<bu::Logger> logger;

/**
 * @brief A pointer to an EXC
 */
typedef std::shared_ptr<BbqueEXC> pBbqueEXC_t;

/**
 * The decription of each FaceDetect parameters
 */
po::options_description opts_desc("FaceDetect Configuration Options");

/**
 * The map of all FaceDetect parameters values
 */
po::variables_map opts_vm;

/**
 * The services exported by the RTLib
 */
RTLIB_Services_t *rtlib;

/**
 * @brief The application configuration file
 */
std::string conf_file = BBQUE_PATH_PREFIX "/" BBQUE_PATH_CONF "/FaceDetect.conf" ;

/**
 * @brief The recipe to use for all the EXCs
 */
std::string recipe;

/**
 * @brief The EXecution Context (EXC) registered
 */
pBbqueEXC_t pexc;


std::string cascade = string(getenv("HOME")) + "/BOSP/contrib/user/FaceDetect/src/data/haarcascades/haarcascade_frontalface_alt.xml";                        
std::string nestedCascade = string(getenv("HOME")) + "/BOSP/contrib/user/FaceDetect/src/data/haarcascades/haarcascade_eye_tree_eyeglasses.xml";
std::string input;
double s;
bool tf;


void ParseCommandLine(int argc, char *argv[]) {
	// Parse command line params
	try {
	po::store(po::parse_command_line(argc, argv, opts_desc), opts_vm);
	} catch(...) {
		std::cout << "Usage: " << argv[0] << " [options]\n";
		std::cout << opts_desc << std::endl;
		::exit(EXIT_FAILURE);
	}
	po::notify(opts_vm);

	// Check for help request
	if (opts_vm.count("help")) {
		std::cout << "Usage: " << argv[0] << " [options]\n";
		std::cout << "To quit during face recognition press q or Q" << std::endl;
		std::cout << opts_desc << std::endl;
		::exit(EXIT_SUCCESS);
	}

	// Check for version request
	if (opts_vm.count("version")) {
		std::cout << "FaceDetect (ver. " << g_git_version << ")\n";
		std::cout << "Copyright (C) 2011 Politecnico di Milano\n";
		std::cout << "\n";
		std::cout << "Built on " <<
			__DATE__ << " " <<
			__TIME__ << "\n";
		std::cout << "\n";
		std::cout << "This is free software; see the source for "
			"copying conditions.  There is NO\n";
		std::cout << "warranty; not even for MERCHANTABILITY or "
			"FITNESS FOR A PARTICULAR PURPOSE.";
		std::cout << "\n" << std::endl;
		::exit(EXIT_SUCCESS);
	}
}

int main(int argc, char *argv[]) {

	opts_desc.add_options()
		("help,h", "print this help message")
		("version,v", "print program version")
		
		("conf,C", po::value<std::string>(&conf_file)-> default_value(conf_file), "FaceDetect configuration file")
		("recipe,r", po::value<std::string>(&recipe)-> default_value("FaceDetect"), "recipe name (for all EXCs)")
		
		("cascade,c", po::value<std::string>(&cascade)-> default_value(cascade), "<cascade_path> this is the primary trained classifier such as frontal face")
		("nested-cascade,n", po::value<std::string>(&nestedCascade)-> default_value(nestedCascade),"nested_cascade_path this an optional secondary classifier such as eyes")	
		("scale,s", po::value<double>(&s)-> default_value(1), "image scale greater or equal to 0.5, try 1.3 for example")
		("try-flip,t", po::value<bool>(&tf)-> default_value(false),"try to flip the image")	
		("filename,f", po::value<std::string>(&input)-> default_value(input), "filename|camera_index to use")
	;

	// Setup a logger
	bu::Logger::SetConfigurationFile(conf_file);
	logger = bu::Logger::GetLogger("facedetect");

	ParseCommandLine(argc, argv);

	// Welcome screen
	logger->Info(".:: FaceDetect (ver. %s) ::.", g_git_version);
	logger->Info("Built: " __DATE__  " " __TIME__);

	// Initializing the RTLib library and setup the communication channel
	// with the Barbeque RTRM
	logger->Info("STEP 0. Initializing RTLib, application [%s]...",
			::basename(argv[0]));

	if ( RTLIB_Init(::basename(argv[0]), &rtlib) != RTLIB_OK) {
		logger->Fatal("Unable to init RTLib (Did you start the BarbequeRTRM daemon?)");
		return RTLIB_ERROR;
	}

	assert(rtlib);

	logger->Info("STEP 1. Registering EXC with recipe <%s>...", recipe.c_str());
	pexc = std::make_shared<FaceDetect>("FaceDetect", cascade, nestedCascade, tf, s, input, recipe, rtlib);
	if (!pexc->isRegistered()) {
		logger->Fatal("Registering failure.");
		return RTLIB_ERROR;
	}


	logger->Info("STEP 2. Starting EXC control thread...");
	pexc->Start();


	logger->Info("STEP 3. Waiting for EXC completion...");
	pexc->WaitCompletion();


	logger->Info("STEP 4. Disabling EXC...");
	pexc = NULL;

	logger->Info("===== FaceDetect DONE! =====");
	return EXIT_SUCCESS;

}

