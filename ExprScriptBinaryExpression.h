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

#ifndef _ExprScriptBinaryExpression_h_
#define _ExprScriptBinaryExpression_h_

#include "ExprScriptExpression.h"


namespace CodeWorker {
	class ExprScriptBinaryExpression : public ExprScriptExpression {
	private:
		ExprScriptExpression* _pLeft;
		ExprScriptExpression* _pRight;

	public:
		ExprScriptBinaryExpression(ExprScriptExpression* pLeft, ExprScriptExpression* pRight) : _pLeft(pLeft), _pRight(pRight) {}
		virtual ~ExprScriptBinaryExpression();

		inline ExprScriptExpression* getLeft() const { return _pLeft; }
		inline ExprScriptExpression* getRight() const { return _pRight; }

		virtual std::string getValue(DtaScriptVariable& visibility) const = 0;
	};


	class ExprScriptConcatenation : public ExprScriptBinaryExpression {
	public:
		ExprScriptConcatenation(ExprScriptExpression* pLeft, ExprScriptExpression* pRight) : ExprScriptBinaryExpression(pLeft, pRight) {}
		virtual ~ExprScriptConcatenation();
		virtual std::string getValue(DtaScriptVariable& visibility) const;

		virtual std::string toString() const;

		virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const;
	};


	class ExprScriptArithmetic : public ExprScriptBinaryExpression {
	private:
		char _cOperator;

	public:
		ExprScriptArithmetic(ExprScriptExpression* pLeft, ExprScriptExpression* pRight, char cOperator) : ExprScriptBinaryExpression(pLeft, pRight), _cOperator(cOperator) {}
		virtual ~ExprScriptArithmetic();
		virtual std::string getValue(DtaScriptVariable& visibility) const;
		virtual int getIntValue(DtaScriptVariable& visibility) const;
		virtual double getDoubleValue(DtaScriptVariable& visibility) const;

		virtual std::string toString() const;

		virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const;
	};


	class ExprScriptShift : public ExprScriptBinaryExpression {
	private:
		char _cOperator;

	public:
		ExprScriptShift(ExprScriptExpression* pLeft, ExprScriptExpression* pRight, char cOperator) : ExprScriptBinaryExpression(pLeft, pRight), _cOperator(cOperator) {}
		virtual ~ExprScriptShift();
		virtual std::string getValue(DtaScriptVariable& visibility) const;
		virtual int getIntValue(DtaScriptVariable& visibility) const;

		virtual std::string toString() const;

		virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const;
	};


	class ExprScriptComparison : public ExprScriptBinaryExpression {
	protected:
		char _cOperator;

	public:
		ExprScriptComparison(ExprScriptExpression* pLeft, ExprScriptExpression* pRight, char cOperator) : ExprScriptBinaryExpression(pLeft, pRight), _cOperator(cOperator) {}
		virtual ~ExprScriptComparison();
		virtual std::string getValue(DtaScriptVariable& visibility) const;

		virtual std::string toString() const;

		virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const;
	};


	class ExprScriptNumericComparison : public ExprScriptComparison {
	public:
		ExprScriptNumericComparison(ExprScriptExpression* pLeft, ExprScriptExpression* pRight, char cOperator) : ExprScriptComparison(pLeft, pRight, cOperator) {}
		virtual ~ExprScriptNumericComparison();
		virtual std::string getValue(DtaScriptVariable& visibility) const;

		virtual std::string toString() const;

		virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const;
	};


	class ExprScriptTernaryOperator : public ExprScriptBinaryExpression {
	protected:
		ExprScriptExpression* _pCondition;

	public:
		ExprScriptTernaryOperator(ExprScriptExpression* pCondition, ExprScriptExpression* pLeft, ExprScriptExpression* pRight) : ExprScriptBinaryExpression(pLeft, pRight), _pCondition(pCondition) {}
		virtual ~ExprScriptTernaryOperator();
		virtual std::string getValue(DtaScriptVariable& visibility) const;

		virtual std::string toString() const;

		virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const;
	};

	class ExprScriptBoolean : public ExprScriptBinaryExpression {
	protected:
		char _cOperator;

	public:
		ExprScriptBoolean(ExprScriptExpression* pLeft, ExprScriptExpression* pRight, char cOperator) : ExprScriptBinaryExpression(pLeft, pRight), _cOperator(cOperator) {}
		virtual ~ExprScriptBoolean();
		virtual std::string getValue(DtaScriptVariable& visibility) const;

		virtual std::string toString() const;

		virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const;
	};

	class ExprScriptInSet : public ExprScriptExpression {
	private:
		ExprScriptExpression* _pLeft;
		std::set<std::string> _listOfConstants;

	public:
		ExprScriptInSet(ExprScriptExpression* pLeft, const std::set<std::string>& listOfConstants) : _pLeft(pLeft), _listOfConstants(listOfConstants) {}
		virtual ~ExprScriptInSet();
		
		virtual std::string getValue(DtaScriptVariable& visibility) const;

		virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;

		virtual std::string toString() const;
	};
}

#endif
