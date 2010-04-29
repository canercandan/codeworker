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

#ifndef _DtaSharpTagsHandler_h_
#define _DtaSharpTagsHandler_h_

namespace CodeWorker {
	class ScpStream;

	class DtaSharpTagsHandlerImpl {
	protected:
		ScpStream* _pInputStream;

		// some attributes concerning '##protect##'
		std::string _sProtectDefine;
		std::string _sProtectKey;

	public:
		inline DtaSharpTagsHandlerImpl(ScpStream* pInputStream) : _pInputStream(pInputStream) {}
		virtual ~DtaSharpTagsHandlerImpl();

		inline ScpStream& getInputStream() const { return *_pInputStream; }
		inline const std::string& getProtectKey() const { return _sProtectKey; }
		inline const std::string& getProtectDefine() const { return _sProtectDefine; }

		virtual bool findExpansionMarkup() = 0;
		virtual bool readMarkupScript(std::string& sScript) = 0;
		virtual bool readMarkupData(std::string& sData) = 0;
		virtual bool readMarkupBegin() = 0;
		virtual int  locateMarkupEnd() = 0;
		virtual void writeMarkupBegin(ScpStream& outputStream) = 0;
		virtual void writeMarkupEnd(ScpStream& outputStream) = 0;

		virtual bool findGenerationProtectBegin(int iEnd) = 0;
		virtual int  locateGenerationProtectEnd() = 0;
		virtual void writeProtectBegin(ScpStream& outputStream, const std::string& sKey, const std::string& sDefine) = 0;
		virtual void writeProtectEnd(ScpStream& outputStream, const std::string& sKey, const std::string& sDefine) = 0;
	};

	class DtaSharpTagsHandler {
	private:
		DtaSharpTagsHandlerImpl* _pInternal;

	public:
		DtaSharpTagsHandler(ScpStream* pInputStream);
		~DtaSharpTagsHandler();

		inline ScpStream& getInputStream() const { return _pInternal->getInputStream(); }
		inline const std::string& getProtectKey() const { return _pInternal->getProtectKey(); }
		inline const std::string& getProtectDefine() const { return _pInternal->getProtectDefine(); }

		inline bool findExpansionMarkup() { return _pInternal->findExpansionMarkup(); }
		inline bool readMarkupScript(std::string& sScript) { return _pInternal->readMarkupScript(sScript); }
		inline bool readMarkupData(std::string& sData) { return _pInternal->readMarkupData(sData); }
		inline bool readMarkupBegin() { return _pInternal->readMarkupBegin(); }
		inline int  locateMarkupEnd() { return _pInternal->locateMarkupEnd(); }
		inline void writeMarkupBegin(ScpStream& outputStream) { _pInternal->writeMarkupBegin(outputStream); }
		inline void writeMarkupEnd(ScpStream& outputStream) { _pInternal->writeMarkupEnd(outputStream); }

		inline bool findGenerationProtectBegin(int iEnd) { return _pInternal->findGenerationProtectBegin(iEnd); }
		inline int  locateGenerationProtectEnd() { return _pInternal->locateGenerationProtectEnd(); }
		inline void writeProtectBegin(ScpStream& outputStream, const std::string& sKey, const std::string& sDefine) { _pInternal->writeProtectBegin(outputStream, sKey, sDefine); }
		inline void writeProtectEnd(ScpStream& outputStream, const std::string& sKey, const std::string& sDefine) { _pInternal->writeProtectEnd(outputStream, sKey, sDefine); }
	};
}

#endif
