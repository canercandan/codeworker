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

#include "UtlString.h"
#include "UtlDate.h"
#include "UtlException.h"
#include "ScpStream.h"
#include "UtlXMLStream.h"

namespace CodeWorker {
	UtlXMLStream::UtlXMLStream(const std::string& sFileName, const bool bModeRead) : _bOwnerOfFileStream(true), _pInputStream(NULL), _pOutputStream(NULL) {
		if (!bModeRead) {
			_pOutputStream = CodeWorker::openOutputFile(sFileName.c_str());
			if (_pOutputStream == NULL) throw UtlException("unable to open file \"" + sFileName + "\" for writing");
				getOutputStream() << "<?xml version=\"1.0\" ?>" << std::endl;
		}
		else {
			_pInputStream = ScpStream::openSTLInputFile(sFileName.c_str());
			if (_pInputStream == NULL) throw UtlException("unable to open file \"" + sFileName + "\" for reading");
			

			std::string sLineVersion;
			if ( ! readLine(getInputStream() , sLineVersion) ) {
				throw UtlException("No version line in XML file: " + sFileName );
			}
		}
		
		_sFileName=sFileName;

	}


	UtlXMLStream::UtlXMLStream(std::istream& myStream) : _pInputStream(&myStream), _bOwnerOfFileStream(false) {
		std::string sLineVersion;
		if ( ! readLine(getInputStream() , sLineVersion) ) {
			throw UtlException("No version line in XML" );
		}
	}
			

	UtlXMLStream::UtlXMLStream(std::iostream& myStream) : _pInputStream(&myStream), _pOutputStream(&myStream), _bOwnerOfFileStream(false) {
		std::string sLineVersion;
		if ( ! readLine(getInputStream() , sLineVersion) ) {
			throw UtlException("No version line in XML" );
		}
	}


	// write part
	UtlXMLStream::~UtlXMLStream() {
		if (_bOwnerOfFileStream) {
			if (_pOutputStream != NULL) {
				((std::ofstream*) _pOutputStream)->close();
				delete _pOutputStream;
			}

			if (_pInputStream != NULL) {
				((std::ifstream*) _pInputStream)->close();
				delete _pInputStream;
			}
		}
	}

	void UtlXMLStream::writeStartTag(const std::string& sTag) {
		getOutputStream() << _sIndentation.c_str() << "<" << sTag.c_str() << ">" << std::endl;
		_sIndentation += "\t";
	}

	void UtlXMLStream::writeEndTag(const std::string& sTag) {
		_sIndentation = _sIndentation.substr(1);
		getOutputStream() << _sIndentation.c_str() << "</" << sTag.c_str() << ">" << std::endl;
	}

	void UtlXMLStream::writeTag(const std::string& sTag) {
		getOutputStream() << _sIndentation.c_str() << "<" << sTag.c_str() << "/>" << std::endl;
	}

	void UtlXMLStream::writeBeginningOfObject(const std::string& sTypeName) {
		getOutputStream() << _sIndentation.c_str() << "<" << sTypeName.c_str();
		_sIndentation += "\t";
	}

	void UtlXMLStream::writeEndOfObject(const std::string& sTypeName) {
		_sIndentation = _sIndentation.substr(1);
		getOutputStream() << _sIndentation.c_str() << "</" << sTypeName.c_str() << ">" << std::endl;
	}

	void UtlXMLStream::writeEndOfObject() {
		_sIndentation = _sIndentation.substr(1);
		getOutputStream() << " />" << std::endl;
	}

	std::string UtlXMLStream::normalizeAttributeName(const std::string& sName) const {
		char c = sName[0];
		if ((c < 'A') || (c > 'Z')) return sName;
		if (sName.size() >= 2) {
			char c1 = sName[1];
			if ((c1 >= 'A') && (c1 <= 'Z')) return sName;
		}
		std::string sNewName = sName;
		sNewName[0] = c + ' ';
		return sNewName;
	}

	void UtlXMLStream::writeAttribute(const std::string& sName, int iValue) {
		std::string sNormalizedName = normalizeAttributeName(sName);
		getOutputStream() << " " << sNormalizedName.c_str() << "=\"" << iValue << "\"";
	}

	void UtlXMLStream::writeAttribute(const std::string& sName, long lValue) {
		std::string sNormalizedName = normalizeAttributeName(sName);
		getOutputStream() << " " << sNormalizedName.c_str() << "=\"" << lValue << "\"";
	}

	void UtlXMLStream::writeAttribute(const std::string& sName, double dValue) {
		std::string sNormalizedName = normalizeAttributeName(sName);
		char sNumber[300];
		sprintf(sNumber, "%f", dValue);
		getOutputStream() << " " << sNormalizedName.c_str() << "=\"" << sNumber << "\"";
	}

	void UtlXMLStream::writeAttribute(const std::string& sName, const std::string& sValue) {
		std::string sNormalizedName = normalizeAttributeName(sName);
		std::string sXMLText = convertToXMLText(sValue);
		getOutputStream() << " " << sNormalizedName.c_str() << "=\"" << sXMLText.c_str() << "\"";
	}

	void UtlXMLStream::writeAttribute(const std::string& sName, bool bValue) {
		std::string sNormalizedName = normalizeAttributeName(sName);
		getOutputStream() << " " << sNormalizedName.c_str() << "=\"";
		if (bValue) getOutputStream() << "True";
		else getOutputStream() << "False";
		getOutputStream() << "\"";
	}

	void UtlXMLStream::writeAttribute(const std::string& sName, const UtlDate& myValue) {
		std::string sNormalizedName = normalizeAttributeName(sName);
		std::string sDate = myValue.getString();
		if (!myValue.isNull()) {
			getOutputStream() << " " << sNormalizedName.c_str() << "=\"" << sDate.c_str() << "\"";
		}
	}

	void UtlXMLStream::writeEndOfAttributes() {
		getOutputStream() << ">" << std::endl;
	}

	void UtlXMLStream::writeArrayElement(int iValue) {
		getOutputStream() << _sIndentation.c_str() << "<int value =\"" << iValue << "\" />" << std::endl;
	}

	void UtlXMLStream::writeArrayElement(double dValue) {
		getOutputStream() << _sIndentation.c_str() << "<double value =\"" << dValue << "\" />" << std::endl;
	}

	void UtlXMLStream::writeArrayElement(const std::string& sValue) {
		getOutputStream() << _sIndentation.c_str() << "<string value =\"" << sValue.c_str() << "\" />" << std::endl;
	}

	void UtlXMLStream::writeArrayElement(bool bValue) {
		getOutputStream() << _sIndentation.c_str() << "<bool value =\"";
		if (bValue) getOutputStream() << "True";
		else getOutputStream() << "False";
		getOutputStream() << "\"/>" << std::endl;
	}

	void UtlXMLStream::writeArrayElement(const UtlDate& myValue) {
		std::string sDate = myValue.getString();
		getOutputStream() << _sIndentation.c_str() << "<date value =\"" << sDate.c_str() << "\" />" << std::endl;
	}

	void UtlXMLStream::writeHashtableEntry(const std::string& sKey, const std::string& sValue) {
		getOutputStream() << _sIndentation.c_str() << "<pair key=\"" << sKey.c_str() << "\" value =\"" << sValue.c_str() << "\" />" << std::endl;
	}

	void UtlXMLStream::writeHashtableEntry(const std::string& sKey, double dValue) {
		getOutputStream() << _sIndentation.c_str() << "<pair key=\"" << sKey.c_str() << "\" value =\"" << dValue << "\" />" << std::endl;
	}

	void UtlXMLStream::writeHashtableEntry(const std::string& sKey, const std::vector<double>& listOfValues) {
		getOutputStream() << _sIndentation.c_str() << "<pair key=\"" << sKey.c_str() << "\" >" << std::endl;
		_sIndentation += "\t";
		for (std::vector<double>::const_iterator i = listOfValues.begin(); i != listOfValues.end(); i++) {
			getOutputStream() << _sIndentation.c_str() << "<double value=\"" << (*i) << "\" >" << std::endl;
		}
		_sIndentation = _sIndentation.substr(1);
		getOutputStream() << _sIndentation.c_str() << "</pair>" << std::endl;
	}

	void UtlXMLStream::writeHashtableEntry(const std::string& sKey, const std::vector<std::string>& listOfValues) {
		getOutputStream() << _sIndentation.c_str() << "<pair key=\"" << sKey.c_str() << "\" >" << std::endl;
		_sIndentation += "\t";
		for (std::vector<std::string>::const_iterator i = listOfValues.begin(); i != listOfValues.end(); i++) {
			getOutputStream() << _sIndentation.c_str() << "<string value=\"" << i->c_str() << "\" >" << std::endl;
		}
		_sIndentation = _sIndentation.substr(1);
		getOutputStream() << _sIndentation.c_str() << "</pair>" << std::endl;
	}

	void UtlXMLStream::writeHashtableEntry(int iKey, int iValue) {
		getOutputStream() << _sIndentation.c_str() << "<pair key=\"" << iKey << "\" value =\"" << iValue << "\" />" << std::endl;
	}

	void UtlXMLStream::writeBeginningOfAggregation(const std::string& sOwnerClass, const std::string& sName) {
		getOutputStream() << _sIndentation.c_str() << "<" << sOwnerClass.c_str() << "_" << sName.c_str() << ">" << std::endl;
		_sIndentation += "\t";
	}

	void UtlXMLStream::writeEndOfAggregation(const std::string& sOwnerClass, const std::string& sName) {
		_sIndentation = _sIndentation.substr(1);
		getOutputStream() << _sIndentation.c_str() << "</" << sOwnerClass.c_str() << "_" << sName.c_str() << ">" << std::endl;
	}

	void UtlXMLStream::writeBeginningOfAssociation(const std::string& sOwnerClass, const std::string& sName) {
		getOutputStream() << _sIndentation.c_str() << "<" << sOwnerClass.c_str() << "_" << sName.c_str() << ">" << std::endl;
		_sIndentation += "\t";
	}

	void UtlXMLStream::writeEndOfAssociation(const std::string& sOwnerClass, const std::string& sName) {
		_sIndentation = _sIndentation.substr(1);
		getOutputStream() << _sIndentation.c_str() << "</" << sOwnerClass.c_str() << "_" << sName.c_str() << ">" << std::endl;
	}

	void UtlXMLStream::writeObjectReference(const std::string& sTypeName, const std::string& /*sIDAttrName*/, const std::string& sIdentifier) {
		getOutputStream() << _sIndentation.c_str() << "<reference type=\"" << sTypeName.c_str() << "\" ID=\"" << sIdentifier.c_str() << "\" />" << std::endl;
	}

	std::string UtlXMLStream::convertToXMLText(const std::string& sText) {
		std::string sResult;
		for (std::string::size_type i = 0; i < sText.size(); i++) {
			char a = sText[i];
			switch(a) {
				case '&': sResult += "&amp;";break;
				case '<': sResult += "&lt;";break;
				case '>': sResult += "&gt;";break;
				case '\"': sResult += "&quot;";break;
				default:
					sResult += a;
					break;
			}
		}
		return sResult;
	}


	// read part
	bool UtlXMLStream::readStartTag( std::string& sTag) {
		int iChar;

		skipBlanks( getInputStream() );

		iChar=readChar(  getInputStream() );
		if (iChar != '<')  {
			return false;
		}

		if (!readWord(  getInputStream(), sTag ))
			return false;

		iChar=readChar(  getInputStream() );
		if (iChar != '>')  {
			return false;
		}

		return true;
	}

	bool UtlXMLStream::readEndTag(std::string& sTag) {
		int iChar;
		
		skipBlanks( getInputStream() );


		iChar=readChar(  getInputStream() );
		if (iChar != '<')  {
			return false;
		}

		iChar=readChar(  getInputStream() );
		if (iChar != '/')  {
			return false;
		}

		if (!readWord(  getInputStream(), sTag ))
			return false;

		iChar=readChar(  getInputStream() );
		if (iChar != '>')  {
			return false;
		}

		return true;
	}

	bool UtlXMLStream::readTag(std::string& sTag) {
		int iChar;
		
		skipBlanks( getInputStream() );
		
		iChar=readChar(  getInputStream() );
		if (iChar != '<')  {
			return false;
		}

		if (!readWord(  getInputStream(), sTag ))
			return false;

		if (iChar != '/')  {
			return false;
		}

		iChar=readChar(  getInputStream() );
		if (iChar != '>')  {
			return false;
		}

		return true;
	}




	bool UtlXMLStream::readBeginningOfObject(std::string& sTypeName) {

		int iChar;
		
		skipBlanks( getInputStream() );

		iChar=readChar(  getInputStream() );
		if (iChar != '<')  {
			return false;
		}

		if (!readWord(  getInputStream(), sTypeName ))
			return false;

		return true;
	}


	bool UtlXMLStream::readName(std::string& sName) {
		skipBlanks( getInputStream() );


		std::string sNonNormaliseName;
		if (!readWord(  getInputStream(), sNonNormaliseName ))
			return false;
		
		sName = normalizeAttributeName(sNonNormaliseName);

		return true;
	}

	bool UtlXMLStream::readBeginningOfAttribute() {
		int iChar;

		skipBlanks( getInputStream() );

		iChar=readChar(  getInputStream() );
		if (iChar != '=')  {
			return false;
		}

		iChar=readChar(  getInputStream() );
		if (iChar != '\"')  {
			return false;
		}

		return true;
	}



	bool UtlXMLStream::readEndOfAttribute() {
		int iChar;
		skipBlanks( getInputStream() );
		iChar=readChar(  getInputStream() );
		if (iChar != '\"')  {
			return false;
		}

		return true;
	}


	bool UtlXMLStream::readEndOfObject( std::string& sTypeName) {
		int iChar;
		skipBlanks( getInputStream() );
		iChar=readChar(  getInputStream() );
		if (iChar != '<')  {
			return false;
		}

		iChar=readChar(  getInputStream() );
		if (iChar != '/')  {
			return false;
		}

		if (!readWord(  getInputStream(), sTypeName ))
			return false;
		
		iChar=readChar(  getInputStream() );
		if (iChar != '>')  {
			return false;
		}
		return true;
	}

	bool UtlXMLStream::readEndOfObject() {

		int iChar;
		skipBlanks( getInputStream() );
		iChar=readChar(  getInputStream() );
		if (iChar != '/')  {
			return false;
		}

		iChar=readChar(  getInputStream() );
		if (iChar != '>')  {
			return false;
		}

		return true;
	}

	bool UtlXMLStream::readAttribute(std::string& sName, int& iValue) {
		
		skipBlanks( getInputStream() );
		if (!readName(sName))
			return false;
		
		if (!readBeginningOfAttribute())
			return false;
		
		if (!readInt(  getInputStream() , iValue))
			return false;

		if (!readEndOfAttribute())
			return false;
			
		return true;
	}

	bool UtlXMLStream::readAttribute(std::string& sName, long& lValue) {
		skipBlanks( getInputStream() );
		if (!readName(sName))
			return false;
		
		if (!readBeginningOfAttribute())
			return false;
		
		if (!readLong(  getInputStream() , lValue))
			return false;

		if (!readEndOfAttribute())
			return false;
		
		return true;
	}

	bool UtlXMLStream::readAttribute(std::string& sName, double& dValue) {
		skipBlanks( getInputStream() );
		if (!readName(sName))
			return false;
		
		if (!readBeginningOfAttribute())
			return false;
		
		if (!readDouble(  getInputStream() , dValue))
			return false;

		if (!readEndOfAttribute())
			return false;
		
		return true;
	}

	bool UtlXMLStream::readAttribute(std::string& sName, std::string& sValue) {
		skipBlanks( getInputStream() );
		
		if (!readName(sName))
			return false;


		// if (!readBeginningOfAttribute())
		//	return false;
		
		// std::string sXMLText;
		// if (!readName( sXMLText))
		//	return false;
		
		int iChar=readChar(  getInputStream() );
		if (iChar != '=')  {
			return false;
		}

		std::string sXMLText;
		if (!readString( getInputStream(), sXMLText))
			return false;
		
		sValue=convertXMLTextToClassicText( sXMLText );

		// if (!readEndOfAttribute())
		//	return false;

		return true;
	}

	bool UtlXMLStream::readAttribute(std::string& sName, bool& bValue) {
		
		skipBlanks( getInputStream() );

		if (!readName(sName))
			return false;
		
		if (!readBeginningOfAttribute())
			return false;
		
		std::string sXMLText;
		if (!readString(  getInputStream() , sXMLText))
			return false;
			
		if (stricmp(sXMLText.c_str(),"TRUE"))
			bValue=true;
		if (stricmp(sXMLText.c_str(),"FALSE"))
			bValue=false;
		else 
			return false;

		if (!readEndOfAttribute())
			return false;

		return true;
	}

	bool UtlXMLStream::readAttribute(std::string& sName, UtlDate& myValue) {
		
		skipBlanks( getInputStream() );

		if (!readName(sName))
			return false;
		
		if (!readBeginningOfAttribute())
			return false;
		
		struct tm tmValue;
		if (!readDate(  getInputStream() , tmValue))
			return false;
		myValue=UtlDate( tmValue.tm_year, tmValue.tm_mon, tmValue.tm_mday, 0, 0, 0);
		

		if (!readEndOfAttribute())
			return false;
		
		return true;
	}

	bool UtlXMLStream::readEndOfAttributes() {
		int iChar;
		skipBlanks( getInputStream() );
		iChar=readChar(  getInputStream() );
		if (iChar != '>')  {
			return false;
		}

		return true;
	}

	bool UtlXMLStream::readArrayElement(int& iValue) {
		skipBlanks( getInputStream() );
		if (!readBeginOfArrayElement("int"))
			return false;

		if (!readInt(  getInputStream() , iValue))
			return false;

		if (!readEndOfArrayElement())
			return false;

		return true;
	}

	bool UtlXMLStream::readArrayElement(double& dValue) {
		
		skipBlanks( getInputStream() );
		if (!readBeginOfArrayElement("double"))
			return false;

		if (!readDouble(  getInputStream() , dValue))
			return false;

		if (!readEndOfArrayElement())
			return false;

		return true;
	}

	bool UtlXMLStream::readArrayElement(std::string& sValue) {
		
		skipBlanks( getInputStream() );

		if (!readBeginOfArrayElement("string"))
			return false;

		std::string sXMLText;
		if (!readString(  getInputStream() , sXMLText))
			return false;
		sValue=convertXMLTextToClassicText( sXMLText );

		if (!readEndOfArrayElement())
			return false;
		
		return true;
	}

	bool UtlXMLStream::readArrayElement(bool& bValue) {
		
		skipBlanks( getInputStream() );

		if (!readBeginOfArrayElement("bool"))
			return false;
		
		std::string sXMLText;
		if (!readString(  getInputStream() , sXMLText))
			return false;
			
		if (stricmp(sXMLText.c_str(),"TRUE"))
			bValue=true;
		if (stricmp(sXMLText.c_str(),"FALSE"))
			bValue=false;
		else 
			return false;

		if (!readEndOfArrayElement())
			return false;

		return true;
	}

	bool UtlXMLStream::readArrayElement(UtlDate& myValue) {
		
		skipBlanks( getInputStream() );
		if (!readBeginOfArrayElement("date"))
			return false;

		struct tm tmValue;
		if (!readDate(  getInputStream() , tmValue))
			return false;
		myValue=UtlDate( tmValue.tm_year, tmValue.tm_mon, tmValue.tm_mday, 0, 0, 0);

		if (!readEndOfArrayElement())
			return false;
		return true;
	}


	bool UtlXMLStream::readKeyOfHashTable(std::string& sKey) {
		
		int iChar;
		skipBlanks( getInputStream() );

		iChar=readChar(  getInputStream() );
		if (iChar != '<')  {
			return false;
		}

		std::string sTrash;
		if (!readString( getInputStream() , sTrash))
			return false;
		if (strcmp(sTrash.c_str(),"pair"))
			return false;

		std::string sKeyTag;
		if (!readAttribute( sKeyTag , sKey))
			return false;

		if (strcmp(sKeyTag.c_str(),"key"))
			return false;
		
		return true;
	}


	bool UtlXMLStream::readEndOfHashTable() {
		
		int iChar;

		skipBlanks( getInputStream() );

		iChar=readChar(  getInputStream() );
		if (iChar != '/')  {
			return false;
		}

		iChar=readChar(  getInputStream() );
		if (iChar != '>')  {
			return false;
		}

		return true;
	}



	bool UtlXMLStream::readHashtableEntry(std::string& sKey, std::string& sValue) {
		int iChar;

		skipBlanks( getInputStream() );
		if (!readKeyOfHashTable(sKey))
			return false;

		iChar=readChar(  getInputStream() );
		if (iChar != ' ')  {
			return false;
		}

		std::string sValueTag;
		if (!readAttribute(  sValueTag , sValue))
			return false;

		if (!readEndOfHashTable())
			return false;

		return true;
	}


	bool UtlXMLStream::readBeginOfArrayElement(std::string sType) {

		int iChar;
		skipBlanks( getInputStream() );
		iChar=readChar(  getInputStream() );
		if (iChar != '<')  {
			return false;
		}

		std::string sReadType;
		if (!readString( getInputStream(), sReadType))
		if (sReadType!=sType) 
				return false;
		
		std::string sValue;
		if (!readString( getInputStream(), sValue))
			return false;

		iChar=readChar(  getInputStream() );
		if (iChar != ' ')  {
			return false;
		}

		if (strcmp(sValue.c_str(),"value") )
			return false;

		return true;
	}

	bool UtlXMLStream::readEndOfArrayElement() {
		int iChar;


		skipBlanks( getInputStream() );
		iChar=readChar(  getInputStream() );
		if (iChar != '/')  {
			return false;
		}

		iChar=readChar(  getInputStream() );
		if (iChar != '>')  {
			return false;
		}
		return true;
	}


	bool UtlXMLStream::readHashtableEntry(std::string& sKey, double& dValue) {
		int iChar;
		skipBlanks( getInputStream() );
		if (!readKeyOfHashTable(sKey))
			return false;

		iChar=readChar(  getInputStream() );
		if (iChar != '>')  {
			return false;
		}

		std::string sValueTag;
		if (!readAttribute(  sValueTag , dValue))
			return false;

		if (!readEndOfHashTable())
			return false;

		return true;
	}

	bool UtlXMLStream::readHashtableEntry(std::string& sKey, std::vector<double>& listOfValues) {
		int iChar;
		skipBlanks( getInputStream() );
		if (!readKeyOfHashTable(sKey))
			return false;

		iChar=readChar(  getInputStream() );
		if (iChar != ' ')  {
			return false;
		}

		iChar=readChar(  getInputStream() );
		if (iChar != '>')  {
			return false;
		}

		bool bAtLeastOne=false;
		bool bContinue=true;

		listOfValues.clear();
		
		while (bContinue) {
			double dValue;
			
			int iPos = getInputStream().tellg();
			
			bContinue=readArrayElement(dValue);
			if (bContinue) {
				listOfValues.push_back(dValue);
				bAtLeastOne=true;
			} 
			else {
				getInputStream().seekg(iPos);
			}
		}
		
		if (!bAtLeastOne) {
			return false;
		}

		if (!readEndOfHashTable())
			return false;

		return true;
	}

	bool UtlXMLStream::readHashtableEntry(std::string& sKey, std::vector<std::string>& listOfValues) {
		int iChar;
		skipBlanks( getInputStream() );
		if (!readKeyOfHashTable(sKey))
			return false;

		iChar=readChar(  getInputStream() );
		if (iChar != ' ')  {
			return false;
		}

		iChar=readChar(  getInputStream() );
		if (iChar != '>')  {
			return false;
		}

		bool bAtLeastOne=false;
		bool bContinue=true;

		listOfValues.clear();
		
		while (bContinue) {
			int iPos = getInputStream().tellg();
			std::string sValue;
			
			bContinue=readArrayElement(sValue);
			if (bContinue) {
				listOfValues.push_back(sValue);
				bAtLeastOne=true;
			} 
			else {
				getInputStream().seekg(iPos);
			}
		}
		
		if (!bAtLeastOne) {
			return false;
		}

		if (!readEndOfHashTable())
			return false;
		
		return true;
	}

	bool UtlXMLStream::readBeginningOfAggregation(std::string& sOwnerClass, std::string& sName) {
		return readBeginningOfAssociation( sOwnerClass, sName);
	}

	bool UtlXMLStream::readEndOfAggregation(std::string& sOwnerClass, std::string& sName) {
		return readEndOfAssociation( sOwnerClass, sName);
	}


	bool UtlXMLStream::readBeginningOfAssociation(std::string& sOwnerClass, std::string& sName) {
		int iChar;
		skipBlanks( getInputStream() );
		iChar=readChar(  getInputStream() );
		if (iChar != '<')  {
			return false;
		}
		
		// Read a string, search '_' and split the string in two
		std::string sCompletWord,sDebut,sFin;
		if (!readWord(  getInputStream(), sCompletWord ))
			return false;

		if (!splitString( sCompletWord, '_', sOwnerClass, sName))
			return false;
		
		iChar=readChar(  getInputStream() );
		if (iChar != '>')  {
			return false;
		}

		return true;
	}

	bool UtlXMLStream::readEndOfAssociation(std::string& sOwnerClass, std::string& sName) {
		
		int iChar;
		skipBlanks( getInputStream() );
		iChar=readChar(  getInputStream() );
		if (iChar != '<')  {
			return false;
		}

		iChar=readChar(  getInputStream() );
		if (iChar != '/')  {
			return false;
		}

		// Read a string, search '_' and split the string in two
		std::string sCompletWord,sDebut,sFin;
		if (!readWord(  getInputStream(), sCompletWord ))
			return false;

		if (!splitString( sCompletWord, '_', sOwnerClass, sName))
			return false;
		
		iChar=readChar(  getInputStream() );
		if (iChar != '>')  {
			return false;
		}
		
		return true;
	}


	bool UtlXMLStream::readObjectReference(std::string& sTypeName, std::string& sIDAttrName, std::string& sIdentifier) {

		int iChar;
		skipBlanks( getInputStream() );
		iChar=readChar(  getInputStream() );
		if (iChar != '<')  {
			return false;
		}
		
		std::string sTrash;
		// reference
		if (!readString( getInputStream(), sTrash)) 
			return false;
		if (strcmp(sTrash.c_str(),"reference"))
			return false;

		// type 
		if (!readAttribute(sTrash, sTypeName))
			return false;

		// ID
		if (!readAttribute( sIDAttrName, sIdentifier)) 
			return false;
		
		iChar=readChar(  getInputStream() );
		if (iChar != '/')  {
			return false;
		}
		
		iChar=readChar(  getInputStream() );
		if (iChar != '>')  {
			return false;
		}

		return true;	
	}

	std::string UtlXMLStream::convertXMLTextToClassicText(const std::string& sText) {
		std::string sResult,sResultInter;
		
		for (std::string::size_type i = 0; i < sText.size(); i++) {
			char a = sText[i];
			switch(a) {
				case '&': sResultInter += "&";break;
				case 'a': sResultInter += "a";break;
				case 'm': sResultInter += "m";break;
				case 'p': sResultInter += "p";break;
				case ';': if (!strcmp(sResultInter.c_str(),"&amp")) { 
							sResult += "&";
							sResultInter = "";
						}

							break;
				default:
					if (!sResultInter.empty()) {
						sResult+=sResultInter;
						sResultInter = "";
					}

					sResult += a;
					break;
			}
		}

		if (!sResultInter.empty()) {
			sResult+=sResultInter;
			sResultInter = "";
		}

		return sResult;
	}
}
