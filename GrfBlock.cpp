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
#include "GrfFunction.h"
#include "DtaClass.h"
#include "CppCompilerEnvironment.h"
#include "DtaScript.h" // declaration of 'GrfScriptBlock'
#include "GrfBlock.h"

namespace CodeWorker {
	GrfBlock::~GrfBlock() {
		clear();
	}

	DtaScript* GrfBlock::getScript() const {
		const GrfBlock* pBlock = this;
		while (pBlock->_pScript == NULL) {
			pBlock = pBlock->getParent();
			if (pBlock == NULL) return NULL;
		}
		return pBlock->_pScript;
	}

	void GrfBlock::clear() {
		for (std::vector<GrfCommand*>::iterator i = _commands.begin(); i != _commands.end(); i++) {
			delete (*i);
		}
		if (_functions != NULL) {
			for (std::list<GrfCommand*>::iterator j = _functions->begin(); j != _functions->end(); j++) {
				GrfCommand* pFunction = *j;
				*j = NULL;
				delete pFunction;
			}
			delete _functions;
		}
		if (_classes != NULL) {
			for (std::list<DtaClass*>::iterator j = _classes->begin(); j != _classes->end(); j++) {
				DtaClass* pClass = *j;
				*j = NULL;
				delete pClass;
			}
			delete _classes;
		}
		delete _variables;
		_commands = std::vector<GrfCommand*>();
		_functions = NULL;
		_classes = NULL;
		_variables = NULL;
	}

	void GrfBlock::applyRecursively(APPLY_ON_COMMAND_FUNCTION apply) {
		GrfCommand::applyRecursively(apply);
		for (std::vector<GrfCommand*>::iterator i = _commands.begin(); i != _commands.end(); i++) (*i)->applyRecursively(apply);
		if (_functions != NULL) {
			for (std::list<GrfCommand*>::iterator i = _functions->begin(); i != _functions->end(); i++) (*i)->applyRecursively(apply);
		}
	}

	void GrfBlock::add(GrfCommand* pCommand) {
		pCommand->setParent(this);
		_commands.push_back(pCommand);
	}

	void GrfBlock::setCommand(int i, GrfCommand* pCommand) {
		_commands[i] = pCommand;
		pCommand->setParent(this);
	}

	void GrfBlock::moveCommands(GrfBlock& destinationBlock) {
		while (!_commands.empty()) {
			GrfCommand* pCommand =  _commands.back();
			pCommand->setParent(&destinationBlock);
			destinationBlock._commands.push_back(pCommand);
			_commands.pop_back();
		}
	}

	bool GrfBlock::removeCommand(GrfCommand* pCommand) {
		for (std::vector<GrfCommand*>::iterator i = _commands.begin(); i != _commands.end(); i++) {
			if (*i == pCommand) {
				delete *i;
				_commands.erase(i);
				return true;
			}
		}
		return false;
	}

	GrfFunction* GrfBlock::addFunction(const std::string& sFunctionName, const std::string& sTemplate, bool bGenericKey) {
		GrfFunction* pFunction = getFunction(sFunctionName, sTemplate, bGenericKey);
		if (pFunction != NULL) return NULL;
		pFunction = new GrfFunction(this, sFunctionName, sTemplate, bGenericKey);
		if (_functions == NULL) _functions = new std::list<GrfCommand*>;
		_functions->push_back(pFunction);
		return pFunction;
	}

	GrfFunction* GrfBlock::addFunction(GrfFunction* pFunction) {
		if (getFunction(pFunction->getFunctionName(), pFunction->getTemplateInstantiation(), pFunction->isGenericKey()) != NULL) return NULL;
		if (_functions == NULL) _functions = new std::list<GrfCommand*>;
		_functions->push_back(pFunction);
		return pFunction;
	}

	GrfFunction* GrfBlock::getFunction(const std::string& sFunctionName, const std::string& sTemplate, bool bGenericKey) const {
		if (_functions != NULL) {
			for (std::list<GrfCommand*>::const_iterator i = _functions->begin(); i != _functions->end(); ++i) {
				GrfFunction* pFunction = (GrfFunction*) (*i);
				if (pFunction->getName() == sFunctionName) {
					if (pFunction->isGenericKey()) {
						if (bGenericKey) {
							return pFunction;
						}
					} else if (pFunction->getTemplateInstantiation() == sTemplate) {
						return pFunction;
					}
				}
			}
		}
		if (getParent() != NULL) {
			return getParent()->getFunction(sFunctionName, sTemplate, bGenericKey);
		}
		return NULL;
	}

	bool GrfBlock::removeFunction(const std::string& sFunctionName, const std::string& sTemplateInstantiation) {
		if (_functions != NULL) {
			for (std::list<GrfCommand*>::iterator i = _functions->begin(); i != _functions->end(); i++) {
				GrfFunction* pFunction = (GrfFunction*) (*i);
				if ((pFunction != NULL) && (pFunction->getName() == sFunctionName) && (pFunction->getTemplateInstantiation() == sTemplateInstantiation)) {
					delete *i;
					_functions->erase(i);
					return true;
				}
			}
		}
		return false;
	}

	bool GrfBlock::removeGenericTemplateFunction(const std::string& sFunctionName) {
		if (_functions != NULL) {
			for (std::list<GrfCommand*>::iterator i = _functions->begin(); i != _functions->end(); ++i) {
				GrfFunction* pFunction = (GrfFunction*) (*i);
				if ((pFunction != NULL) && (pFunction->getName() == sFunctionName) && pFunction->isGenericKey()) {
					delete *i;
					_functions->erase(i);
					return true;
				}
			}
		}
		return false;
	}

	void GrfBlock::moveFunctions(GrfBlock& destinationBlock) {
		if ((_functions != NULL) && !_functions->empty()) {
			if (destinationBlock._functions == NULL) destinationBlock._functions = new std::list<GrfCommand*>;
			do {
				GrfFunction* pFunction = (GrfFunction*) _functions->front();
				destinationBlock.removeFunction(pFunction->getName(), pFunction->getTemplateInstantiation());
				pFunction->setParent(&destinationBlock);
				destinationBlock._functions->push_back(pFunction);
				_functions->pop_front();
			} while (!_functions->empty());
			delete _functions;
			_functions = NULL;
		}
	}


	bool GrfBlock::addLocalVariable(const std::string& sVarName, EXPRESSION_TYPE varType) {
		if (_variables == NULL) {
			_variables = new std::map<std::string, EXPRESSION_TYPE>;
		} else {
			std::map<std::string, EXPRESSION_TYPE>::const_iterator cursor = _variables->find(sVarName);
			if (cursor != _variables->end()) return false;
		}
		(*_variables)[sVarName] = varType;
		return true;
	}

	bool GrfBlock::addBNFLocalVariable(const std::string& sVarName, EXPRESSION_TYPE varType) {
		if (getParent() != NULL) return getParent()->addBNFLocalVariable(sVarName, varType);
		return false;
	}

	bool GrfBlock::removeVariable(const std::string& sVarName) {
		if (_variables == NULL) return false;
		std::map<std::string, EXPRESSION_TYPE>::iterator cursor = _variables->find(sVarName);
		if (cursor == _variables->end()) return false;
		_variables->erase(cursor);
		return true;
	}

	EXPRESSION_TYPE GrfBlock::getLocalVariable(const std::string& sVarName) const {
		if (_variables != NULL) {
			std::map<std::string, EXPRESSION_TYPE>::iterator cursor = _variables->find(sVarName);
			if (cursor != _variables->end()) return cursor->second;
		}
		return UNKNOWN_EXPRTYPE;
	}

	EXPRESSION_TYPE GrfBlock::getVariable(const std::string& sVarName) const {
		EXPRESSION_TYPE result = getLocalVariable(sVarName);
		if (result == UNKNOWN_EXPRTYPE) {
			if (getParent() != NULL) return getParent()->getVariable(sVarName);
			if ((sVarName == "this") || (sVarName == "project") || (sVarName == "null") || (DtaProject::getInstance().getGlobalVariableType(sVarName) != UNKNOWN_EXPRTYPE)) return NODE_EXPRTYPE;
		}
		return result;
	}

	void GrfBlock::moveVariables(GrfBlock& destinationBlock) {
		if ((_variables != NULL) && !_variables->empty()) {
			if (destinationBlock._variables == NULL) destinationBlock._variables = new std::map<std::string, EXPRESSION_TYPE>;
			for (std::map<std::string, EXPRESSION_TYPE>::const_iterator i = _variables->begin(); i != _variables->end(); ++i) {
				(*destinationBlock._variables)[i->first] = i->second;
			}
			delete _variables;
			_variables = NULL;
		}
	}


	DtaClass* GrfBlock::addClass(const std::string& sClassName) {
		DtaClass* pClass = getClass(sClassName);
		if (pClass != NULL) return NULL;
		pClass = new DtaClass(this, sClassName);
		if (_classes == NULL) _classes = new std::list<DtaClass*>;
		_classes->push_back(pClass);
		return pClass;
	}

	DtaClass* GrfBlock::getClass(const std::string& sClassName) const {
		if (_classes != NULL) {
			for (std::list<DtaClass*>::const_iterator i = _classes->begin(); i != _classes->end(); ++i) {
				DtaClass* pClass = *i;
				if (pClass->getName() == sClassName) {
					return pClass;
				}
			}
		}
		if (getParent() != NULL) return getParent()->getClass(sClassName);
		return NULL;
	}

	bool GrfBlock::removeClass(const std::string& sClassName) {
		if (_classes != NULL) {
			for (std::list<DtaClass*>::iterator i = _classes->begin(); i != _classes->end(); i++) {
				DtaClass* pClass = *i;
				if ((pClass != NULL) && (pClass->getName() == sClassName)) {
					delete *i;
					_classes->erase(i);
					return true;
				}
			}
		}
		return false;
	}

	void GrfBlock::moveClasses(GrfBlock& destinationBlock) {
		if ((_classes != NULL) && !_classes->empty()) {
			if (destinationBlock._classes == NULL) destinationBlock._classes = new std::list<DtaClass*>;
			do {
				DtaClass* pClass = _classes->front();
				destinationBlock.removeClass(pClass->getName());
				pClass->setBlock(&destinationBlock);
				destinationBlock._classes->push_back(pClass);
				_classes->pop_front();
			} while (!_classes->empty());
			delete _classes;
			_classes = NULL;
		}
	}

	DtaTargetLanguageTypeSpecifier* GrfBlock::addTargetLanguageTypeSpecifier(const std::string& sTypeSpec) {
		DtaTargetLanguageTypeSpecifier* pClass = getTargetLanguageTypeSpecifier(sTypeSpec);
		if (pClass != NULL) return NULL;
		pClass = new DtaTargetLanguageTypeSpecifier(this, sTypeSpec);
		if (_classes == NULL) _classes = new std::list<DtaClass*>;
		_classes->push_back(pClass);
		return pClass;
	}

	DtaTargetLanguageTypeSpecifier* GrfBlock::getTargetLanguageTypeSpecifier(const std::string& sTypeSpec) {
		if (_classes != NULL) {
			for (std::list<DtaClass*>::const_iterator i = _classes->begin(); i != _classes->end(); ++i) {
				DtaClass* pClass = *i;
				if (pClass->getName() == sTypeSpec) {
					DtaTargetLanguageTypeSpecifier* pTypeSpec = dynamic_cast<DtaTargetLanguageTypeSpecifier*>(pClass);
					return pTypeSpec;
				}
			}
		}
		if (getParent() != NULL) return getParent()->getTargetLanguageTypeSpecifier(sTypeSpec);
		return NULL;
	}


	SEQUENCE_INTERRUPTION_LIST GrfBlock::executeInternal(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result = NO_INTERRUPTION;
		if (hasLocalVariables()) {
			DtaScriptVariable localVariables(&visibility, "##stack## block");
			for (std::vector<GrfCommand*>::iterator i = _commands.begin(); i != _commands.end(); i++)
				if ((result = (*i)->execute(localVariables)) != NO_INTERRUPTION) break;
		} else {
			for (std::vector<GrfCommand*>::iterator i = _commands.begin(); i != _commands.end(); i++)
				if ((result = (*i)->execute(visibility)) != NO_INTERRUPTION) break;
		}
		return result;
	}

	SEQUENCE_INTERRUPTION_LIST GrfBlock::executeAt(DtaScriptVariable& visibility, int iPosition) {
		SEQUENCE_INTERRUPTION_LIST result = NO_INTERRUPTION;
		if (hasLocalVariables()) {
			DtaScriptVariable localVariables(&visibility, "##stack## block");
			for (std::vector<GrfCommand*>::iterator i = _commands.begin() + iPosition; i != _commands.end(); i++)
				if ((result = (*i)->execute(localVariables)) != NO_INTERRUPTION) break;
		} else {
			for (std::vector<GrfCommand*>::iterator i = _commands.begin() + iPosition; i != _commands.end(); i++)
				if ((result = (*i)->execute(visibility)) != NO_INTERRUPTION) break;
		}
		return result;
	}

	SEQUENCE_INTERRUPTION_LIST GrfBlock::executeInto(DtaScriptVariable& visibility, int iStart, int iEnd) {
		SEQUENCE_INTERRUPTION_LIST result = NO_INTERRUPTION;
		for (int i = iStart; i <= iEnd; i++) {
			result = _commands[i]->execute(visibility);
			if (result != NO_INTERRUPTION) break;
		}
		return result;
	}

	void GrfBlock::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		if ((_functions != NULL) && !_functions->empty() && (dynamic_cast<const GrfScriptBlock*>(this) == 0)) {
			std::string sMessage = "\"";
			if (this->_sParsingFilePtr != 0) {
				sMessage += this->_sParsingFilePtr;
			} else {
				sMessage += theCompilerEnvironment.getCurrentScriptSession()._sFilename;
			}
			sMessage += "\": C++ generation doesn't admit a function definition into a block of instructions;\nplease move the following one to the primary scope of the script:\n";
			for (std::list<GrfCommand*>::const_iterator i = _functions->begin(); i != _functions->end(); i++) {
				sMessage += "\t- " + dynamic_cast<GrfFunction*>(*i)->getSignature() + "\n";
			}
			throw UtlException(sMessage);
		}
		bool bBrackets = theCompilerEnvironment.bracketsToNextBlock();
		bool bCarriage = theCompilerEnvironment.carriageReturnAfterBlock();
		if (!bCarriage) theCompilerEnvironment.carriageReturnAfterBlock(true);
		if (bBrackets) {
			theCompilerEnvironment.incrementIndentation();
			CW_BODY_STREAM << "{";
			if (!_commands.empty()) CW_BODY_ENDL;
		} else {
			theCompilerEnvironment.bracketsToNextBlock(true);
		}
		std::vector<GrfCommand*>::const_iterator i;
		for (i = _commands.begin(); i != _commands.end(); i++) {
			(*i)->compileCpp(theCompilerEnvironment);
		}
		if (bBrackets) {
			theCompilerEnvironment.decrementIndentation();
			CW_BODY_INDENT << "}";
			if (bCarriage) CW_BODY_ENDL;
		}
	}

	void GrfBlock::compileCppFunctions(CppCompilerEnvironment& theCompilerEnvironment) const {
		if (_functions != NULL) {
			for (std::list<GrfCommand*>::const_iterator i = _functions->begin(); i != _functions->end(); i++) {
				theCompilerEnvironment.newFunction(dynamic_cast<GrfFunction*>(*i));
			}
			for (std::list<GrfCommand*>::const_iterator j = _functions->begin(); j != _functions->end(); j++) {
				(*j)->compileCpp(theCompilerEnvironment);
			}
		}
	}

	void GrfBlock::compileCppBNFSequence(CppCompilerEnvironment& theCompilerEnvironment) const {
		std::vector<GrfCommand*>::const_iterator i = getCommands().begin();
		for (;;) {
			(*i)->compileCpp(theCompilerEnvironment);
			i++;
			if (i == getCommands().end()) break;
			CW_BODY_INDENT << "if (_compilerClauseSuccess) {";CW_BODY_ENDL;
			theCompilerEnvironment.incrementIndentation();
		}
		int j = getNbCommands() - 1;
		while (j > 0) {
			theCompilerEnvironment.decrementIndentation();
			CW_BODY_INDENT << "}";CW_BODY_ENDL;
			j--;
		}
	}

	void GrfBlock::compileCppBetween(CppCompilerEnvironment& theCompilerEnvironment, int iStart, int iEnd) const {
		std::vector<GrfCommand*>::const_iterator i;
		std::vector<GrfCommand*>::const_iterator endIterator = _commands.begin() + iEnd;
		for (i = _commands.begin() + iStart; i != endIterator; i++) {
			(*i)->compileCpp(theCompilerEnvironment);
		}
	}

	void GrfBlock::compileCppAt(CppCompilerEnvironment& theCompilerEnvironment, int iStart) const {
		std::vector<GrfCommand*>::const_iterator i;
		for (i = _commands.begin() + iStart; i != _commands.end(); i++) {
			(*i)->compileCpp(theCompilerEnvironment);
		}
	}
}
