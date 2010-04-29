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

#ifndef _DtaScriptVariable_h_
#define _DtaScriptVariable_h_

#ifndef WIN32
#	include <features.h>
#	if defined(__cplusplus) && __GNUC_PREREQ (4, 3)
#		include <cstring>
#		include <cstdlib>
#	endif
#endif

#include <string>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <memory>

namespace CodeWorker {
	class DtaArrayIterator;
	class ExprScriptVariable;
	class ExprScriptExpression;
	class DtaProject;
	class DtaScriptReferenceVariable;
	class GrfBlock;
	class CppCompilerEnvironment;
	class ExternalValueNode;

	class DtaScriptVariable;

	class DtaNodeListeners;

	class DtaNodeListener {
	public:
		DtaScriptVariable* _pNode;
		friend class DtaNodeListeners;

	public:
		inline DtaNodeListener() : _pNode(NULL) {}
		virtual ~DtaNodeListener();

		inline DtaScriptVariable* getNode() const { return _pNode; }

		virtual void onSetValue(const char* tcValue) = 0;
		virtual void onConcatValue(const char* tcValue) = 0;
		virtual void onSetReference(DtaScriptVariable* pReference) = 0;
		virtual void onSetExternal(ExternalValueNode* pExternalValue) = 0;
		virtual void onSetIterator(DtaArrayIterator** pIteratorData) = 0;
		virtual void onAddAttribute(DtaScriptVariable* pAttribute) = 0;
		virtual void onRemoveAttribute(DtaScriptVariable* pAttribute) = 0;
		virtual void onArrayHasChanged() = 0;
		virtual void onClearContent() = 0;
		virtual void onDelete() = 0;
	};

	class DtaScriptVariableList {
		private:
			DtaScriptVariable* _pNode;
			DtaScriptVariableList* _pNext;

		public:
			inline DtaScriptVariableList(DtaScriptVariable* pNode = NULL, DtaScriptVariableList* pNext = NULL) : _pNode(pNode), _pNext(pNext) {}
			inline ~DtaScriptVariableList() { delete _pNext; }

			inline DtaScriptVariable* getNode() const { return _pNode; }
			inline void setNode(DtaScriptVariable* pNode) { _pNode = pNode; }
			inline DtaScriptVariableList* getNext() const { return _pNext; }
			inline void setNext(DtaScriptVariableList* pNext) { _pNext = pNext; }

			void pushBack(DtaScriptVariable* pVariable);
	};

	class DtaScriptVariable {
	private:
		const char* _tcName;
		DtaScriptVariable* _pParent;
		DtaScriptVariableList* _pAttributes;
		struct NodeArrayType {
			std::list<DtaScriptVariable*> items;
			mutable std::map<std::string, DtaScriptVariable*>* cache;
			inline NodeArrayType() : cache(NULL) {}
			NodeArrayType(DtaScriptVariable* pElt);
			~NodeArrayType();
		};
		NodeArrayType* _pArray;
		std::set<DtaScriptVariable*>* _pReferencedByVariables;
		enum VALUE_NODE_TYPE {NO_VALUE_NODE = 0, STRING_VALUE_NODE = 1, REFERENCE_VALUE_NODE = 2, INDEX_VALUE_NODE = 6, EXTERNAL_VALUE_NODE = 8};
		VALUE_NODE_TYPE _eValueNode;
		union VALUE_NODE {
			DtaScriptVariable* var;
			struct STRING_VALUE_NODE {
				char* text;
				size_t length;
			};
			STRING_VALUE_NODE str;
			ExternalValueNode* ext;
			struct INDEX_VALUE_NODE {
				DtaArrayIterator** var;
				bool own;
			};
			INDEX_VALUE_NODE ind;
		};
		VALUE_NODE _value;
		DtaNodeListeners* _pListeners;

	public:
		inline DtaScriptVariable() : _pParent(NULL), _eValueNode(NO_VALUE_NODE), _tcName(NULL), _pArray(NULL), _pReferencedByVariables(NULL), _pAttributes(NULL) {
			_pListeners = NULL;
		}
		inline DtaScriptVariable(DtaScriptVariable* pParent, const char* tcName)
			: _pParent(pParent), _eValueNode(NO_VALUE_NODE), _pArray(NULL), _pReferencedByVariables(NULL), _pAttributes(NULL)
		{
			_pListeners = NULL;
#ifdef WIN32
			_tcName = ::_strdup(tcName);
#else
			_tcName = ::strdup(tcName);
#endif
		}
		inline DtaScriptVariable(DtaScriptVariable* pParent, const std::string& sName)
			: _pParent(pParent), _eValueNode(NO_VALUE_NODE), _pArray(NULL), _pReferencedByVariables(NULL), _pAttributes(NULL)
		{
			_pListeners = NULL;
#ifdef WIN32
				_tcName = ::_strdup(sName.c_str());
#else
				_tcName = ::strdup(sName.c_str());
#endif
		}
		~DtaScriptVariable();

		inline DtaScriptVariable* getParent() const { return _pParent; }
		DtaScriptVariable* getRoot() const;
		DtaScriptVariable* getReferencedVariable() const;
		inline ExternalValueNode* getExternalValueNode() const { return ((_eValueNode == EXTERNAL_VALUE_NODE) ? _value.ext : NULL); }
		std::string getCompleteName() const;
		inline const char* getName() const { return _tcName; }
		void changeName(const char* tcName);
		bool isLocal() const;
		inline DtaScriptVariableList* getAttributes() const { return _pAttributes; }
		DtaScriptVariable* detachAttribute(const char* tcName);

		void addListener(DtaNodeListener* pListener); // belongs to the node!
		bool removeListener(DtaNodeListener* pListener);

		const std::list<DtaScriptVariable*>* getArray() const;
		int getArraySize() const;
		const std::map<std::string, DtaScriptVariable*>* getSortedArray() const;
		std::auto_ptr<std::vector<DtaScriptVariable*> > getSortedNoCaseArray() const;
		std::auto_ptr<std::vector<DtaScriptVariable*> > getSortedArrayOnValue() const;
		std::auto_ptr<std::vector<DtaScriptVariable*> > getSortedNoCaseArrayOnValue() const;
		DtaScriptVariable* getArrayElement(const std::string& sPosition) const;
		DtaScriptVariable* getArrayElement(int iPosition) const;
		void invertArray();
		DtaScriptVariable* addElement(const std::string& sName);
		DtaScriptVariable* addElement(int iKey);
		DtaScriptVariable* insertElementAt(const std::string& sProposedKey, int iPosition);
		void removeArrayElement();
		void removeArrayElement(const char* sPosition);
		void removeArrayElement(int iPosition);
		void removeFirstElement();
		void removeLastElement();
		void clearArray();
		void sortArray();

		int getNumberOfAttributes() const;
		DtaScriptVariable* getVariable(const ExprScriptVariable& exprVariable);
		DtaScriptVariable* getNonRecursiveVariable(DtaScriptVariable& visibility, const ExprScriptVariable& exprVariable);
		DtaScriptVariable* getNonRecursiveNonReferencedVariable(DtaScriptVariable& visibility, const ExprScriptVariable& exprVariable);
		DtaScriptVariable* getVariableForReferenceAssignment(const ExprScriptVariable& exprVariable);
		DtaScriptVariable* getExistingVariable(const ExprScriptVariable& exprVariable);
		DtaScriptVariable* getNoWarningExistingVariable(const ExprScriptVariable& exprVariable);

		DtaScriptVariable* getNonRecursiveVariable(const char* sName);
		DtaScriptVariable* getVariable(const char* sName, bool bRecursiveSearch = true, bool bCreateIfUnknown = true, bool bReferencedVariable = true);
		DtaScriptVariable* getNode(const char* sName);
		DtaScriptVariable* insertNode(const char* sName);
		DtaScriptVariable* getEvaluatedNode(const std::string& sDynamicVariable);
		DtaScriptVariable* getOrCreateLocalEvaluatedNode(const std::string& sDynamicVariable);
		DtaScriptVariable* insertEvaluatedNode(const std::string& sDynamicVariable);
		DtaScriptVariable* insertClassicalEvaluatedNode(const std::string& sDynamicVariable);
		void moveToNode(DtaScriptVariable& node);
		bool equal(const DtaScriptVariable& variable);

		DtaScriptVariable* createNodeArgument(const char* sName, DtaScriptVariable* pParameter);
		DtaScriptVariable* getNodeArgument(const char* sName) const;
		DtaScriptVariable* createIterator(const char* sName, DtaArrayIterator** pIteratorData);
		DtaScriptVariable* getIterator(const char* sName) const;
		DtaArrayIterator* getIteratorData() const;

		void setValueAtVariable(const char* sVariable, const char* sValue, bool bRecursiveSearch = false, bool bCreateIfUnknown = true);

		const char* getValue() const;
		size_t getValueLength() const;
		double getDoubleValue() const;
		bool   getBooleanValue() const;
		int    getIntValue() const;
		void clearValue();
		void setValue(const char* tcValue);
		void setValue(bool bValue);
		void setValue(double dValue);
		void setValue(int iValue);
		void setValue(const std::list<std::string>& listOfValues);
		void setValue(DtaScriptVariable* pVariable);
		void setValue(ExternalValueNode* pExternalValue);
		void setValue(DtaArrayIterator* pIteratorData);
		void setIteratorData(DtaArrayIterator** pIteratorData);
		void concatValue(const char* tcValue);

		void remove(DtaScriptVariable* pVariable);
		void remove(const char* sAttribute);
		void removeRecursive(const char* sAttribute);
		void clearContent();
		void addArrayElements(const DtaScriptVariable& variable);
		void copyAll(const DtaScriptVariable& variable, bool bMerge = false);
		DtaScriptVariable* pushItem(const std::string& sValue);
		void slideNodeContent(DtaScriptVariable& visibility, const ExprScriptVariable& varExpr);

		bool traceValue(int iDepth = 0, const std::string& sIndent = "", const std::string& sPrefix = "");
		void traceObject(int iDepth = 0, const std::string& sIndent = "");
		void traceStack();

	private:
		DtaScriptVariable* addInternalElement(const std::string& sName);
		DtaScriptVariable* getInternalVariableExtraction(DtaScriptVariable& visibility, const ExprScriptVariable& exprVariable, bool bRecursiveSearch, bool bCreateIfUnknown, bool bReferencedVariable);
		void removeReference();
		void addReferencedBy(DtaScriptVariable* pVariable);
	};
}

#endif
