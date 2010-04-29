package org.codeworker.jni.tests;

class AreasIterator {
	private org.codeworker.IParseTree[] allPositions_;
	private int iCurrentPosition_ = 0;
	private org.codeworker.IParseTree currentPosition_ = null;
	private int iCurrentRuleBegin_ = 0;
	private org.codeworker.IParseTree[] allRulesBegin_ = null;
	
	public AreasIterator(org.codeworker.IParseTree[] allPositions) { allPositions_ = allPositions; }

	public org.codeworker.IParseTree getCurrentPosition() {
		if (currentPosition_ == null) currentPosition_ = allPositions_[iCurrentPosition_];
		return currentPosition_;
	}
	
	public org.codeworker.IParseTree getCurrentRulesBegin() {
		if (allRulesBegin_ == null) {
			org.codeworker.IParseTree currentPosition = getCurrentPosition();
			org.codeworker.IParseTree beginningRules = currentPosition.getNode("begin");
			if (beginningRules == null) return null;
			allRulesBegin_ = beginningRules.getArray();
		}
		if (iCurrentRuleBegin_ >= allRulesBegin_.length) return null;
		return allRulesBegin_[iCurrentRuleBegin_];
	}

	public boolean nextCurrentRulesBegin() {
		++iCurrentRuleBegin_;
		if (iCurrentRuleBegin_ >= allRulesBegin_.length) return nextPosition();
		return true;
	}

	public boolean matchCurrentRulesEnd(String sEndOffset) {
		return getCurrentPosition().getName().equals(sEndOffset);
	}
		
	public boolean nextPosition() {
		++iCurrentPosition_;
		currentPosition_ = null;
		iCurrentRuleBegin_ = 0;
		allRulesBegin_ = null;
		return iCurrentPosition_ < allPositions_.length;
	}
}


public class MatchingTest {
	public static void main(String args[]) {
		// load the JNI wrapper onto CodeWorker
		try {
			System.loadLibrary("JNICodeWorker");
		} catch(Exception exception) {
			System.out.println("Unable to load the library: '" + exception.toString() + "'");
		}

		// compile a BNF script file and execute the parsing
		org.codeworker.jni.ParseTree theContext = new org.codeworker.jni.ParseTree();
		org.codeworker.jni.CompiledBNFScript theBNFScript = new org.codeworker.jni.CompiledBNFScript();
		theBNFScript.buildFromFile("data/MatchingTest/CSVParser.cwp");
		theBNFScript.parse(theContext, "data/MatchingTest/example.csv");
		
		// display all rules of the grammar
		org.codeworker.IParseTree rulesNode = theContext.getNode("rules");
		org.codeworker.IParseTree[] allRules = rulesNode.getArray();
		System.out.println("all rules:");
		for (int i = 0; i < allRules.length; ++i) {
			System.out.println("\t" + allRules[i].getValue());
		}
		
		// display the coverage of the text by the BNF grammar
		org.codeworker.IParseTree areasNode = theContext.getNode("areas");
		org.codeworker.IParseTree[] allPositions = areasNode.getArray();
		System.out.println("");
		System.out.println("coverage of text by BNF rules:");
		displayCoverage(allPositions);

		// choose default colors for the 8 first rules
		String backgroundColors[] = {"#BBBBBB", "#808080", null, "red", null, null, "orange", null};
		String foregroundColors[] = {null, null, null, "yellow", "magenta", "pink", "green", "blue"};
		System.out.println("");
		System.out.println("highlighted rules (8 first rules of the grammar):");
		for (int i = 0; i < allRules.length; ++i) {
			String backgroundColor = backgroundColors[i];
			if (i < backgroundColors.length) {
				backgroundColor = backgroundColors[i];
				if (backgroundColor != null) allRules[i].insertNode("color").insertNode("background").setValue(backgroundColor);
			}
			String foregroundColor = foregroundColors[i];
			if (i < foregroundColors.length) {
				foregroundColor = foregroundColors[i];
				if (foregroundColor != null) allRules[i].insertNode("color").insertNode("foreground").setValue(foregroundColor);
			}
			if ((foregroundColor != null) || (backgroundColor != null)) {
				System.out.println("\t" + allRules[i].getValue() + " [" + backgroundColor + ", " + foregroundColor + "]");
			} else {
				System.out.println("\t" + allRules[i].getValue() + " ... ignored");
			}
		}
		
		// generate the syntax highlighting in HTML, according to the BNF.
		// Note: "BNFcoverage.cwt" works for ANY grammar!
		org.codeworker.jni.CompiledTranslationScript theCSV2HTMLScript = new org.codeworker.jni.CompiledTranslationScript();
		theCSV2HTMLScript.buildFromFile("data/MatchingTest/BNFcoverage.cwp");
		theCSV2HTMLScript.translate(theContext, "data/MatchingTest/example.csv", "data/MatchingTest/example.csv.html");
		
		// to finish, display of the whole parse tree, up to depth = 10
		System.out.println("");
		System.out.println("display of the whole parse tree, up to depth = 4:");
		org.codeworker.jni.Runtime.traceObject(theContext, 4);
	}
	
	private static void displayCoverage(org.codeworker.IParseTree[] allPositions) {
		if (allPositions.length == 0) return;
		AreasIterator it = new AreasIterator(allPositions);
		displayCoverage(it, "\t");
	}

	private static boolean displayCoverage(AreasIterator it, String sIndent) {
		org.codeworker.IParseTree thisRuleSet = it.getCurrentRulesBegin();
		if (thisRuleSet == null) return false;
		org.codeworker.IParseTree firstRuleName = thisRuleSet.getArray()[0].getReference();
		String sBeginningOffset = it.getCurrentPosition().getName();
		String sEndOffset = thisRuleSet.getName();
		System.out.println(sIndent + firstRuleName.getValue() + " [" + sBeginningOffset + ", " + sEndOffset + "]");
		it.nextCurrentRulesBegin();
		while (!it.matchCurrentRulesEnd(sEndOffset)) {
			if (!displayCoverage(it, sIndent + '\t')) {
				if (it.matchCurrentRulesEnd(sEndOffset)) break;
				it.nextPosition();
			}
		}
		return true;
	}
}
