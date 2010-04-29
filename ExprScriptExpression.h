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

#ifndef _ExprScriptExpression_h_
#define _ExprScriptExpression_h_

#include <string>
#include <list>
#include <map>

namespace CodeWorker {
	enum EXPRESSION_RETURN_TYPE {
		NO_TYPE,
		BOOL_TYPE,
		INT_TYPE,
		DOUBLE_TYPE,
		STRING_TYPE,
		ULONG_TYPE,
		USHORT_TYPE,
		TREE_TYPE,
		ALIEN_TYPE
	};

	class DtaScriptVariable;
	class ExprScriptVariable;
	class DtaScript;
	class CppCompilerEnvironment;

	class ExprScriptExpression {
	protected:
		ExprScriptExpression() {}

	public:
		virtual ~ExprScriptExpression();
		virtual bool isAFunctionExpression() const { return false; }
		virtual std::string getValue(DtaScriptVariable& visibility) const = 0;
		virtual double getDoubleValue(DtaScriptVariable& visibility) const;
		virtual int getIntValue(DtaScriptVariable& visibility) const;
		virtual DtaScriptVariable* getReferencedVariable() const;

		virtual ExprScriptExpression* clone() const;

		virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const;
		virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppUnsignedShort(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppUnsignedLong(CppCompilerEnvironment& theCompilerEnvironment) const;

		virtual bool compileCppExpr(CppCompilerEnvironment& theCompilerEnvironment) const;

		virtual std::string toString() const;
	};


	class ExprScriptReference : public ExprScriptExpression {
	private:
		ExprScriptExpression* _pRef;
		DtaScriptVariable* _pVariable;

	public:
		ExprScriptReference(ExprScriptExpression* pRef) : _pRef(pRef), _pVariable(NULL) {}
		ExprScriptReference(DtaScriptVariable* pVariable) : _pRef(NULL), _pVariable(pVariable) {}
		virtual ~ExprScriptReference();
		virtual std::string getValue(DtaScriptVariable& visibility) const;
		virtual DtaScriptVariable* getReferencedVariable() const;
	};


	class ExprScriptUnaryMinus : public ExprScriptExpression {
	private:
		ExprScriptExpression* _pExpression;

	public:
		ExprScriptUnaryMinus(ExprScriptExpression* pExpression) : _pExpression(pExpression) {}
		virtual ~ExprScriptUnaryMinus();
		
		virtual std::string getValue(DtaScriptVariable& visibility) const;
		virtual double getDoubleValue(DtaScriptVariable& visibility) const;

		virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const;

		virtual std::string toString() const;
	};

	class ExprScriptBitwiseNot : public ExprScriptExpression {
	private:
		ExprScriptExpression* _pExpression;

	public:
		ExprScriptBitwiseNot(ExprScriptExpression* pExpression) : _pExpression(pExpression) {}
		virtual ~ExprScriptBitwiseNot();
		
		virtual std::string getValue(DtaScriptVariable& visibility) const;
		virtual int getIntValue(DtaScriptVariable& visibility) const;

		virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const;

		virtual std::string toString() const;
	};

	class ExprScriptConstant : public ExprScriptExpression {
	protected:
		std::string _sValue;

	public:
		ExprScriptConstant() {}
		ExprScriptConstant(const char* sValue) : _sValue(sValue) {}
		ExprScriptConstant(bool bValue);
		ExprScriptConstant(int iValue);
		ExprScriptConstant(double dValue);
		virtual ~ExprScriptConstant();
		
		inline const std::string& getConstant() const { return _sValue; }
		inline void setConstant(const char* tcValue, int iLength) { _sValue.assign(tcValue, iLength); }
		virtual std::string getValue(DtaScriptVariable& /*visibility*/) const;
		virtual double getDoubleValue(DtaScriptVariable& visibility) const;
		virtual int getIntValue(DtaScriptVariable& visibility) const;

		virtual ExprScriptExpression* clone() const;

		virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const;
		virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const;

		virtual std::string toString() const;
	};


	class ExprScriptConstantTree : public ExprScriptExpression {
	private:
		ExprScriptExpression* _pValue;
		std::map<std::string, ExprScriptExpression*> _listOfAttributes;
		std::list<ExprScriptExpression*> _listOfItems;
		std::list<ExprScriptExpression*> _listOfKeys;

	public:
		inline ExprScriptConstantTree() {}
		virtual ~ExprScriptConstantTree();
		
		void setValueExpression(ExprScriptExpression* pValue);
		void addAttribute(const std::string& sName, ExprScriptExpression* pTree);
		void addItem(ExprScriptExpression* pKey, ExprScriptExpression* pItem);
		void addItem(ExprScriptExpression* pItem);
		void setTree(DtaScriptVariable& visibility, DtaScriptVariable&  theVariable) const;
		virtual std::string getValue(DtaScriptVariable& /*visibility*/) const;

		virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;
		EXPRESSION_RETURN_TYPE compileCppInit(CppCompilerEnvironment& theCompilerEnvironment, ExprScriptVariable& theVariable) const;
		EXPRESSION_RETURN_TYPE compileCppInit(CppCompilerEnvironment& theCompilerEnvironment, const std::string& sVariable) const;

		virtual std::string toString() const;

	private:
		EXPRESSION_RETURN_TYPE compileCppInit(CppCompilerEnvironment& theCompilerEnvironment, const std::string& sVariable, int iDepth) const;
	};


	class ExprScriptResolvedVariable : public ExprScriptExpression {
	private:
		DtaScriptVariable* _pVariable;

	public:
		inline ExprScriptResolvedVariable(DtaScriptVariable* pVariable) : _pVariable(pVariable) {}
		virtual ~ExprScriptResolvedVariable();
		
		inline DtaScriptVariable* getVariable() const { return _pVariable; }

		virtual std::string getValue(DtaScriptVariable& /*visibility*/) const;

		virtual std::string toString() const;
	};


	class ExprScriptScriptFile : public ExprScriptExpression {
	private:
		bool _bFileName;
		union SCRIPT_BODY {
			ExprScriptExpression* _pFileName;
			DtaScript* _pBody;
		};
		SCRIPT_BODY script;

	public:
		inline ExprScriptScriptFile(ExprScriptExpression* pFileName) : _bFileName(true) { script._pFileName = pFileName; }
		inline ExprScriptScriptFile(DtaScript* pBody) : _bFileName(false) { script._pBody = pBody; }
		~ExprScriptScriptFile();

		inline bool isFileName() const { return _bFileName; }
		inline ExprScriptExpression* getFileName() const { return script._pFileName; }
		inline DtaScript* getBody() const { return script._pBody; }

		inline void release() { script._pBody = NULL; }

		virtual std::string getValue(DtaScriptVariable& visibility) const;
		virtual std::string toString() const;
	};


	class ExprScriptAlien : public ExprScriptConstant {
	private:
		std::string _sTargetLanguage;

	public:
		ExprScriptAlien(const char* tcAlien, const std::string& sTargetLanguage) : ExprScriptConstant(tcAlien), _sTargetLanguage(sTargetLanguage) {}
		virtual ~ExprScriptAlien();
		
		virtual std::string getValue(DtaScriptVariable& /*visibility*/) const;
		virtual double getDoubleValue(DtaScriptVariable& visibility) const;
		virtual int getIntValue(DtaScriptVariable& visibility) const;

		virtual ExprScriptExpression* clone() const;

		virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const;
		virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const;

		virtual std::string toString() const;
	};
}

#endif
