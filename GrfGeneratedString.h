#ifndef _GrfGeneratedString_h_
#define _GrfGeneratedString_h_

#include "GrfBlock.h"

namespace CodeWorker {
	class ExprScriptVariable;
	class DtaPatternScript;

	class GrfGeneratedString : public GrfBlock {
	private:
		DtaPatternScript* _pPatternScript;
		ExprScriptVariable* _pOutputString;

	public:
		GrfGeneratedString(DtaPatternScript* pPatternScript, GrfBlock* pParent) : GrfBlock(pParent), _pPatternScript(pPatternScript), _pOutputString(NULL) {}
		virtual ~GrfGeneratedString();

		inline void setOutputString(ExprScriptVariable* pOutputString) { _pOutputString = pOutputString; }
		virtual void compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;

	protected:
		virtual SEQUENCE_INTERRUPTION_LIST executeInternal(DtaScriptVariable& visibility);
	};
}

#endif
