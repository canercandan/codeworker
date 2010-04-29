/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2005 Cédric Lemaire

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

package org.codeworker;

public interface IBNFRuntimeEnvironment {
	boolean implicitCopy();
	void pushIgnoreMode(org.codeworker.IBNFRuntimeIgnore ignoreMode, int iNewIgnoreMode, org.codeworker.IEXECUTE_CLAUSE newExecuteClause);
	void popIgnoreMode(org.codeworker.IBNFRuntimeIgnore ignoreMode);
	void pushImplicitCopy(org.codeworker.IBNFRuntimeTransformationMode transformationMode, boolean bNewImplicitCopy);
	void popImplicitCopy(org.codeworker.IBNFRuntimeTransformationMode transformationMode);
	void addClauseSignature(String sSignature);
	void activateMatchingAreas();
	void storeMatchingAreas(org.codeworker.IParseTree pStorage);

	int skipEmptyChars();
	void writeBinaryData(byte[] tcText);
	void writeBinaryData(byte cChar);
};
