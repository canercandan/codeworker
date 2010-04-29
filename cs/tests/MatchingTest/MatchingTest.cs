namespace tests 
{
	class AreasIterator 
	{
		private CodeWorker.ParseTree[] allPositions_;
		private int iCurrentPosition_ = 0;
		private CodeWorker.ParseTree currentPosition_ = null;
		private int iCurrentRuleBegin_ = 0;
		private CodeWorker.ParseTree[] allRulesBegin_ = null;
	
		public AreasIterator(CodeWorker.ParseTree[] allPositions) { allPositions_ = allPositions; }

		public CodeWorker.ParseTree getCurrentPosition() 
		{
			if (currentPosition_ == null) currentPosition_ = allPositions_[iCurrentPosition_];
			return currentPosition_;
		}
	
		public CodeWorker.ParseTree getCurrentRulesBegin() 
		{
			if (allRulesBegin_ == null) 
			{
				CodeWorker.ParseTree currentPosition = getCurrentPosition();
				CodeWorker.ParseTree beginningRules = currentPosition.getNode("begin");
				if (beginningRules == null) return null;
				allRulesBegin_ = beginningRules.array;
			}
			if (iCurrentRuleBegin_ >= allRulesBegin_.GetLength(0)) return null;
			return allRulesBegin_[iCurrentRuleBegin_];
		}

		public bool nextCurrentRulesBegin() 
		{
			++iCurrentRuleBegin_;
			if (iCurrentRuleBegin_ >= allRulesBegin_.GetLength(0)) return nextPosition();
			return true;
		}

		public bool matchCurrentRulesEnd(string sEndOffset) 
		{
			return getCurrentPosition().name == sEndOffset;
		}
		
		public bool nextPosition() 
		{
			++iCurrentPosition_;
			currentPosition_ = null;
			iCurrentRuleBegin_ = 0;
			allRulesBegin_ = null;
			return iCurrentPosition_ < allPositions_.GetLength(0);
		}
	}


	public class MatchingTest 
	{
		public static void Main() 
		{
			// load the assembly of CodeWorker
			CodeWorker.Main.initialize();

			try 
			{
				// compile a BNF script file and execute the parsing
				CodeWorker.ParseTree theContext = new CodeWorker.ParseTree();
				CodeWorker.CompiledBNFScript theBNFScript = new CodeWorker.CompiledBNFScript();
				theBNFScript.buildFromFile("../../data/MatchingTest/CSVParser.cwp");
				theBNFScript.parse(theContext, "../../data/MatchingTest/example.csv");
				
				// display all rules of the grammar
				CodeWorker.ParseTree rulesNode = theContext.getNode("rules");
				CodeWorker.ParseTree[] allRules = rulesNode.array;
				System.Console.WriteLine("all rules:");
				for (int i = 0; i < allRules.GetLength(0); ++i) 
				{
					System.Console.WriteLine("\t" + allRules[i].text);
				}
				
				// display the coverage of the text by the BNF grammar
				CodeWorker.ParseTree areasNode = theContext.getNode("areas");
				CodeWorker.ParseTree[] allPositions = areasNode.array;
				System.Console.WriteLine("");
				System.Console.WriteLine("coverage of text by BNF rules:");
				displayCoverage(allPositions);

				// choose default colors for the 8 first rules
				string[] backgroundColors = {"#BBBBBB", "#808080", null, "red", null, null, "orange", null};
				string[] foregroundColors = {null, null, null, "yellow", "magenta", "pink", "green", "blue"};
				System.Console.WriteLine("");
				System.Console.WriteLine("highlighted rules (8 first rules of the grammar):");
				for (int i = 0; i < allRules.GetLength(0); ++i) 
				{
					string backgroundColor = backgroundColors[i];
					if (i < backgroundColors.GetLength(0)) 
					{
						backgroundColor = backgroundColors[i];
						if (backgroundColor != null) allRules[i].insertNode("color").insertNode("background").text = backgroundColor;
					}
					string foregroundColor = foregroundColors[i];
					if (i < foregroundColors.GetLength(0)) 
					{
						foregroundColor = foregroundColors[i];
						if (foregroundColor != null) allRules[i].insertNode("color").insertNode("foreground").text = foregroundColor;
					}
					if ((foregroundColor != null) || (backgroundColor != null)) 
					{
						System.Console.WriteLine("\t" + allRules[i].text + " [" + backgroundColor + ", " + foregroundColor + "]");
					} 
					else 
					{
						System.Console.WriteLine("\t" + allRules[i].text + " ... ignored");
					}
				}
				
				// generate the syntax highlighting in HTML, according to the BNF.
				// Note: "BNFcoverage.cwt" works for ANY grammar!
				CodeWorker.CompiledTranslationScript theCSV2HTMLScript = new CodeWorker.CompiledTranslationScript();
				theCSV2HTMLScript.buildFromFile("../../data/MatchingTest/BNFcoverage.cwp");
				theCSV2HTMLScript.translate(theContext, "../../data/MatchingTest/example.csv", "../../data/MatchingTest/example.csv.html");
				
				// to finish, display of the whole parse tree, up to depth = 4
				System.Console.WriteLine("");
				System.Console.WriteLine("display of the whole parse tree, up to depth = 4:");
				CodeWorker.Runtime.traceObject(theContext, 4);
			}
			catch(System.Exception exception) 
			{
				System.Console.WriteLine(exception);
			}
			CodeWorker.Main.terminate();
		}
		
		private static void displayCoverage(CodeWorker.ParseTree[] allPositions) 
		{
			if (allPositions.GetLength(0) == 0) return;
			AreasIterator it = new AreasIterator(allPositions);
			displayCoverage(it, "\t");
		}

		private static bool displayCoverage(AreasIterator it, string sIndent) 
		{
			CodeWorker.ParseTree thisRuleSet = it.getCurrentRulesBegin();
			if (thisRuleSet == null) return false;
			CodeWorker.ParseTree firstRuleName = thisRuleSet.array[0].reference;
			string sBeginningOffset = it.getCurrentPosition().name;
			string sEndOffset = thisRuleSet.name;
			System.Console.WriteLine(sIndent + firstRuleName.text + " [" + sBeginningOffset + ", " + sEndOffset + "]");
			it.nextCurrentRulesBegin();
			while (!it.matchCurrentRulesEnd(sEndOffset)) 
			{
				if (!displayCoverage(it, sIndent + '\t')) 
				{
					if (it.matchCurrentRulesEnd(sEndOffset)) break;
					it.nextPosition();
				}
			}
			return true;
		}
	}
}
