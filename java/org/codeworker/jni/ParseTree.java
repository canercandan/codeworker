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

public class ParseTree implements org.codeworker.IParseTree {
	protected byte[] cppInstance_;
	private boolean owner_;

	public ParseTree() { owner_ = true;cppInstance_ = init(); }
	public ParseTree(byte[] cppInstance) { owner_ = false;cppInstance_ = cppInstance; }
	protected void finalize() { if (owner_) destroy(); }
	
	public native String getName();
	public native String getValue();
	public native String getStringValue();
	public native void setValue(String sValue);
	public native org.codeworker.IParseTree getReference();

	public native org.codeworker.IParseTree[] getArray();
	public native String[] getAttributeNames();

	public native org.codeworker.IParseTree getNode(String sAttribute);
	public native org.codeworker.IParseTree insertNode(String sAttribute);

	private native byte[] init();
	private native void destroy();

	protected ParseTree(boolean owner) { owner_ = owner;cppInstance_ = null; }
}
