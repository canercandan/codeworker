/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2002 Cédric Lemaire

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

To contact the author: codeworker@free.fr
*/

#ifdef WIN32
#pragma warning (disable : 4786)
#endif

#include "UtlException.h"
#include "ScpStream.h"
#include "DtaProject.h"
#include "CGRuntime.h"
#include "DtaSharpTagsHandler.h"
#include "DtaProtectedAreasBag.h"

namespace CodeWorker {
	DtaProtectedAreasBag::~DtaProtectedAreasBag() {
		clearAll();
	}

	void DtaProtectedAreasBag::clearAll() {
		for (std::map<std::string, DtaProtectedArea*>::const_iterator i = _codes.begin(); i != _codes.end(); i++) {
			if (i->second != NULL) delete i->second;
		}
		_codes = std::map<std::string, DtaProtectedArea*>();
	}

	std::string DtaProtectedAreasBag::getProtection(const char* sProtection) const {
		std::map<std::string, DtaProtectedArea*>::const_iterator i = _codes.find(sProtection);
		if (i != _codes.end()) return i->second->getText();
		return "";
	}

	void DtaProtectedAreasBag::setProtection(const char* sProtection, const char* sContent) {
		std::map<std::string, DtaProtectedArea*>::const_iterator i = _codes.find(sProtection);
		if (i != _codes.end()) {
			if (i->second->isAlreadyGenerated()) throw UtlException(std::string("protected area called \"") + sProtection + "\" has already been generated");
			i->second->setText(sContent);
		} else {
			DtaProtectedArea* pArea = new DtaProtectedArea(sContent, "");
			_codes[sProtection] = pArea;
		}
	}

	bool DtaProtectedAreasBag::removeProtection(const char* sProtection) {
		std::map<std::string, DtaProtectedArea*>::iterator cursor = _codes.find(sProtection);
		if (cursor == _codes.end()) return false;
		if (cursor->second->isAlreadyGenerated()) return false;
		cursor->second->isAlreadyGenerated(true);
		return true;
	}

	std::list<std::string> DtaProtectedAreasBag::getProtectionKeys() const {
		std::list<std::string> listOfKeys;
		for (std::map<std::string, DtaProtectedArea*>::const_iterator i = _codes.begin(); i != _codes.end(); i++) {
			if (i->second != NULL) listOfKeys.push_back(i->first);
		}
		return listOfKeys;
	}

	std::list<std::string> DtaProtectedAreasBag::remainingProtectionKeys() const {
		std::list<std::string> listOfKeys;
		for (std::map<std::string, DtaProtectedArea*>::const_iterator i = _codes.begin(); i != _codes.end(); i++) {
			register DtaProtectedArea* pArea = i->second;
			if ((pArea != NULL) && !pArea->isAlreadyGenerated()) {
				listOfKeys.push_back(i->first);
			}
		}
		return listOfKeys;
	}

	const DtaProtectedArea& DtaProtectedAreasBag::registerNewProtection(const char* sProtection) {
		std::map<std::string, DtaProtectedArea*>::const_iterator i = _codes.find(sProtection);
		DtaProtectedArea* pArea;
		if (i == _codes.end()) {
			pArea = new DtaProtectedArea("");
			_codes[sProtection] = pArea;
		} else {
			pArea = i->second;
			if (pArea->isAlreadyGenerated()) throw UtlException(std::string("protected area called \"") + sProtection + "\" has already been generated");
		}
		pArea->isAlreadyGenerated(true);
		return *pArea;
	}

	void DtaProtectedAreasBag::loadProtectedCodes(const char* sFile) {
		std::string sCompleteFileName;
		ScpStream* pInputFile = ScpStream::openInputFileFromIncludePath(sFile, sCompleteFileName);
		if (pInputFile != NULL) {
			try {
				recoverProtectedCodes(*pInputFile);
			} catch(UtlException& e) {
				std::string sMessage = sFile;
				sMessage += ", ";
				sMessage += e.getMessage();
				pInputFile->close();
				delete pInputFile;
				throw UtlException(e.getTraceStack(), sMessage);
			} catch(std::exception&) {
				pInputFile->close();
				delete pInputFile;
				throw;
			}
			pInputFile->close();
			delete pInputFile;
		}
	}

	void DtaProtectedAreasBag::recoverProtectedCodes(ScpStream& inputFile) {
		if (!DtaProject::getInstance().getCommentBegin().empty()) {
			int iLocation = inputFile.getInputLocation();
			inputFile.setInputLocation(0);
			DtaSharpTagsHandler tagsHandler(&inputFile);
			while (recoverProtectedCode(tagsHandler)) ;
			inputFile.setInputLocation(iLocation);
		}
	}

	void DtaProtectedAreasBag::recoverMarker(DtaSharpTagsHandler& tagsHandler) {
		ScpStream& inputFile = tagsHandler.getInputStream();
		if (tagsHandler.readMarkupBegin()) {
			std::string sStringMarkerKey = "\"" + CGRuntime::composeCLikeString(DtaProject::getInstance().getMarkupKey()) + "\"";
			int iBegin = inputFile.getInputLocation();
			int iEnd = tagsHandler.locateMarkupEnd();
			int iAfterMarkupEnd = inputFile.getInputLocation();
			inputFile.setInputLocation(iBegin);
			while (recoverProtectedCode(tagsHandler, iEnd)) ;
			inputFile.setInputLocation(iAfterMarkupEnd);
		}
	}

	bool DtaProtectedAreasBag::recoverProtectedCode(DtaSharpTagsHandler& tagsHandler, int iEnd) {
		ScpStream& inputFile = tagsHandler.getInputStream();
		if (!tagsHandler.findGenerationProtectBegin(iEnd)) return false;
		if (_codes.find(tagsHandler.getProtectKey()) != _codes.end()) throw UtlException(inputFile, "a protected area called \"" + tagsHandler.getProtectKey() + "\" already exists");
		int iBeginCode = inputFile.getInputLocation();
		int iEndCode = tagsHandler.locateGenerationProtectEnd();
		if ((iBeginCode < 0) || (iBeginCode > iEndCode) || (iEndCode > inputFile.size())) throw UtlException(inputFile, "inconsistence in boundaries of the protected area called \"" + tagsHandler.getProtectKey() + "\"");
		if (iBeginCode != iEndCode) {
			std::string sCode(inputFile.readBuffer() + iBeginCode, iEndCode - iBeginCode);
			DtaProtectedArea* pArea = new DtaProtectedArea(sCode, tagsHandler.getProtectDefine());
			_codes[tagsHandler.getProtectKey()] = pArea;
		}
		return true;
	}
}
