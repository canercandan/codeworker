/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2004 Cédric Lemaire

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
#include "UtlDate.h"
#include "ScpStream.h"

#include "DtaProject.h"
#include "CGRuntime.h"
#include "DtaBNFScript.h"

#include "DtaSharpTagsHandler.h"

namespace CodeWorker {

	DtaSharpTagsHandlerImpl::~DtaSharpTagsHandlerImpl() {}


	class DtaClassicalSharpTagsHandler : public DtaSharpTagsHandlerImpl {
	private:
		std::string _sCommentEnd;

		// some attributes concerning '##markup##'
		bool _bKeyIsIdentifier;

	public:
		DtaClassicalSharpTagsHandler(ScpStream* pInputStream) : DtaSharpTagsHandlerImpl(pInputStream) {
			if ((DtaProject::getInstance().getTextMode() == DtaProject::DOS_MODE) && (DtaProject::getInstance().getCommentEnd() == "\n")) _sCommentEnd = "\r\n";
			else _sCommentEnd = DtaProject::getInstance().getCommentEnd();
		}

		virtual bool findExpansionMarkup() {
			// complete reading of the markup announcement
			std::string sMarker = DtaProject::getInstance().getCommentBegin() + "##mark";
			if (!_pInputStream->findString(sMarker)) return false;
			if (!_pInputStream->isEqualTo("er##") && !_pInputStream->isEqualTo("up##")) throw UtlException(*_pInputStream, "unrecognized markup format: '##markup##' or '##marker##' expected");
			_pInputStream->skipEmpty();
			std::string sMarkerKey;
			if (_pInputStream->readString(sMarkerKey)) _bKeyIsIdentifier = false;
			else if (_pInputStream->readIdentifier(sMarkerKey)) _bKeyIsIdentifier = true;
			else {
				throw UtlException(*_pInputStream, "unrecognized markup format: string or identifier expected after '##markup##'");
			}
			DtaProject::getInstance().setMarkupKey(sMarkerKey);
			while (!_pInputStream->isEqualTo(DtaProject::getInstance().getCommentEnd())) {
				int a = _pInputStream->readChar();
				if ((a < 0) || (a > (int) ' ')) throw UtlException(*_pInputStream, "end of comment expected after '##markup##\"" + DtaProject::getInstance().getMarkupKey() + "\"'");
			}
			return true;
		}

		virtual bool readMarkupScript(std::string& sScript) {
			std::string sScriptMarker = DtaProject::getInstance().getCommentBegin() + "##script##";
			if (!_pInputStream->isEqualTo(sScriptMarker)) return false;
			// extracting the script block
			//  //##script## ... //##script##
			while (!_pInputStream->isEqualTo(DtaProject::getInstance().getCommentEnd())) {
				int a = _pInputStream->readChar();
				if ((a < 0) || (a > (int) ' ')) throw UtlException(*_pInputStream, "end of comment expected after '##script##'");
			}
			int iStartScript = _pInputStream->getInputLocation();
			if (!_pInputStream->findString(sScriptMarker)) throw UtlException(*_pInputStream, "'##script##' expected to close the template-based script block of markup '" + DtaProject::getInstance().getMarkupKey() + "'");
			_pInputStream->setInputLocation(_pInputStream->getInputLocation() - sScriptMarker.size());
			_pInputStream->readLastChars(_pInputStream->getInputLocation() - iStartScript, sScript);
			_pInputStream->setInputLocation(_pInputStream->getInputLocation() + sScriptMarker.size());
			while (!_pInputStream->isEqualTo(DtaProject::getInstance().getCommentEnd())) {
				int a = _pInputStream->readChar();
				if ((a < 0) || (a > (int) ' ')) throw UtlException(*_pInputStream, "end of comment expected after '##script##'");
			}
			std::string::size_type iIndex = 0;
			register char a;
			while ((a = sScript[iIndex]) && (a > '\0') && (a <= ' ')) iIndex++;
			int iCBLength = DtaProject::getInstance().getCommentBegin().size();
			while (strncmp(sScript.c_str() + iIndex, DtaProject::getInstance().getCommentBegin().c_str(), iCBLength) == 0) {
				std::string::size_type iNewIndex = sScript.find(DtaProject::getInstance().getCommentEnd(), iIndex);
				if (iNewIndex == std::string::npos) break;
				std::string sTemp;
				if (iNewIndex > 0) sTemp = sScript.substr(0, iIndex);
				std::string::size_type iNextIndex = iNewIndex + DtaProject::getInstance().getCommentEnd().size();
				sTemp += sScript.substr(iIndex + iCBLength, iNewIndex - (iIndex + iCBLength));
				sTemp += '\n';
				if (iNextIndex < sScript.size()) {
					sScript = sTemp + sScript.substr(iNextIndex);
					iIndex = sTemp.size();
					register char a;
					while ((a = sScript[iIndex]) && (a > '\0') && (a <= ' ')) iIndex++;
				} else {
					sScript = sTemp;
					break;
				}
			}
			return true;
		}

		virtual bool readMarkupData(std::string& sData) {
			std::string sDataMarker = DtaProject::getInstance().getCommentBegin() + "##data##";
			if (!_pInputStream->isEqualTo(sDataMarker)) return false;
			// extracting the data block
			//  //##data## ... //##data##
			while (!_pInputStream->isEqualTo(DtaProject::getInstance().getCommentEnd())) {
				int a = _pInputStream->readChar();
				if ((a < 0) || (a > (int) ' ')) throw UtlException(*_pInputStream, "end of comment expected after '##data##'");
			}
			int iStartData = _pInputStream->getInputLocation();
			if (!_pInputStream->findString(sDataMarker)) throw UtlException(*_pInputStream, "'##data##' expected to close the data block of markup '" + DtaProject::getInstance().getMarkupKey() + "'");
			_pInputStream->setInputLocation(_pInputStream->getInputLocation() - sDataMarker.size());
			_pInputStream->readLastChars(_pInputStream->getInputLocation() - iStartData, sData);
			_pInputStream->setInputLocation(_pInputStream->getInputLocation() + sDataMarker.size());
			while (!_pInputStream->isEqualTo(DtaProject::getInstance().getCommentEnd())) {
				int a = _pInputStream->readChar();
				if ((a < 0) || (a > (int) ' ')) throw UtlException(*_pInputStream, "end of comment expected after '##data##'");
			}
			return true;
		}

		virtual bool readMarkupBegin() {
			if (DtaProject::getInstance().getCommentBegin().empty() || !_pInputStream->isEqualTo(DtaProject::getInstance().getCommentBegin() + "##begin##")) return false;
			std::string sStringMarkerKey = "\"" + CGRuntime::composeCLikeString(DtaProject::getInstance().getMarkupKey()) + "\"";
			if (!_pInputStream->isEqualTo(sStringMarkerKey)) throw UtlException(*_pInputStream, "'##begin##' tag is expected to concern marker called '" + sStringMarkerKey + "'");
			if (!_pInputStream->findString(DtaProject::getInstance().getCommentEnd())) throw UtlException(*_pInputStream, "'##begin##' tag is expected to end with an end of comment marker");
			return true;
		}

		virtual int locateMarkupEnd() {
			std::string sMarkupBegin = DtaProject::getInstance().getCommentBegin() + "##end##";
			if (!_pInputStream->findString(sMarkupBegin))  throw UtlException(*_pInputStream, "'##begin##' tag is expected to have an associated '##end##' tag");
			int iEnd = _pInputStream->getInputLocation() - sMarkupBegin.size();
			std::string sStringMarkerKey = "\"" + CGRuntime::composeCLikeString(DtaProject::getInstance().getMarkupKey()) + "\"";
			if (!_pInputStream->isEqualTo(sStringMarkerKey)) throw UtlException(*_pInputStream, "'##end##' tag is expected to concern marker called " + sStringMarkerKey);
			if (!_pInputStream->findString(DtaProject::getInstance().getCommentEnd())) throw UtlException(*_pInputStream, "'##end##' tag is expected to end with an end of comment marker");
			return iEnd;
		}

		virtual void writeMarkupBegin(ScpStream& outputStream) {
			outputStream << DtaProject::getInstance().getCommentBegin() << "##begin##";
			if (_bKeyIsIdentifier) outputStream << DtaProject::getInstance().getMarkupKey();
			else outputStream.writeString(DtaProject::getInstance().getMarkupKey());
			outputStream << _sCommentEnd;
		}

		virtual void writeMarkupEnd(ScpStream& outputStream) {
			outputStream << DtaProject::getInstance().getCommentBegin() << "##end##";
			if (_bKeyIsIdentifier) outputStream << DtaProject::getInstance().getMarkupKey();
			else outputStream.writeString(DtaProject::getInstance().getMarkupKey());
			outputStream << _sCommentEnd;
		}


		virtual bool findGenerationProtectBegin(int iEnd) {
			std::string sBeginning = DtaProject::getInstance().getCommentBegin() + "##protect";
			if (iEnd < 0) {
				if (!_pInputStream->findString(sBeginning.c_str())) return false;
			} else {
				if (!_pInputStream->findString(sBeginning.c_str(), iEnd)) return false;
			}
			_pInputStream->skipEmpty();
			bool bNotProtectDefine = _pInputStream->isEqualTo('!') && _pInputStream->skipEmpty();
			if (_pInputStream->readIdentifier(_sProtectDefine)) {
				if (bNotProtectDefine) _sProtectDefine = "!" + _sProtectDefine;
				_pInputStream->skipEmpty();
			} else if (bNotProtectDefine) {
				throw UtlException(*_pInputStream, "'!' should be followed by a property identifier");
			} else {
				_sProtectDefine = "";
			}
			if (!_pInputStream->isEqualTo("##")) throw UtlException(*_pInputStream, "wrong syntax for announcing protected area, '##' expected");
			if (!_pInputStream->readString(_sProtectKey)) throw UtlException(*_pInputStream, "wrong syntax for announcing protected area");
			if (!_pInputStream->findString(DtaProject::getInstance().getCommentEnd().c_str())) throw UtlException(*_pInputStream, "end of comment expected after announcing protected area \"" + _sProtectKey + "\"");
			return true;
		}

		virtual int locateGenerationProtectEnd() {
			std::string sBeginning = DtaProject::getInstance().getCommentBegin() + "##protect";
			if (!_pInputStream->findString(sBeginning.c_str())) throw UtlException(*_pInputStream, "end of protected area \"" + _sProtectKey + "\" expected");
			int iEnd = _pInputStream->getInputLocation() - sBeginning.size();
			_pInputStream->skipEmpty();
			std::string sProtectDefine;
			bool bNotProtectDefine = _pInputStream->isEqualTo('!') && _pInputStream->skipEmpty();
			if (_pInputStream->readIdentifier(sProtectDefine)) {
				if (bNotProtectDefine) sProtectDefine = "!" + sProtectDefine;
				_pInputStream->skipEmpty();
			} else if (bNotProtectDefine) {
				throw UtlException(*_pInputStream, "'!' should be followed by a property identifier");
			}
			if (sProtectDefine != _sProtectDefine) throw UtlException(*_pInputStream, "wrong syntax for finishing protected area \"" + _sProtectKey + "\", defines are different");
			if (!_pInputStream->isEqualTo("##")) throw UtlException(*_pInputStream, "wrong syntax for finishing protected area \"" + _sProtectKey + "\", '##' expected");
			std::string sProtectKey;
			if (!_pInputStream->readString(sProtectKey)) throw UtlException(*_pInputStream, "wrong syntax for finishing protected area \"" + _sProtectKey + "\"");
			if (sProtectKey != _sProtectKey) throw UtlException(*_pInputStream, "the key \"" + sProtectKey + "\" of the preserved area's end doesn't match with key \"" + _sProtectKey + "\", announced at the beginning");
			if (!_pInputStream->findString(DtaProject::getInstance().getCommentEnd().c_str())) throw UtlException(*_pInputStream, "end of comment expected after finishing protected area \"" + _sProtectKey + "\"");
			return iEnd;
		}

		virtual void writeProtectBegin(ScpStream& outputStream, const std::string& sKey, const std::string& sDefine) {
			outputStream << DtaProject::getInstance().getCommentBegin() << "##protect";
			if (!sDefine.empty()) outputStream << " " << sDefine;
			outputStream << "##";
			outputStream.writeString(sKey);
			if ((DtaProject::getInstance().getTextMode() == DtaProject::DOS_MODE) && (DtaProject::getInstance().getCommentEnd() == "\n")) outputStream << "\r\n";
			else outputStream << DtaProject::getInstance().getCommentEnd();
		}

		virtual void writeProtectEnd(ScpStream& outputStream, const std::string& sKey, const std::string& sDefine) {
			writeProtectBegin(outputStream, sKey, sDefine);
		}
	};


	class DtaCustomSharpTagsHandler : public DtaSharpTagsHandlerImpl {
	private:
		DtaBNFScript* _pReader;
		DtaPatternScript* _pWriter;
		DtaScriptVariable _context;

	public:
		inline DtaCustomSharpTagsHandler(ScpStream* pInputStream, DtaBNFScript* pReader, DtaPatternScript* pWriter)
			: DtaSharpTagsHandlerImpl(pInputStream),
			  _pReader(pReader), _pWriter(pWriter),
			  _context(&(DtaProject::getInstance()), "handler_context")
		{
		}

	private:
		bool executeReader() {
			CGRuntimeInputStream newInputStream(_pInputStream);
			return (_pReader->DtaPatternScript::execute(_context) == NO_INTERRUPTION);
		}

		void executeWriter(ScpStream& outputStream) {
			CGRuntimeOutputStream newOutputStream(&outputStream);
			_pWriter->execute(_context);
		}

	public:
		virtual bool findExpansionMarkup() {
			_context.insertNode("execute")->setValue("find_expansion_markup");
			if (!executeReader()) return false;
			DtaScriptVariable* pVariable = _context.getNode("markup_key");
			if (pVariable == NULL) throw UtlException("field 'this.markup_key' should have been populated by the 'find_expansion_markup' reader of the generation tags handler designated by key '" + DtaProject::getInstance().getCurrentGenerationTagsHandler() + "'");
			const char* tcMarkupKey = pVariable->getValue();
			if (tcMarkupKey != NULL) {
				DtaProject::getInstance().setMarkupKey(tcMarkupKey);
			} else {
				DtaProject::getInstance().setMarkupKey("");
			}
			return true;
		}

		virtual bool readMarkupScript(std::string& sScript) {
			_context.insertNode("execute")->setValue("read_markup_script");
			if (!executeReader()) return false;
			DtaScriptVariable* pVariable = _context.getNode("script");
			if (pVariable == NULL) throw UtlException("field 'this.script' should have been populated by the 'read_markup_script' reader of the generation tags handler designated by key '" + DtaProject::getInstance().getCurrentGenerationTagsHandler() + "'");
			const char* tcScript = pVariable->getValue();
			sScript = ((tcScript == NULL) ? "" : tcScript);
			return true;
		}

		virtual bool readMarkupData(std::string& sData) {
			_context.insertNode("execute")->setValue("read_markup_data");
			if (!executeReader()) return false;
			DtaScriptVariable* pVariable = _context.getNode("data");
			if (pVariable == NULL) throw UtlException("field 'this.data' should have been populated by the 'read_markup_data' reader of the generation tags handler designated by key '" + DtaProject::getInstance().getCurrentGenerationTagsHandler() + "'");
			const char* tcData = pVariable->getValue();
			sData = ((tcData == NULL) ? "" : tcData);
			return true;
		}

		virtual bool readMarkupBegin() {
			_context.insertNode("execute")->setValue("read_markup_begin");
			if (!executeReader()) return false;
			return true;
		}

		virtual int locateMarkupEnd() {
			_context.insertNode("execute")->setValue("locate_markup_end");
			if (!executeReader()) throw UtlException(*_pInputStream, "unable to locate the end of the markup '" + DtaProject::getInstance().getMarkupKey() + "'");
			DtaScriptVariable* pVariable = _context.getNode("beginning_position");
			if (pVariable == NULL) throw UtlException("field 'this.beginning_position' should have been populated by the 'locate_markup_end' reader of the generation tags handler designated by key '" + DtaProject::getInstance().getCurrentGenerationTagsHandler() + "'");
			return pVariable->getIntValue();
		}

		virtual void writeMarkupBegin(ScpStream& outputStream) {
			_context.insertNode("execute")->setValue("write_markup_begin");
			executeWriter(outputStream);
		}

		virtual void writeMarkupEnd(ScpStream& outputStream) {
			_context.insertNode("execute")->setValue("write_markup_end");
			executeWriter(outputStream);
		}


		virtual bool findGenerationProtectBegin(int iEnd) {
			_context.insertNode("execute")->setValue("find_generation_protect_begin");
			if (iEnd >= 0) _context.insertNode("end_position")->setValue(iEnd);
			else _context.remove("end_position");
			if (!executeReader()) return false;
			DtaScriptVariable* pVariable = _context.getNode("id");
			if (pVariable == NULL) throw UtlException("field 'this.id' should have been populated by the 'find_generation_protect_begin' reader of the generation tags handler designated by key '" + DtaProject::getInstance().getCurrentGenerationTagsHandler() + "'");
			const char* tcProtectKey = pVariable->getValue();
			_sProtectKey = ((tcProtectKey == NULL) ? "" : tcProtectKey);
			pVariable = _context.getNode("property");
			if (pVariable != NULL) {
				const char* tcProtectDefine = pVariable->getValue();
				_sProtectDefine = ((tcProtectDefine == NULL) ? "" : tcProtectDefine);
			} else _sProtectDefine = "";
			return true;
		}

		virtual int locateGenerationProtectEnd() {
			_context.insertNode("execute")->setValue("locate_generation_protect_end");
			_context.insertNode("id")->setValue(getProtectKey().c_str());
			if (!executeReader()) throw UtlException(*_pInputStream, "unable to locate the end of the protected area '" + getProtectKey() + "'");
			DtaScriptVariable* pVariable = _context.getNode("beginning_position");
			if (pVariable == NULL) throw UtlException("field 'this.beginning_position' should have been populated by the 'locate_generation_protect_end' reader of the generation tags handler designated by key '" + DtaProject::getInstance().getCurrentGenerationTagsHandler() + "'");
			return pVariable->getIntValue();
		}

		virtual void writeProtectBegin(ScpStream& outputStream, const std::string& sKey, const std::string& sDefine) {
			_context.insertNode("execute")->setValue("write_protect_begin");
			_context.insertNode("id")->setValue(sKey.c_str());
			if (!sDefine.empty()) _context.insertNode("property")->setValue(sDefine.c_str());
			else _context.remove("property");
			executeWriter(outputStream);
		}

		virtual void writeProtectEnd(ScpStream& outputStream, const std::string& sKey, const std::string& sDefine) {
			_context.insertNode("execute")->setValue("write_protect_end");
			_context.insertNode("id")->setValue(sKey.c_str());
			if (!sDefine.empty()) _context.insertNode("property")->setValue(sDefine.c_str());
			else _context.remove("property");
			executeWriter(outputStream);
		}
	};


	DtaSharpTagsHandler::DtaSharpTagsHandler(ScpStream* pInputStream) {
		std::string sKey = DtaProject::getInstance().getCurrentGenerationTagsHandler();
		if (sKey.empty()) {
			_pInternal = new DtaClassicalSharpTagsHandler(pInputStream);
		} else {
			DtaBNFScript* pReader;
			DtaPatternScript* pWriter;
			if (!DtaProject::getInstance().getGenerationTagsHandler(sKey, pReader, pWriter)) throw UtlException(*pInputStream, "internal error: no generation tags handler registered to the key '" + sKey + "'");
			_pInternal = new DtaCustomSharpTagsHandler(pInputStream, pReader, pWriter);
		}
	}

	DtaSharpTagsHandler::~DtaSharpTagsHandler() {
		delete _pInternal;
	}
}
