s
// The controlling sequence stops on the first statement of the leader script.
// We go the next instruction:
n
// twice more:
n2
//let plunge into the BNF-driven script:
s
//We are pointing to the beginning of the rule. Let execute '#ignore(C++)':
s
//Let go to the unbounded expression '[class_declaration]*':
s
//Now, we have a look to 'class_declaration':
s
//We visit 'INDENT:"class"' and we step over immediatly. Into a BNF-driven script, tokens of a
//sequence are iterated step by step, and 'next' runs all the sequence in one shot:
s
n
//We visit 'INDENT:sClassName' and we step over immediatly:
s
n
//What about all local variables available on the stack?
l
//What is the value of 'sClassName'?
t sClassName
//Now, we are looking at a classical statement of the language, an 'insert' assignment. But
//it might be more convenient to see more source code:
d 4
//What about the call stack?
stack
//Exiting the debug session:
q
