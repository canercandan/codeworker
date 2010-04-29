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

package org.codeworker.jni;

public class BNFRuntimeEnvironment implements org.codeworker.IBNFRuntimeEnvironment {
	@SuppressWarnings("unused")
	private byte[] cppInstance_;

	public BNFRuntimeEnvironment(org.codeworker.IEXECUTE_CLAUSE executeClause, int iIgnoreMode, boolean bImplicitCopy) {
		cppInstance_ = init(executeClause, iIgnoreMode, bImplicitCopy);
	}

	protected void finalize() { destroy(); }

	public native boolean implicitCopy();
	public native void pushIgnoreMode(org.codeworker.IBNFRuntimeIgnore ignoreMode, int iNewIgnoreMode, org.codeworker.IEXECUTE_CLAUSE newExecuteClause);
	public native void popIgnoreMode(org.codeworker.IBNFRuntimeIgnore ignoreMode);
	public native void pushImplicitCopy(org.codeworker.IBNFRuntimeTransformationMode transformationMode, boolean bNewImplicitCopy);
	public native void popImplicitCopy(org.codeworker.IBNFRuntimeTransformationMode transformationMode);
	public native void addClauseSignature(String sSignature);
	public native void activateMatchingAreas();
	public native void storeMatchingAreas(org.codeworker.IParseTree pStorage);

	public native int skipEmptyChars();
	public native void writeBinaryData(byte[] tcText);
	public native void writeBinaryData(byte cChar);
	
	private native byte[] init(org.codeworker.IEXECUTE_CLAUSE executeClause, int iIgnoreMode, boolean bImplicitCopy);
	private native void destroy();
};
