/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2006 Cédric Lemaire

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

#ifndef _GrfAspectAdvice_h_
#define _GrfAspectAdvice_h_

#include "GrfBlock.h"

namespace CodeWorker {
	class ExprScriptExpression;
	class DtaPatternScript;
	class GrfJointPoint;

	class GrfAspectAdvice : public GrfBlock {
	public:
		enum ADVICE_TYPE {ADVICE_BEFORE, ADVICE_AROUND, ADVICE_AFTER, ADVICE_BEFORE_ITERATION, ADVICE_AROUND_ITERATION, ADVICE_AFTER_ITERATION};

	private:
		ADVICE_TYPE _type;
		DtaPatternScript* _pPatternScript;
		std::string _sMatchingName;
		ExprScriptExpression* _pPointcut;

	public:
		inline GrfAspectAdvice(DtaPatternScript* pPatternScript, GrfBlock* pParent, ADVICE_TYPE type, const std::string& sMatchingName) : GrfBlock(pParent), _pPatternScript(pPatternScript), _type(type), _pPointcut(NULL), _sMatchingName(sMatchingName) {}
		virtual ~GrfAspectAdvice();

		inline ADVICE_TYPE getType() const { return _type; }
		inline void setMatchingName(const std::string& sMatchingName) { _sMatchingName = sMatchingName; }
		inline void setPointcut(ExprScriptExpression* pPointcut) { _pPointcut = pPointcut; }
		bool matchPointcut(GrfJointPoint& jointPoint, DtaScriptVariable& context) const;

		virtual void compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;

	protected:
		virtual SEQUENCE_INTERRUPTION_LIST executeInternal(DtaScriptVariable& visibility);
	};
}

#endif
