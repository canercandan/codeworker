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

#ifndef _UtlXMLStream_h_
#define _UtlXMLStream_h_

#include <string>
#include <vector>
#include <fstream>


namespace CodeWorker {
	class UtlDate;

	class UtlXMLStream {

		// common part
		public:
			
			// mode write default bec
			UtlXMLStream(const std::string& sFileName, const bool bModeRead );
			UtlXMLStream(std::iostream& myStream);

			virtual ~UtlXMLStream();

			inline const std::string& getFileName() const { return _sFileName; }
			virtual std::string normalizeAttributeName(const std::string& sName) const;
			
		// write part
		private:
			std::ostream* _pOutputStream;
			std::string _sIndentation;
			bool _bOwnerOfFileStream;

		public:
			inline UtlXMLStream(std::ostream& myStream) : _pOutputStream(&myStream), _bOwnerOfFileStream(false) {}
			
			inline std::ostream& getOutputStream() const { return *_pOutputStream; }

			virtual void writeStartTag(const std::string& sTag);
			virtual void writeEndTag(const std::string& sTag);
			virtual void writeTag(const std::string& sTag);
			virtual void writeBeginningOfObject(const std::string& sTypeName);
			virtual void writeEndOfObject(const std::string& sTypeName);
			virtual void writeEndOfObject();
			virtual void writeAttribute(const std::string& sName, int iValue);
			virtual void writeAttribute(const std::string& sName, long lValue);
			virtual void writeAttribute(const std::string& sName, double dValue);
			virtual void writeAttribute(const std::string& sName, const std::string& sValue);
			virtual void writeAttribute(const std::string& sName, bool bValue);
			virtual void writeAttribute(const std::string& sName, const UtlDate& myValue);
			virtual void writeEndOfAttributes();
			virtual void writeArrayElement(int iValue);
			virtual void writeArrayElement(double dValue);
			virtual void writeArrayElement(const std::string& sValue);
			virtual void writeArrayElement(bool bValue);
			virtual void writeArrayElement(const UtlDate& myValue);
			virtual void writeHashtableEntry(const std::string& sKey, const std::string& sValue);
			virtual void writeHashtableEntry(const std::string& sKey, double dValue);
			virtual void writeHashtableEntry(const std::string& sKey, const std::vector<double>& listOfValues);
			virtual void writeHashtableEntry(const std::string& sKey, const std::vector<std::string>& listOfValues);
			virtual void writeHashtableEntry(int iKey, int iValue);
			virtual void writeBeginningOfAggregation(const std::string& sOwnerClass, const std::string& sName);
			virtual void writeEndOfAggregation(const std::string& sOwnerClass, const std::string& sName);
			virtual void writeBeginningOfAssociation(const std::string& sOwnerClass, const std::string& sName);
			virtual void writeEndOfAssociation(const std::string& sOwnerClass, const std::string& sName);
			virtual void writeObjectReference(const std::string& sTypeName, const std::string& /*sIDAttrName*/, const std::string& sIdentifier);

			std::string convertToXMLText(const std::string& sText);


		// Read part
		private:
			std::istream* _pInputStream;
			std::string _sFileName;

		public:
			UtlXMLStream(std::istream& myStream);
			inline std::istream& getInputStream() const { return *_pInputStream; }
			
			virtual bool readStartTag( std::string& sTag);
			virtual bool readEndTag( std::string& sTag);
			virtual bool readTag( std::string& sTag);
			virtual bool readBeginningOfObject( std::string& sTypeName);
			virtual bool readEndOfObject( std::string& sTypeName);
			virtual bool readEndOfObject();
			virtual bool readAttribute( std::string& sName, int& iValue);
			virtual bool readAttribute( std::string& sName, long& lValue);
			virtual bool readAttribute( std::string& sName, double& dValue);
			virtual bool readAttribute( std::string& sName,  std::string& sValue);
			virtual bool readAttribute( std::string& sName, bool& bValue);
			virtual bool readAttribute( std::string& sName,  UtlDate& myValue);
			virtual bool readEndOfAttributes();
			virtual bool readArrayElement(int& iValue);
			virtual bool readArrayElement(double& dValue);
			virtual bool readArrayElement( std::string& sValue);
			virtual bool readArrayElement(bool& bValue);
			virtual bool readArrayElement( UtlDate& myValue);
			virtual bool readHashtableEntry( std::string& sKey,  std::string& sValue);
			virtual bool readHashtableEntry( std::string& sKey, double& dValue);
			virtual bool readHashtableEntry( std::string& sKey,  std::vector<double>& listOfValues);
			virtual bool readHashtableEntry( std::string& sKey,  std::vector<std::string>& listOfValues);
			virtual bool readBeginningOfAggregation( std::string& sOwnerClass,  std::string& sName);
			virtual bool readEndOfAggregation( std::string& sOwnerClass,  std::string& sName);
			virtual bool readBeginningOfAssociation( std::string& sOwnerClass,  std::string& sName);
			virtual bool readEndOfAssociation( std::string& sOwnerClass,  std::string& sName);
			virtual bool readObjectReference( std::string& sTypeName,  std::string& sIDAttrName,  std::string& sIdentifier);

			
			
		private:
			std::string convertXMLTextToClassicText(const std::string& sText);

			bool readKeyOfHashTable(std::string& sKey);
			bool readBeginningOfAttribute() ;
			bool readEndOfAttribute() ;
			bool readEndOfHashTable() ;
			bool readName(std::string& sName) ;
			bool readBeginOfArrayElement(std::string sType) ;
			bool readEndOfArrayElement() ;
	};
}

#endif
