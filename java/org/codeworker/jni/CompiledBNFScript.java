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

public class CompiledBNFScript {
	@SuppressWarnings("unused")
	private byte[] cppInstance_;
	
	public CompiledBNFScript() { cppInstance_ = init(); }
	protected void finalize() { destroy(); }

	public native void buildFromString(String sText);
	public native void buildFromFile(String sScriptFile);

	public native void parse(ParseTree pContext, String sParsedFile);
	public native void parseString(ParseTree pContext, String sText);
	
	private native byte[] init();
	private native void destroy();
}
