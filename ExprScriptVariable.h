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

#ifndef _ExprScriptVariable_h_
#define _ExprScriptVariable_h_

#include <string>

#include "ExprScriptExpression.h"

namespace CodeWorker {
	class DtaScriptVariable;
	class DynPackage;

	class ExprScriptVariable : public ExprScriptExpression {
	public:
		static unsigned char ATTRIBUTE_ACCESS;
		static unsigned char ARRAY_ACCESS;
		static unsigned char ARRAY_KEY_ACCESS;
		static unsigned char ARRAY_POSITION_ACCESS;
		static unsigned char NEXT_ACCESS;
		static unsigned char EVALUATION_ACCESS;

	protected:
		std::string _sName;
		ExprScriptVariable* _pNext;
		unsigned char _iAccess;
		ExprScriptExpression* _pExpression;
		DynPackage* _pPackage;

	public:
		ExprScriptVariable(ExprScriptVariable* pParent, const char* sName) : _sName(sName), _pNext(NULL), _pExpression(NULL), _iAccess(ATTRIBUTE_ACCESS), _pPackage(NULL) { pParent->setNext(this); }
		ExprScriptVariable(const char* sName, DynPackage* pPackage = NULL) : _sName(sName), _pNext(NULL), _pExpression(NULL), _iAccess(ATTRIBUTE_ACCESS), _pPackage(pPackage) {}
		virtual ~ExprScriptVariable();

		inline bool isAttributeOnly() const { return _iAccess == ATTRIBUTE_ACCESS; }
		inline bool isAttribute() const { return (_iAccess & ATTRIBUTE_ACCESS) != 0; }
		inline const std::string& getName() const { return _sName; }
		void setName(const std::string& sName) { _sName = sName; }
		inline bool isNext() const { return (_iAccess & NEXT_ACCESS) != 0; }
		inline ExprScriptVariable* getNext() const { return _pNext; }
		void setNext(ExprScriptVariable* pNext) { _pNext = pNext;_iAccess |= NEXT_ACCESS; }
		inline ExprScriptExpression* getExpression() const { return _pExpression; }
		inline bool isArray() const { return (_iAccess & ARRAY_ACCESS) == ARRAY_ACCESS; }
		inline bool isArrayKey() const { return (_iAccess & ARRAY_KEY_ACCESS) == ARRAY_KEY_ACCESS; }
		inline ExprScriptExpression* getArrayKey() const { return ((_iAccess & ARRAY_KEY_ACCESS) == ARRAY_KEY_ACCESS) ? _pExpression : NULL; }
		void setArrayKey(ExprScriptExpression* pArrayKey) { _pExpression = pArrayKey;_iAccess |= ARRAY_KEY_ACCESS; }
		inline bool isArrayPosition() const { return (_iAccess & ARRAY_POSITION_ACCESS) == ARRAY_POSITION_ACCESS; }
		inline ExprScriptExpression* getArrayPosition() const { return ((_iAccess & ARRAY_POSITION_ACCESS) == ARRAY_POSITION_ACCESS) ? _pExpression : NULL; }
		void setArrayPosition(ExprScriptExpression* pArrayPosition) { _pExpression = pArrayPosition;_iAccess |= ARRAY_POSITION_ACCESS; }
		inline bool isEvaluation() const { return (_iAccess & EVALUATION_ACCESS) == EVALUATION_ACCESS; }
		inline ExprScriptExpression* getEvaluation() const { return ((_iAccess & EVALUATION_ACCESS) == EVALUATION_ACCESS) ? _pExpression : NULL; }
		void setEvaluation(ExprScriptExpression* pEvaluation) { _pExpression = pEvaluation;_iAccess &= ~ATTRIBUTE_ACCESS;_iAccess |= EVALUATION_ACCESS; }
		inline DynPackage* getPackage() const { return _pPackage; }
		void setPackage(DynPackage* pPackage) { _pPackage = pPackage; }

		virtual std::string getValue(DtaScriptVariable& visibility) const;

		virtual ExprScriptExpression* clone() const;

		virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const;
		virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const;

		virtual void compileCppForGetOrCreateLocal(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual void compileCppForSet(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppExpr(CppCompilerEnvironment& theCompilerEnvironment) const;

		virtual void compileCppForBNFSet(CppCompilerEnvironment& theCompilerEnvironment) const;

		virtual std::string toString() const;

	protected:
		virtual void compileCppFollowing(CppCompilerEnvironment& theCompilerEnvironment, bool bFirstCall = false) const;
		virtual void compileCppFollowingForSet(CppCompilerEnvironment& theCompilerEnvironment, bool bFirstCall = false) const;
	};


	class ExprScriptAlienVariable : public ExprScriptVariable {
	private:
		std::string _sTargetLanguage;

	public:
		ExprScriptAlienVariable(ExprScriptVariable* pParent, const char* tcCompleteName, const std::string& sTargetLanguage) : ExprScriptVariable(pParent, tcCompleteName), _sTargetLanguage(sTargetLanguage) {}
		ExprScriptAlienVariable(const char* tcCompleteName, const std::string& sTargetLanguage) : ExprScriptVariable(tcCompleteName), _sTargetLanguage(sTargetLanguage) {}
		virtual ~ExprScriptAlienVariable();

		inline const std::string& getTargetLanguage() const { return _sTargetLanguage; }

		virtual std::string getValue(DtaScriptVariable& visibility) const;

		virtual ExprScriptExpression* clone() const;

		virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const;
		virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const;

		virtual void compileCppForGetOrCreateLocal(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual void compileCppForSet(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppExpr(CppCompilerEnvironment& theCompilerEnvironment) const;

		virtual void compileCppForBNFSet(CppCompilerEnvironment& theCompilerEnvironment) const;

		virtual std::string toString() const;

	protected:
		virtual void compileCppFollowing(CppCompilerEnvironment& theCompilerEnvironment, bool bFirstCall = false) const;
		virtual void compileCppFollowingForSet(CppCompilerEnvironment& theCompilerEnvironment, bool bFirstCall = false) const;
	};
}

#endif

