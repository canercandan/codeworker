@echo ** Example 1. CodeWorker as an 'inline code expander'
@CodeWorker -translate inlineCodeExpander.cwp inlineCodeExpander.sqlc inlineCodeExpander.c
@echo .

@echo ** Example 2. CodeWorker as a 'mixed code generator'
@CodeWorker -expand mixedCodeGenerator.cwt mixedCodeGenerator.cpp
@echo .

@echo ** Example 3. CodeWorker to add intrusive features in a programming language
@CodeWorker -expand switch.cwt switch.cpp
@echo .

@echo ** Example 4. CodeWorker as a 'DSL builder'
@CodeWorker -script tinyDSL_leader.cws
@echo .

@echo ** Example 5. CodeWorker as a 'source-to-source translator'
@CodeWorker -translate tinyDSL2HTML.cwp tinyDSL_spec.txt tinyDSL_spec.html
@echo .

@echo ... and a lot of subjects concerning the generative programming!
