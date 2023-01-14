/*******************************************************************************
* This file is part of psx-comBINe. Please see the github:
* https://github.com/ADBeta/psx-comBINe
*
* psx-comBINe is a simple program to combine multiple .bin files into a single
* file, and modified the .cue file indexing, this is ideal for PSX/PS1 CD-ROMs
* e.g. Rayman to get them ready for cue2pops or some emulators. I also find it 
* improves reliabilty when buring to a disk to only have one .bin file.
*
* Cue File manager. Uses TeFiEd to handle output file cue file track, file and 
* index text
* 
* this might be turned into its own project if I see it useful in future.
*
* (c) ADBeta
*******************************************************************************/
#include "cueHandler.h"

#include <iostream>
#include <vector>

#include "helpers.h"
#include "TeFiEd.h"

CueHandler::CueHandler(const char* filename) {
	//Open the cue file passed
	cueFile = new TeFiEd(filename);
	
	//Set safety flag on the .cue input file. 100KB
	cueFile->setByteLimit(102400);
	
	cueFile->setVerbose(true);
}

/** File Managment ************************************************************/
void CueHandler::read() {
	if(cueFile->read() != 0) {
		exit(EXIT_FAILURE); 
	}
}

void CueHandler::create() {
	if(cueFile->create() != 0) {
		exit(EXIT_FAILURE);
	}
}

/** Reading functions *********************************************************/
std::string CueHandler::getFILE(const std::string line) {
	//Find the first and last quote mark, and handle errors if none exist.
	size_t firstQuote, lastQuote;
	//First Quote
	firstQuote = line.find('"') + 1;
	
	//Last Quote
	if(firstQuote != std::string::npos) lastQuote = line.find('"', firstQuote);
	
	if(firstQuote == std::string::npos || lastQuote == std::string::npos) {
		//Fatal error
		errorMsg(2, "A filename in .cue file is corrupted (Missing quotes)");	
	}
	
	//Create a new string from that substring - subst(firstQuite, length)
	std::string filename = line.substr(firstQuote, (lastQuote - firstQuote));
	return filename;
}

void CueHandler::pushFILEToVector(std::vector<std::string> &vect) {
	size_t matchLineNo;
	while(( matchLineNo = cueFile->findNext("FILE") )) {
		//If the current line isn't valid, prompt with continue message.
		//Exit if false, continue if true.
		if(lineIsValid(matchLineNo) == false) {
			if(promptContinue() == false) return;
		}
		
		//Push the filename string to the vector.
		vect.push_back(cueFile->parentDir() + 
		                getFILE(cueFile->getLine(matchLineNo)));
	}
}

/** Writing functions *********************************************************/
bool CueHandler::lineIsValid(const size_t lineNo) {
	std::string lineStr = cueFile->getLine(lineNo);
	
	//Make sure the file extension is .bin	
	if(lineStr.find(".bin") == std::string::npos) {
		errorMsg(0, "file does not have .bin extension");
		return false;
	}
	
	//Make sure the file type is BINARY
	if(lineStr.find("BINARY") == std::string::npos) {
		errorMsg(0, "file is not of type BINARY");
		return false;
	}
	
	//File is valid
	return true;
}

/** AUX functions *************************************************************/
std::string CueHandler::bytesToTimestamp(const size_t bytes) {
	/* The timestamp is in Minute:Second:Frame format.
	There are 75 sectors per second, 2352 bytes per sector. If the input number 
	is not divisible by 1 sector, then exit the program.
	
	I am trying to use divide then modulo so the compiler stands some chance of
	optimizing my code to use the remainder of the ASM div operator. */
	
	std::string timestamp;	
	
	//There are 2352 byte per sector. Calculate how many sectors are in the file
	size_t sectors = bytes / 2352;
	
	//Error check if the input is divisible by a sector. Exit if not
	if(bytes % 2352 != 0) {
		errorMsg(2, "bin file is not divisible by SECTOR size (Corrupt bin)");
	}
	
	//75 sectors per second. Frames are the left over sectors from a second
	int seconds = sectors / 75;
	int rFrames = sectors % 75;
	
	//Convert seconds to minutes. Seconds is the remainder of itself after / 60
	int minutes = seconds / 60;
	seconds = seconds % 60;
	
	//If minutes exceeds 99, there is probably an error due to Audio CD Standard
	if(minutes > 99) errorMsg(1, "Total bin file size exceeds 99 minutes");
	
	//Now the string can be formed from the values. Need to 0 pad each value
	timestamp.append(padIntStr(minutes, 2) + ":" + padIntStr(seconds, 2) + ":"
	                           + padIntStr(rFrames, 2));
	
	return timestamp;
}





/** Private *******************************************************************/
std::string CueHandler::padIntStr(const unsigned long val, unsigned int pad) {
	std::string intStr = std::to_string(val);
	
	//Pad if selected
	if(pad != 0 && pad > intStr.length()) {
		unsigned int padDelta = pad - intStr.length();
		intStr.insert(0, padDelta, '0');
	}
	
	return intStr;
}




