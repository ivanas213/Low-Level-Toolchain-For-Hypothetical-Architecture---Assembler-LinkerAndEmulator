#include "../inc/Assembler.hpp"
#include <string>

#include <iostream>
#include <FlexLexer.h>

using namespace std;
bool Assembler::commentLine=false;
bool Assembler::newLine=true;
bool Assembler::error=false;
bool Assembler::readingSymbols=false;
bool Assembler::readingNumber=false;
bool Assembler::readingSection=false;
bool Assembler::readingRegister=false;
bool Assembler::readingString=false;
bool Assembler::end=false;
bool Assembler::readingCSR=false;
bool Assembler::readingOperand=false;
bool Assembler::readingLabel=false;
bool Assembler::emptyLine=false;
bool Assembler::noSection=true;
const char* Assembler::buffer=nullptr;
int Assembler::location_counter=0;
SectionTable* Assembler::sectionTable=new SectionTable();
SymbolTable* Assembler::symbolTable=new SymbolTable();
Assembler* Assembler::assembler=nullptr;
Assembler::Type Assembler::currentAssembling;
ifstream Assembler::input_stream;
string Assembler::output_file;
FlexLexer* Assembler::flexer;
RelocationTable Assembler::relocationTable=*(new RelocationTable());
int Assembler::sectionCounter=-1;

Assembler::CommandType Assembler::currentCommandType=Assembler::CommandType::zero_arg;
string Assembler::currentsection;
std::tr1::unordered_map<std::string, int> Assembler::sections;
string Assembler::input_file;
string Assembler::in;

// Converts a number (0-15) to its corresponding hexadecimal character ('0'-'9', 'A'-'F').
char Assembler::getHexChar(int number) {
  if (number >= 0 && number <= 9) return '0' + number;
  else return 'A' + number - 10;
}

// Converts a number (0-15) to its inverted hexadecimal character in the range ('F'-'A' or '0'-'5').
char Assembler::getHexCharInverted(int number) {
  if (number >= 0 && number <= 5) {
    return 'F' - number;
  }
  else return '0' + 15 - number;
}

// Converts a given integer to its hexadecimal representation.
// The parameter `sz` represents the number of hexadecimal digits to output.
// For positive numbers, it converts the number directly.
// For negative numbers, it converts using two's complement inversion.
string Assembler::getHex(int number, int sz) {
  string ret = "";
  
  if (number >= 0) {
    // Handling positive numbers
    for (int i = 0; i < sz; i++) {
      int num = number % 16; // Get the least significant 4 bits
      string s = "";
      string ch = "";
      ch.push_back(getHexChar(num)); // Convert to hexadecimal character
      s.insert(0, ch); // Insert character to string
      
      ch.erase(0, 1);
      number = number >> 4; // Shift number to process the next 4 bits
      num = number % 16;
      
      ch.push_back(getHexChar(num));
      s.insert(0, ch);
      ret.append(s);
      number = number >> 4; // Shift again
    }
  } else {
    // Handling negative numbers using two's complement inversion
    number = 0 - number;
    bool add = true;
    
    for (int i = 0; i < 4; i++) {
      int num = number % 16;
      string s = "";
      string ch = "";
      char hx = getHexCharInverted(num); // Invert the number
      
      if (add) {
        if (hx == 'F') hx = '0';
        else { hx++; add = false; }
      }
      
      ch.push_back(hx);
      s.insert(0, ch);
      ch.erase(0, 1);
      number = number >> 4; // Shift number to process the next 4 bits
      num = number % 16;
      number = number >> 4; // Shift again
      
      hx = getHexCharInverted(num); // Invert again
      
      if (add) {
        if (hx == 'F') hx = '0';
        else { hx++; add = false; }
      }
      
      ch.push_back(hx);
      s.insert(0, ch);
      ret.append(s);
    }
  }
  return ret;
}

// Constructor for Assembler class, initializes input and output streams, 
// sets up lexer and initializes symbol and section tables.
Assembler::Assembler(string input, string output) {
  input_file = input;
  output_file = output;
  in = input;
  input_stream.open(input, ios::in); // Open input file
  
  if (!input_stream) {
    cout << "Input file not found";
  }
  
  flexer = new yyFlexLexer(&input_stream); // Initialize lexer
  location_counter = 0; // Set initial location counter
  symbolTable = new SymbolTable(); // Initialize symbol table
  sectionTable = new SectionTable(); // Initialize section table
}

// Static method to initialize the assembler singleton instance
Assembler Assembler::init(string input, string output) {
  if (assembler == nullptr) {
    assembler = new Assembler(input, output);
  }
  return *assembler;
}


void Assembler::firstPass(){
    int pick=1; // represents the result of lexical analysis.
    char* section_name=nullptr; //olds the name of the current section.
	  
	// Loop until there is an error or the ".end" directive is reached.
    while(pick>0 && !error && !end){
		  
        pick=pickNextAndRow(); // Call the function to pick the next token and row number.
		
		// If pick equals 111, it means some error has occurred, so we terminate.
        if(pick==111){
          error=true;
          break;
        }
		
		// Skip over unwanted tokens (like whitespace or comments).
        while(pick==0 || pick>=1000){
          pick=flexer->yylex();  // Call the lexical analyzer for the next token.
        }
		
		// If the line is a comment, continue skipping until the comment ends.
        if(commentLine){
          while(commentLine){
            flexer->yylex();
          }
        }
        else 
			
		// Based on the current state, we process either a directive, command, or label.
			switch(currentAssembling){
			  
				// Processing assembler directives like `.section`, `.global`, `.word`, etc.
				case directive:{
				
					// If the directive is `.section`
					if(!strcmp(buffer,".section")){
						pick=pickNext();
						string b=buffer;
					
						// Check if the section name ends with a comma (error condition).
						if(b.back()==','){
						  error=true;
						  break;
						}
					
						// If we are reading symbols (label or section names).
						if(readingSymbols){
							
						  // If no section is defined, we set the size of the current section.
						  if(!noSection){
							sectionTable->setSize(currentsection,location_counter);
						  }
						  else{
							noSection=false;

						  }
						  // Set and add the current section.
						  Assembler::setCurrentSection((string)buffer);
						  sectionTable->addSection(currentsection);
						  
						  //Reset the location counter.
						  location_counter = 0;

						  // Insert the section symbol into the symbol table.
						  symbolTable->insertNewSymbol(buffer, location_counter, 0, SymbolTable::Type::SECTION, SymbolTable::Bind::GLOBAL, currentsection);
						  readingSymbols = false;
						}
						else break;
					}
				  
					// Handle `.extern` directive (external symbols).
					else if(!strcmp(buffer,".extern")){
						bool next=true;
						while(next){
						  pick=pickNext();
						  string b=buffer;
						  
						  // If we're not reading symbols, it's an error.
						  if(!readingSymbols) {
							error=true;
							break;
						  }
						  else{
							readingSymbols=false;
						  }

						  // Check if the symbol ends with a comma.
						  if (b.back() != ',') {
							next = false;
						  } else {
							b.pop_back();  // Remove the comma.
						  }
						  
						   // If the symbol is not declared yet, add it as an external symbol.
						   if (!symbolTable->isDeclared(b)) {
							 symbolTable->insertNewSymbol(b, -1, 0, SymbolTable::Type::OBJECT, SymbolTable::Bind::GLOBAL, "UND");
						   } else if (symbolTable->isDefined(b)) {
							  error = true;  // If the symbol is already defined, it's an error.
							  break;
							}
							symbolTable->setExtern(b);  // Mark the symbol as external.
						}
					}
					
					// Handle `.global` directive (global symbols).
					else if(!strcmp(buffer,".global")){
						bool next=true;
						string put;
						bool br=false;
						
						// Loop to process each global symbol.
						while(next){
							int pick=pickNext();   
							string b=buffer;
							// If the symbol ends with a comma, continue.
							if (b.back() != ',') {
								next = false;
							} else {
								b.pop_back();  // Remove the comma.
							}
							
							// If not reading symbols, it's an error.
							if (!readingSymbols) {
								error = true;
								br = true;
							} else {
								readingSymbols = false;
							}

							// Insert the symbol if not declared, or mark it as global.
							if (!symbolTable->isDeclared(b)) {
								symbolTable->insertNewSymbol(b, -1, 0, SymbolTable::OBJECT, SymbolTable::GLOBAL, "UND");
							} else if (!symbolTable->haveBind(b)) {
								symbolTable->setGlobal(b);
							}
							
						}
						
						if(br) break;
               
					}
				
					// Handle `.word` directive (define a word).
					else if(!strcmp(buffer,".word")){
						bool next=true;
						string put;
						while(next){
							pick=pickNext();
							string b=buffer;
							put=buffer;

						// If the word ends with a comma, continue.
						if (b.back() != ',') {
							next = false;
						} else {
							put.pop_back();  // Remove the comma.
						}
						
						// If we're not reading symbols or numbers, it's an error.
						if (!readingSymbols && !readingNumber) {
							error = true;
							break;
						} else if (readingSymbols) {
							readingSymbols = false;
							if (!symbolTable->isDeclared(put)) {
								symbolTable->insertNewSymbol(put, -1, 4, SymbolTable::OBJECT, SymbolTable::LOCAL, "UND");
							}
							} else if (readingNumber) {
								readingNumber = false;
								int num = stoi(put);  // Convert string to number.
								sectionTable->addLiteral(sectionTable->getSize() - 1, num, 4, -1);
							}
							location_counter += 4;  // Increase location counter by 4 bytes.
						}
					}
					// Handle `.skip` directive (skip some memory locations).
					else if (!strcmp(buffer, ".skip")) {
						pick = pickNext();
						if (!readingNumber) {
							error = true;
							break;
						} 
						else{
							string b = buffer;
							location_counter += stoi(b);  // Increase the location counter by the specified number.
						}
					}
					
					// Handle `.end` directive (end of the assembly).
					else if (!strcmp(buffer, ".end")) {
						end = true;
					}

					// Handle `.ascii` directive (ASCII string data).
					else if (!strcmp(buffer, ".ascii")) {
						pick = pickNext();
						if (!readingString) {
							error = true;
							break;
						}
						else{
							readingString = false;
							string b = buffer;
							location_counter += b.size() - 2;  // Adjust for the string size.
						}
					}

					break;
				}
				// Processing assembler commands.
				case command:{
					// Handling commands with zero arguments
					if(currentCommandType==zero_arg){
						if(!strcmp(buffer,"halt")) {location_counter+=4;} // 'halt' instruction occupies 4 bytes.
						if(!strcmp(buffer,"ret")) {location_counter+=4;} //'ret' instruction occupies 4 bytes.
						if(!strcmp(buffer,"iret")) {location_counter+=8;} // 'iret' instruction occupies 8 bytes.
						else if(!strcmp(buffer,"int")) location_counter+=4; // 'int' instruction occupies 4 bytes.
					}
					// Handling commands with zero arguments
					else if(currentCommandType==one_arg){
						if(!strcmp(buffer,"call") ||!strcmp(buffer,"jmp")){
							int pick=pickNext(); // Retrieve the next token to determine type.
							
							// Check if a valid symbol or number is read
							if(!readingSymbols && !readingNumber ){
								error=true;
								break;
							}
							else if(readingSymbols ){
								// Process the symbol on the first pass
								processSymbolFirstPass(buffer);
							}
							else {
								// Process the literal value on the first pass
								processLiteralFirstPass(buffer);
							}
						location_counter+=4;// Update location counter for 4-byte instruction.

						}
						else if(!strcmp(buffer, "push") || !strcmp(buffer, "pop") || !strcmp(buffer, "not")) {
							int pick = pickNext();  // Retrieve the next token to determine type

							// Check if a register is being read as expected for these commands
							if(!readingRegister) {
								error = true;  // Set error flag if a register is not being read
								break;  // Exit the case if there's an error
							} 
							else {
								string b = buffer;
							
								// Check if there's an incorrect comma at the end of the buffer
								if(b.back() == ',') {
									error = true;  // Set error flag if a comma is incorrectly placed
									break;
								} 
							
								// Reset register reading state for the next instruction
								readingRegister = false;
							}

							location_counter += 4;  // Update location counter to account for 4-byte instruction
						}

					}
					else if(currentCommandType == two_args) {
						// Handle `ld` command with two arguments
						if(!strcmp(buffer, "ld")) {
							pick = pickNext();
							string b = buffer;

							// Ensure the first argument ends with a comma
							if(b.back() != ',') {
								error = true;
								break;
							}

							// Check if an operand is being read (either symbol, number, or operand)
							if(!readingSymbols && !readingNumber && !readingOperand) {
								error = true;
								break;
							} 
							else {
								// Process based on type of operand
								if(readingNumber) {
									readingNumber = false;
									processLiteralFirstPass(buffer);  // Process literal
									location_counter += 4;  // Increment location counter
								}
								else if(readingOperand) {
									readingOperand = false;
									processOperandFirstPass(buffer);  // Process operand
								}
								else if(readingSymbols) {
									readingSymbols = false;
									processSymbolFirstPass(buffer);  // Process symbol
									location_counter += 4;  // Increment for symbol data
								}

								pick = pickNext();  // Move to the second argument

								// Validate if the next argument is a register
								if(!readingRegister) {
									error = true;
									break;
								}
								else {
									string b = buffer;
								
									// Check if comma is incorrectly placed at the end
									if(b.back() == ',') {
										error = true;
										break;
									}
								
									readingRegister = false;  // Reset reading state
								}
								location_counter += 4;  // Final increment for instruction size
							}
						}
						// Handle `st` command
						else if(!strcmp(buffer, "st")) {
							pick = pickNext();
						
							// Check if a register is being read as the first argument
							if(!readingRegister) {
								error = true;
								break;
							}
							else {
								readingRegister = false;
								pick = pickNext();  // Move to the second argument

								// Ensure we are reading either a symbol, number, or operand
								if(!readingSymbols && !readingNumber && !readingOperand) {
									error = true;
									break;
								}
							
								string b = buffer;

								// Check if comma is incorrectly placed at the end
								if(b.back() == ',') {
									error = true;
									break;
								}

								// Process operand type accordingly
								if(readingNumber) {
									readingNumber = false;
									processLiteralFirstPass(buffer);
								}
								else if(readingOperand) {
									readingOperand = false;
									processOperandFirstPass(buffer);
								}
								else if(readingSymbols) {
									readingSymbols = false;
									processSymbolFirstPass(buffer);
								}
							}
							location_counter += 4;  // Increment for instruction size
						}
						// Handle `csrrd` command
						else if(!strcmp(buffer, "csrrd")) {
							pick = pickNext();

							// Check if reading CSR is expected
							if(!readingCSR) {
								error = true;
								break;
							}
							
							readingCSR = false;
							string b = buffer;

							// Check if comma is missing between CSR and register
							if(b.back() != ',') {
								error = true;
								break;
							}

							pick = pickNext();  // Move to register argument

							// Ensure that the next argument is a register
							if(!readingRegister) {
								error = true;
								break;
							}
							
							readingRegister = false;
							b = buffer;

							// Ensure no trailing comma in buffer
							if(b.back() == ',') {
								error = true;
								break;
							}

							location_counter += 4;  // Increment for instruction
						}
						// Handle `csrwr` command
						else if(!strcmp(buffer, "csrwr")) {
							pick = pickNext();

							// Verify the first argument is a register
							if(!readingRegister) {
								error = true;
								break;
							}

							readingRegister = false;
							string b = buffer;

							// Check for comma separation
							if(b.back() != ',') {
								error = true;
								break;
							}

							pick = pickNext();  // Move to CSR argument

							// Ensure the next argument is a CSR
							if(!readingCSR) {
								error = true;
								break;
							}

							readingCSR = false;
							b = buffer;

							// Verify no trailing comma in buffer
							if(b.back() == ',') {
								error = true;
								break;
							}

							location_counter += 4;  // Increment for instruction size
						}
						// Handle generic two-argument instructions
						else {
							pick = pickNext();

							// Verify the first argument is a register
							if(!readingRegister) {
								error = true;
								break;
							}

							string b = buffer;

							// Ensure comma separation
							if(b.back() != ',') {
								error = true;
								break;
							}

							readingRegister = false;
							pick = pickNext();  // Move to second argument

							// Verify the second argument is a register
							if(!readingRegister) {
								error = true;
								break;
							}

							b = buffer;

							// Ensure no trailing comma after the last argument
							if(b.back() == ',') {
								error = true;
								break;
							}

							readingRegister = false;
							location_counter += 4;  // Increment location counter for instruction
						}
					}

					else if(currentCommandType == three_args) {
						// Loop to process the first two arguments (registers)
						for(int i = 0; i < 2; i++) {
							pick = pickNext();  // Move to the next argument

							// Ensure each argument is a register
							if(!readingRegister) {
								error = true;  // Set error if not a register
								break;
							}
							readingRegister = false;  // Reset reading state

							string b = buffer;

							// Validate that each of the first two arguments ends with a comma
							if(b.back() != ',') {
								error = true;  // Set error if no comma
								break;
							}
						}

						// Pick the third argument after processing two registers
						pick = pickNext();

						// Validate if the third argument is a valid symbol, number, or operand
						if(!readingSymbols && !readingNumber && !readingOperand) {
							error = true;  // Set error if it is none of these types
							break;
						}

						string b = buffer;

						// Check if there is an invalid trailing comma in the last argument
						if(b.back() == ',') {
							error = true;  // Set error if comma found
							break;
						}

						// Process the third argument based on its type
						if(readingNumber) {
							processLiteralFirstPass(buffer);  // Process if a literal/number
							readingNumber = false;  // Reset reading state
						}
						else if(readingOperand) {
							processOperandFirstPass(buffer);  // Process if an operand
							readingOperand = false;  // Reset reading state
						}
						else {
							processSymbolFirstPass(buffer);  // Process if a symbol
							readingSymbols = false;  // Reset reading state
						}

						// Increment the location counter to account for the instruction size
						location_counter += 4;
					}
					break;
				}
				case label: {
					// Ensure the label is being read in the correct context
					if(!readingLabel) {
						error = true;  // Set error if label is not expected
						break;
					}
					readingLabel = false;  // Reset reading state for labels

					// Convert buffer to string and remove trailing character (likely ':')
					string b = buffer;
					b.pop_back();

					// Check if the label already exists in the symbol table
					if(!symbolTable->isDeclared(b)) {
						// If the label is not declared, insert it into the symbol table
						// with the current location counter and assign it as an object with
						// local binding in the current section
						symbolTable->insertNewSymbol(b, location_counter, 0, SymbolTable::Type::OBJECT, SymbolTable::Bind::LOCAL, currentsection);
					} 
					else {
						// If the label is already declared, define it with the current
						// location counter and section to resolve any forward references
						symbolTable->define(b, location_counter, currentsection);
					}
					break;  // Exit the label case
				}


				}
				if (end) {
					// Set the size of the current section in the section table to the current location counter,
					// marking the end of this section's size tracking.
					sectionTable->setSize(currentsection, location_counter);
				}

				// If the end of the input has been reached or an error has occurred, exit the loop.
				if (end || error) break;

		}
		if (!end) {
			// Set an error flag if the end of the input has not been reached,
			// indicating unexpected continuation of parsing.
			error = true;
		}
     
}

void Assembler::secondPass(){
	// Create an input stream from the input file.
	ifstream is;
	noSection = true; // Indicates if no section has been defined yet.
	is.open(input_file, ios::in); // Open the input file for reading.
	flexer = new yyFlexLexer(&is); // Initialize the lexer with the input stream.
  
	location_counter = 0; // Reset the location counter for the second pass.
	bool end = false; // A flag to determine if the end of the parsing is reached.
	currentsection = ""; // The current section being processed.
	int pick = 1; // Variable to store the result of parsing operations.

	// Main loop to process each line until the end or an error is found.
	while(pick > 0 && !error && !end){
		pick = pickNextAndRow(); // Process the next item (could be an instruction, label, etc.).
    
		// Error handling: if `pick` returns a specific error code, stop processing.
		if(pick == 111){
			error = true;
			break;
		}
    
		// Skip lines or tokens that are not relevant (e.g., whitespace or comments).
		while(pick == 0 || pick >= 1000){
			pick = flexer->yylex(); // Get the next token from the lexer.
		}

		// If a comment line is detected, continue to skip tokens until the end of the comment.
		if(commentLine){
			while(commentLine){
				flexer->yylex(); // Continue to lex through the comment until it's finished.
			}
		}
		else {
			switch(currentAssembling) {
				// Handling of directives encountered during assembly
				case directive: {
					// If the directive is a section definition
					if(!strcmp(buffer,".section")) {
						int pick = pickNext(); // Move to the next token
						string b = buffer;
						
						// If the buffer ends with a comma, it's an error
						if(b.back() == ',') {
							error = true;
							break;
						}
						
						// Check if the next token is a valid symbol
						if(!readingSymbols) {
							error = true;
							break;
						} else {
							readingSymbols = false; // Reset the flag after reading
							
							// If we're not at the start of a new section, add all literals from the current section's literal pool into the code section, updating the location counter accordingly
							if(!noSection) {
								addPoolCode();
							} else {
								noSection = false; // We're now inside a section
							}

							sectionCounter++; // Increment the section counter
							location_counter = 0; // Reset the location counter for the new section
							currentsection = (string)buffer; // Store the name of the new section
						}
					}   
					// Handling global or extern directives
					else if(!strcmp(buffer,".global") || !strcmp(buffer,".extern")) {
						pickRow(); // Skip to the next line as no additional processing is needed in second pass
						continue;
					}
					// If the directive is a word definition (e.g., ".word")
					else if(!strcmp(buffer,".word")) {
						bool next = true; // Flag to indicate if more values follow
						string put;

						// Loop to handle multiple words in the directive
						while(next) {
							pick = pickNext(); // Move to the next token
							string b = buffer;
							
							// If the buffer does not end with a comma, stop reading more values
							if(b.back() != ',') {
								next = false;
							} else {
								put = buffer;
								put.pop_back(); // Remove the trailing comma
							}

							// Check if the current token is a valid symbol or number
							if(!readingSymbols && !readingNumber) {
								error = true;
								break;
							}
							// If it's a number
							else if(readingNumber) {
								readingNumber = false; // Reset the flag
								
								string b = buffer;
								int cd = stoi(buffer); // Convert the string to an integer
								string hx = getHex(cd, 4); // Convert the integer to a hex string of 4 bytes
								
								// Add each byte of the hex value to the section's code
								for(int i = 0; i < 4; i++) {
									string sub = hx.substr(2*i, 2); // Extract two characters at a time
									sectionTable->addCode(sectionCounter, sub); // Add to the section's code
								}
							}
							// If it's a symbol
							else if(readingSymbols) {
								readingSymbols = false; // Reset the flag
								string sym = buffer;
								
								// If the symbol ends with a comma, it's an error
								if(sym.back() == ',') {
									error = true;
									break;
								}

								// Retrieve symbol information for later use
								int value = symbolTable->getValue(sym);
								string section = symbolTable->getSection(sym);
								SymbolTable::Bind bind = symbolTable->getBind(sym);
								bool error = false; // Reset error flag
								
								// Additional processing would go here (error handling not yet implemented)
								if(error == true) break;
							}  
							// Update the location counter to reflect the number of bytes read
							location_counter += 4;
						}
					}
				

					else if(!strcmp(buffer,".skip")){
						// Handle the .skip directive, which reserves a specified number of bytes in the current section.
						pick = pickNext();
						if(!readingNumber){
							// If the next token is not a number, set the error flag and break out of the loop.
							error = true;
							break;
						}
						else {
							// Convert the number to an integer (number of bytes to skip) and update the location counter.
							string b = buffer;
							location_counter += stoi(b);
							// Fill the skipped space with zeros ("00" in hexadecimal) in the section's code.
							for(int i = 0; i < stoi(b); i++){
								sectionTable->addCode(sectionCounter, "00");
							}
						}
					}
					else if(!strcmp(buffer,".end")){
						// Handle the .end directive, signaling the end of the assembly process.
						end = true;
					}
					else if(!strcmp(buffer,".ascii")){
						// Handle the .ascii directive, which defines a string constant to be stored in the current section.
						pick = pickNext();
						if(!readingString){
							// If the next token is not a string, set the error flag and break out of the loop.
							error = true;
							break;
						}
						else {
							// Remove the enclosing quotes from the string and process each character.
							readingString = false;
							string b = buffer;
							b.pop_back();  // Remove the trailing quote.
							b.erase(0, 1); // Remove the leading quote.
							
							// Convert each character to its hexadecimal representation and add it to the section's code.
							while(b.size() > 0){
								char c = b.back();
								b.pop_back();
								sectionTable->addCode(sectionCounter, getHex((int)c, 1));
								location_counter++;
							}
						}
					}
				}
        case command: {
			if(!strcmp(buffer,"halt")) {
				// If the "halt" command is encountered, add four "00" bytes to indicate a halt instruction.
				for(int i=0; i<4; i++) {
					sectionTable->addCode(sectionCounter,"00");
				}
				location_counter += 4; // Update the location counter by 4 bytes.
			}
			else if(!strcmp(buffer,"int")) {
				// For the int command, add 10 followed by three 00 bytes.
				sectionTable->addCode(sectionCounter,"10");
				for(int i=0; i<3; i++) sectionTable->addCode(sectionCounter,"00");
				location_counter += 4; // Advance the location counter by 4 bytes.
			}
			else if(!strcmp(buffer,"ret")) {
				// The ret command adds specific bytes "93", "FE", "00", and "04" to indicate a return instruction.
				sectionTable->addCode(sectionCounter,"93");
				sectionTable->addCode(sectionCounter,"FE");
				sectionTable->addCode(sectionCounter,"00");
				sectionTable->addCode(sectionCounter,"04");
				location_counter += 4; // Update location counter by 4 bytes.
			}
			else if(!strcmp(buffer,"iret")) {
				// For the iret command, add a sequence of 8 bytes indicating an interrupt return.
				sectionTable->addCode(sectionCounter,"96");
				sectionTable->addCode(sectionCounter,"0E");
				sectionTable->addCode(sectionCounter,"00");
				sectionTable->addCode(sectionCounter,"04");
				sectionTable->addCode(sectionCounter,"93");
				sectionTable->addCode(sectionCounter,"FE");
				sectionTable->addCode(sectionCounter,"00");
				sectionTable->addCode(sectionCounter,"08");
				location_counter += 8; // Update the location counter by 8 bytes.
			}
			else if(!strcmp(buffer,"call")) {
				int pick = pickNext();
				bool op = false;

				// call command expects a symbol or a number as an argument.
				if(!readingSymbols && !readingNumber) {
					error = true;
					break;
				}

				string sym = buffer;
				if(sym.back() == ',') { // Error if the argument has a trailing comma.
					error = true;
					break;
				}

				int val;
				// Process the argument either as a literal or a symbol.
				if(readingNumber) { val = processLiteralSecondPass(buffer); readingNumber = false; }
				else if(readingSymbols) { val = processSymbolSecondPass(buffer); readingSymbols = false; }

				bool error;
				string s = getHex3(val, error);

				// Add the opcode 21 and construct the instruction from the argument's hex value.
				sectionTable->addCode(sectionCounter,"21");
				sectionTable->addCode(sectionCounter,"F0");
				string ss = "0";
				ss.push_back(s[0]);
				s.erase(0,1);
				sectionTable->addCode(sectionCounter,ss);
				sectionTable->addCode(sectionCounter,s);

				location_counter += 4; // Update location counter by 4 bytes.
			}
			else if(!strcmp(buffer,"jmp")) {
				int pick = pickNext();

				// jmp command expects a symbol or a number as an argument.
				if(!readingSymbols && !readingNumber) {
					error = true;
					break;
				}

				string sym = buffer;
				if(sym.back() == ',') { // Error if the argument has a trailing comma.
					error = true;
					break;
				}

				int val;
				// Process the argument either as a literal or a symbol.
				if(readingNumber) { val = processLiteralSecondPass(buffer); readingNumber = false; }
				else if(readingSymbols) { val = processSymbolSecondPass(buffer); readingSymbols = false; }

				bool error;
				string d = getHex3(val, error);

				// Add the opcode "38" and construct the instruction using the hex value of the argument.
				sectionTable->addCode(sectionCounter,"38");
				sectionTable->addCode(sectionCounter,"F0");
				string s = "0";
				s.push_back(d[0]);
				d.erase(0,1);
				sectionTable->addCode(sectionCounter,s);
				sectionTable->addCode(sectionCounter,d);

				location_counter += 4; // Update location counter by 4 bytes.
			}
		 
			else if(currentCommandType==three_args) {  
				int ind;
				string bbb=buffer;

				// Determine the instruction type: beq (0), bne (1), or otherwise, assigns 2
				if(!strcmp(bbb.c_str(),"beq")) ind=0;
				else if(!strcmp(bbb.c_str(),"bne")) ind=1;
				else ind=2;

				string reg[2];
		
				// Loop to read two register arguments
				for(int i=0;i<2;i++){
					pick=pickNext();

					// If the expected register type isn't read, trigger error and break
					if(!readingRegister){
						error=true;
						break;
					}
			
					readingRegister=false;
					string b=buffer;
			
					// Validate if the register ends with a comma
					if(b.back()!=','){
						error=true;
						break;
					}

					// Remove trailing comma and prefix (e.g., '$')
					b.pop_back();
					b.erase(0,1);

					// Determine register encoding (p -> F, s -> E, or numerical)
					if(b[0]=='p') reg[i]="F";
					else if(b[0]=='s') reg[i]="E";
					else {
						b.erase(0,1);
						reg[i]=getHexChar(stoi(b));  // Convert register number to hexadecimal character
					}
				}
		
				// Read the next value as either a symbol or number
				pick=pickNext();
				if(!readingSymbols && !readingNumber){
					error=true;
					break;
				}

				string b=buffer;

				// Validate format by checking for trailing comma
				if(b.back()==','){
					error=true;
					break;
				}

				int v;

				// Process symbol or literal depending on input, converting to hex
				if(readingSymbols) { v=processSymbolSecondPass(buffer); readingSymbols=false; }
				else if(readingNumber){ v= processLiteralSecondPass(buffer); readingNumber=false; }

				bool error;
				string d=getHex3(v,error);

				// Encode instruction type for branch commands
				if(ind==0) sectionTable->addCode(sectionCounter,"39");
				else if(ind==1) sectionTable->addCode(sectionCounter,"3A");
				else sectionTable->addCode(sectionCounter,"3B");

				string f="F";  // Default prefix for encoding
				f.append(reg[0]);  // Append first register code
				reg[1].push_back(d[0]);  // Append relevant data to second register code
				d.erase(0,1);  // Remove processed part of 'd'
		
				// Add encoded instruction and operands to section table
				sectionTable->addCode(sectionCounter,f);
				sectionTable->addCode(sectionCounter,reg[1]);
				sectionTable->addCode(sectionCounter,d);
		
				// Update location counter for instruction length
				location_counter+=4;
			}
			else if(!strcmp(buffer, "push")) {
				// Handle the "push" command
				int pick = pickNext();
				string reg;

				// Check if the next argument is a valid register
				if(!readingRegister) {
					error = true;
					break;
				} else {
					string b = buffer;

					// Ensure there is no trailing comma in the register argument
					if(b.back() == ',') {
						error = true;
						break;
					}

					b.erase(0, 1); // Remove the leading 'r'

					// Convert register to its corresponding hexadecimal value
					if(b[0] == 'p') reg = "F";  // For "pc" register
					else if(b[0] == 's') reg = "E";  // For "sp" register
					else {
						b.erase(0, 1);
						reg = getHexChar(stoi(b));
					}
					readingRegister = false;

					// Add opcode and relevant register codes for the "push" operation
					sectionTable->addCode(sectionCounter, "81");  // Opcode for "push"
					sectionTable->addCode(sectionCounter, "E0");  // Additional command bits
					reg.push_back('F');  // Append "F" to specify push to stack
					sectionTable->addCode(sectionCounter, reg);
					sectionTable->addCode(sectionCounter, "FC");  // Command to finalize push
				}
				location_counter += 4;  // Update location counter
			}

			else if(!strcmp(buffer, "pop")) {
				// Handle the "pop" command
				int pick = pickNext();
				string reg;

				// Check if the next argument is a valid register
				if(!readingRegister) {
					error = true;
					break;
				} else {
					string b = buffer;

					// Ensure there is no trailing comma in the register argument
					if(b.back() == ',') {
						error = true;
						break;
					}

					b.erase(0, 1); // Remove the leading 'r'

					// Convert register to its corresponding hexadecimal value
					if(b[0] == 'p') reg = "F";  // For "pc" register
					else if(b[0] == 's') reg = "E";  // For "sp" register
					else {
						b.erase(0, 1);
						reg = getHexChar(stoi(b));
					}
					readingRegister = false;

					// Add opcode and relevant register codes for the "pop" operation
					sectionTable->addCode(sectionCounter, "93");  // Opcode for "pop"
					reg.push_back('E');  // Append "E" to specify pop from stack
					sectionTable->addCode(sectionCounter, reg);
					sectionTable->addCode(sectionCounter, "00");
					sectionTable->addCode(sectionCounter, "04");  // Command to finalize pop
				}
				location_counter += 4;  // Update location counter
			}

			else if(!strcmp(buffer, "xchg")) {
				// Handle the "xchg" (exchange) command
				pick = pickNext();

				// Ensure that the first argument is a register
				if(!readingRegister) {
					error = true;
					break;
				}

				string b = buffer;

				// Check for trailing comma in the register argument
				if(b.back() != ',') {
					error = true;
					break;
				} else {
					b.pop_back();  // Remove the comma
				}
				b.erase(0, 1);  // Remove the leading 'r'

				string reg1;

				// Convert first register to its hexadecimal code
				if(b[0] == 'p') reg1 = "F";  // For "pc" register
				else if(b[0] == 's') reg1 = "E";  // For "sp" register
				else {
					b.erase(0, 1);
					reg1 = getHexChar(stoi(b));
				}
				readingRegister = false;

				pick = pickNext();

				// Ensure that the second argument is also a register
				if(!readingRegister) {
					error = true;
					break;
				}

				b = buffer;

				// Check for trailing comma in the second register argument
				if(b.back() == ',') {
					error = true;
					break;
				}
				readingRegister = false;

				string reg2;
				b.erase(0, 1);  // Remove the leading 'r'

				// Convert second register to its hexadecimal code
				if(b[0] == 'p') reg2 = "F";  // For "pc" register
				else if(b[0] == 's') reg2 = "E";  // For "sp" register
				else {
					b.erase(0, 1);
					reg2 = getHexChar(stoi(b));
				}

				// Add opcode and register codes for "xchg" operation
				sectionTable->addCode(sectionCounter, "40");  // Opcode for "xchg"
				string r = "0";
				r.append(reg1);  // Append first register code
				sectionTable->addCode(sectionCounter, r);
				reg2.push_back('0');  // Append "0" to specify register pair exchange
				sectionTable->addCode(sectionCounter, reg1);
				sectionTable->addCode(sectionCounter, "00");  // Finalize xchg command
				location_counter += 4;  // Update location counter
			}

            else if(!strcmp(buffer, "add") || !strcmp(buffer, "sub") || !strcmp(buffer, "mul") || !strcmp(buffer, "div") || !strcmp(buffer, "and") || !strcmp(buffer, "or") || !strcmp(buffer, "xor") || !strcmp(buffer, "shl") || !strcmp(buffer, "shr")) {
				// Handle arithmetic and logical operations

				string buf = buffer;  // Store operation name
				pick = pickNext();  // Move to the next part of the command
				string reg1, reg2, reg3;

				// Verify if the next argument is a register
				if(!readingRegister) {
					error = true;
					break;
				}

				string b = buffer;

				// Check if there is a comma after the first register argument
				if(b.back() != ',') {
					error = true;
					break;
				}

				b.pop_back();  // Remove the comma
				b.erase(0, 1); // Remove leading 'r'

				// Determine the register code for the first register
				if(b[0] == 'p') reg1 = "F";  // Special register "pc"
				else if(b[0] == 's') reg1 = "E";  // Special register "sp"
				else {
					b.erase(0, 1);
					reg1 = getHexChar(stoi(b));
				}
				readingRegister = false;

				pick = pickNext();  // Move to the next register

				// Check if the second argument is a valid register
				if(!readingRegister) {
					error = true;
					break;
				}

				b = buffer;

				// Ensure there is no trailing comma in the second register argument
				if(b.back() == ',') {
					error = true;
					break;
				}

				b.erase(0, 1);  // Remove leading 'r'

				// Determine the register codes for the second and third registers
				if(b[0] == 'p') reg2 = reg3 = "F";  // Special register "pc"
				else if(b[0] == 's') reg2 = reg3 = "E";  // Special register "sp"
				else {
					b.erase(0, 1);
					reg2 = reg3 = getHexChar(stoi(b));
				}
				readingRegister = false;

				// Add the corresponding opcode based on the operation
				if(!strcmp(buf.c_str(), "add")) sectionTable->addCode(sectionCounter, "50");  // Opcode for "add"
				else if(!strcmp(buf.c_str(), "sub")) sectionTable->addCode(sectionCounter, "51");  // Opcode for "sub"
				else if(!strcmp(buf.c_str(), "mul")) sectionTable->addCode(sectionCounter, "52");  // Opcode for "mul"
				else if(!strcmp(buf.c_str(), "div")) sectionTable->addCode(sectionCounter, "53");  // Opcode for "div"
				else if(!strcmp(buf.c_str(), "and")) sectionTable->addCode(sectionCounter, "61");  // Opcode for "and"
				else if(!strcmp(buf.c_str(), "or")) sectionTable->addCode(sectionCounter, "62");  // Opcode for "or"
				else if(!strcmp(buf.c_str(), "xor")) sectionTable->addCode(sectionCounter, "63");  // Opcode for "xor"
				else if(!strcmp(buf.c_str(), "shl")) sectionTable->addCode(sectionCounter, "70");  // Opcode for "shift left"
				else if(!strcmp(buf.c_str(), "shr")) sectionTable->addCode(sectionCounter, "71");  // Opcode for "shift right"

				reg2.append(reg3);  // Combine reg2 and reg3 into one string
				reg1.push_back('0');  // Append '0' to reg1 to complete the instruction format

				// Add the processed codes to the section table
				sectionTable->addCode(sectionCounter, reg2);  // Second and third register code
				sectionTable->addCode(sectionCounter, reg1);  // First register code with '0' appended
				sectionTable->addCode(sectionCounter, "00");  // Final byte of instruction

				location_counter += 4;  // Update the location counter
			}
            // Check if the current instruction is the "not" operation
			else if(!strcmp(buffer, "not")) {
				int pick = pickNext(); // Calls pickNext() to retrieve the next token
				string reg; // Defines a variable to store the register code

				// Check if a valid register is being read
				if(!readingRegister) {
					error = true; // Set error to true if a valid register is not being read
					break;
				} else {
					string b = buffer; // Copy the contents of `buffer` into `b`

					// Check if there is an incorrect comma at the end of the register name
					if(b.back() == ',') {
						error = true; // Set error to true if there's a misplaced comma
						break;
					}

					b.erase(0, 1); // Remove the first character of `b` (the register indicator)

					// Assign specific codes based on the register type
					if(b[0] == 'p') 
						reg = "FF"; // Set `reg` to "FF" for a certain type of register
					else if(b[0] == 's') 
						reg = "EE"; // Set `reg` to "EE" for another type of register
					else {
						b.erase(0, 1); // Remove additional character(s) for processing the numeric part
						reg = getHexChar(stoi(b)); // Convert string to integer and get hexadecimal character
						reg.push_back(getHexChar(stoi(b))); // Append the hex character again for the complete code
					}
					readingRegister = false; // Reset readingRegister flag to indicate register processing is done
				}

				// Add operation and register code to the section table
				sectionTable->addCode(sectionCounter, "60"); // "60" indicates the "not" operation
				sectionTable->addCode(sectionCounter, reg);  // Add the register code
				sectionTable->addCode(sectionCounter, "00"); // Add padding bytes for alignment
				sectionTable->addCode(sectionCounter, "00");
				
				location_counter += 4; // Increment location counter by 4 to account for instruction length
			}

                else if(!strcmp(buffer,"ld")){

                  pick=pickNext();
                  string b=buffer;
                   string rg="";
                   int number=-1;
                   if(b.back()!=',') {

                      error=true;
                      break;
                    }
                  if(!readingSymbols && !readingNumber && !readingOperand){

                    error=true;
                    break;
                  }
                  else{
                    bool op=false;
                    int v;
                    string symbol_name;

                    if(readingNumber){
                      readingNumber=false;
                      v=processLiteralSecondPass(buffer);
                    }
                    else if(readingOperand){
                      readingOperand=false;
                      v=processOperandSecondPass(buffer,rg,number,symbol_name);
                      op=true;
                    }
                    else if(readingSymbols){
                      readingSymbols=false;
                      v=processSymbolSecondPass(buffer);
                    }
                    

                    string vv=getHex3(v,error);
                    string dd;
                    if(number==-1) dd="000";
                    else dd=getHex3(number,error);
                    if(error){
                      cout<<"-------Greska:Operand nije moguce predstaviti na 12 bita"<<endl;
                      break;
                    }
                    pick=pickNext();
                    string reg;

                    if(!readingRegister){
                      error=true;
                      break;
                    }
                    else{
                      string b=buffer;
                      if(b.back()==',') {
                        error=true;
                        break;
                      }
                      
                      b.erase(0,1);
                      if(b[0]=='p') reg="F";
                      else if(b[0]=='s') reg="E";
                      else {
                        b.erase(0,1);
                        reg=getHexChar(stoi(b));
                      }
                        string reg1=reg;
                        string reg2=reg;
                      
                      readingRegister=false;
                      sectionTable->addCode(sectionCounter,"92");
                     
                      if((op && rg.size()==0 )||!op){
                        reg.push_back('F');
                        string zero="0";
                        sectionTable->addCode(sectionCounter,reg);
                        zero.push_back(vv[0]);
                        vv.erase(0,1);
                        sectionTable->addCode(sectionCounter,zero); 
                        sectionTable->addCode(sectionCounter,vv);
                      }
                      else if(op && rg.size()>0){
                        reg.append(rg);
                        string zero="0";
                        sectionTable->addCode(sectionCounter,reg);
                        zero.push_back(dd[0]);
                        dd.erase(0,1);
                        sectionTable->addCode(sectionCounter,zero); 
                        sectionTable->addCode(sectionCounter,dd);
                      }
                      
                      if(!op){
                        sectionTable->addCode(sectionCounter,"92");
                        reg.pop_back();
                        reg.append(reg);
                        sectionTable->addCode(sectionCounter,reg);
                        sectionTable->addCode(sectionCounter,"00");
                        sectionTable->addCode(sectionCounter,"00");
                        if(strcmp(symbol_name.c_str(),"")){
                          SymbolTable::Bind b=symbolTable->getBind(symbol_name);
                          int loc=symbolTable->getValue(symbol_name);
                        }
                        location_counter+=4;
                      }
                    }
                  }
                  location_counter+=4;
                 }
                 else if(!strcmp(buffer,"st")){
                  pick=pickNext();

                  if(!readingRegister){
                    error=true;
                    break;
                  }
                  else{
                    readingRegister=false;
                    string reg;
                    string b=buffer;
                    if(b.back()!=',') {
                      error=true;
                      break;
                    }
                    b.erase(0,1);
                    if(b[0]=='p') reg="F";
                      else if(b[0]=='s') reg="E";
                      else {
                        b.erase(0,1);
                        reg=getHexChar(stoi(b));
                      }
                             
                    pick=pickNext();
                    if(!readingSymbols && !readingNumber && !readingOperand) {
                      error=true;
                      break;
                    }
                    

                    int v;
                    int number;
                    string rg;
                    string symbol_name;
                    bool op=false;
                    if(readingNumber){

                      readingNumber=false;
                     v= processLiteralSecondPass(buffer);
                    }
                    else if(readingOperand){

                      readingOperand=false;
                     v= processOperandSecondPass(buffer,rg,number,symbol_name);
                     op=true;
                    }
                    else if(readingSymbols){

                     readingSymbols=false;
                     v= processSymbolSecondPass(buffer);
                    }
                    else {

                    }

                    string ret=getHex3(v,error);

                    if(!op){
                      sectionTable->addCode(sectionCounter,"82");
                      sectionTable->addCode(sectionCounter,"F0");
                      reg.push_back(ret[0]);
                      ret.erase(0,1);
                      sectionTable->addCode(sectionCounter,reg);
                      sectionTable->addCode(sectionCounter,ret);

                    }
                    else{
                      sectionTable->addCode(sectionCounter,"80");
                      rg.push_back('0');
                      sectionTable->addCode(sectionCounter,rg);
                      string dd;
                      if(number==-1) dd="000";
                      else dd=getHex3(number,error);
                      reg.push_back(dd[0]);
                      dd.erase(0,1);
                      sectionTable->addCode(sectionCounter,reg);
                      sectionTable->addCode(sectionCounter,dd);
                      if(strcmp(symbol_name.c_str(),"")){
                          SymbolTable::Bind b=symbolTable->getBind(symbol_name);
                          int loc=symbolTable->getValue(symbol_name);
                        }

                    }
                  
                    location_counter+=4;
                  }
                }
                  else if(!strcmp(buffer,"csrrd")){
                  pick=pickNext();
                  if(!readingCSR){
                    error=true;
                    break;
                  }
                  readingCSR=false;
                  string b=buffer;
                  if(b.back()!=','){
                    error=true;
                    break;
                  }
                  string reg;
                  string csr;
                  b.erase(0,1);
                  if(b[0]=='s') csr="0";
                  else if(b[0]=='h') csr="1";
                  else csr="2";
                  pick=pickNext();
                  if(!readingRegister){
                    error=true;
                    break;
                  }
                  readingRegister=false;
                  b=buffer;
                  if(b.back()==','){
                    error=true;
                    break;
                  }
                  b.erase(0,1);
                  if(b[0]=='p') reg="F";
                  else if(b[0]=='s') reg="E";
                  else {
                        b.erase(0,1);
                        reg=getHexChar(stoi(b));
                  }
                  sectionTable->addCode(sectionCounter,"90");
                  reg.append(csr);
                  sectionTable->addCode(sectionCounter,reg);
                  sectionTable->addCode(sectionCounter,"00");
                  sectionTable->addCode(sectionCounter,"00");
                  location_counter+=4;

                }else if(!strcmp(buffer,"csrwr")){
                  pick=pickNext();
                  string reg;
                  string csr;
                  if(!readingRegister){
                    error=true;
                    break;
                  }
                  readingRegister=false;
                  string b=buffer;
                  if(b.back()!=','){
                    error=true;
                    break;
                    
                  }
                  b.erase(0,1);
                 if(b[0]=='p') reg="F";
                  else if(b[0]=='s') reg="E";
                  else {
                        b.erase(0,1);
                        reg=getHexChar(stoi(b));
                  }

                  pick=pickNext();
                  if(!readingCSR){
                    error=true;
                    break;
                  }
                  readingCSR=false;
                  b=buffer;
                  if(b.back()==','){
                    error=true;
                    break;
                  }
                  b.erase(0,1);
                  if(b[0]=='s') csr="0";
                  else if(b[0]=='h') csr="1";
                  else csr="2";
                  sectionTable->addCode(sectionCounter,"94");
                  csr.append(reg);
                  sectionTable->addCode(sectionCounter,csr);
                  sectionTable->addCode(sectionCounter,"00");
                  sectionTable->addCode(sectionCounter,"00");
                  location_counter+=4;

                }
          break;
        }
        case label:{
              if(!readingLabel){
                error=true;
                break;
              }
              readingLabel=false;
              break;
            }   
        
      }

  }  
  }
  if(!end){error=true;cout<<"Nema kraja!"<<endl;}
  else addPoolCode();
  if(symbolTable->checkForErrors())
  print();
  else cout<<"Postoje nedefinisani simboli!"<<endl;


}

int Assembler::convertToDecimal(string hex){
  int factor=0;
  int res=0;
  if(hex.size()==10&&hex[2]>='8' && hex[2]<='f') {while(hex.size()>2){
    char x=hex.back();
    hex.pop_back();
    int h=getNotChar(x);
    res+=h*pow(16,factor);
    factor++;
  }
  res++;
  return -res;
  }
  else {while(hex.size()>2){
    char x=hex.back();
    hex.pop_back();
    int h=getChar(x);
    res+=h*pow(16,factor);
    factor++;
  }
  }
  return res;
}

void Assembler::putInBuffer(const char* b){
  buffer=b;
}

void Assembler::addToSections(string sectionName){

  if(sections.find(sectionName)==sections.end()){
    sections[sectionName]=sections.size();
  }
}

int Assembler::pickNext(){
  int p=0;
  while(p==0){
     p=flexer->yylex();
  }
  return p;
}
int Assembler::pickRow(){
  int p=0;
  while(p!=1000){
     p=flexer->yylex();
  }
  return p;
}

int Assembler::pickNextAndRow(){
  int p=0;
  while(p==0|| p==1000){
     p=flexer->yylex();
  }
  return p;
}

void Assembler::processOperandFirstPass(string buffer){
                      string b=buffer;
                      int number=-1;
                      string symbol_name="";
                      bool reg=false;
                      if(b.back()==',') b.pop_back();
                      if(b[0]=='$') {b.erase(0,1);                     
}
                     // bool that=false;
                    //  if(b[0]!='[' && b[0]!='%') {that=true;}
                      int i=0;
                      if(b[0]=='['){
                        reg=true;
                        for(i=0;i<b.size();i++) {
                          char c=b[i];
                          if(b[i]=='+') break;
                        }
                        if(i!=b.size()) {
                          b.erase(0,i+1);
                          if(b[0]==' ') b.erase(0,1);
                          b.pop_back();
                          if((b[0]>='0' && b[0]<='9')) {
                           // that=true;
                            if(b[1]=='x'){
                            number=convertToDecimal(b);
                            }
                            else number=stol(b);
                            


                          }
                          else symbol_name=b;
                        }
                      }
                      else{
                         if((b[0]>='0' && b[0]<='9')) {
                           // that=true;
                            if(b[1]=='x'){
                              number=convertToDecimal(b);

                            }
                            else number=stoul(b);
                          }
                          else symbol_name=b;
                      }
                      if(strcmp(symbol_name.c_str(),"") &&!symbolTable->isDeclared(b)){
                        symbolTable->insertNewSymbol(symbol_name,-1,0,SymbolTable::OBJECT,SymbolTable::LOCAL,"no_section");
                        
                      }
                      if(number!=-1){
                        sectionTable->addLiteral(sectionTable->getSize()-1,number,4,-1);
                      }
                      readingOperand=false;
                      
}
void Assembler::processSymbolFirstPass(string buffer){
                      string b=buffer;
                      if(b.back()==',') b.pop_back();
                      
                      if(!symbolTable->isDeclared(b)){
                        symbolTable->insertNewSymbol(b,-1,0,SymbolTable::OBJECT,SymbolTable::LOCAL,"no_section");

                      }
                      readingSymbols=false;
                    
}
void Assembler::processLiteralFirstPass(string buffer){
  string b=buffer;
  if(b.back()==',') b.pop_back();
  int number;
  if(b[1]=='x') number=convertToDecimal(b);
  else number=stoul(b);
  sectionTable->addLiteral(sectionTable->getSize()-1,number  ,4,-1);
  readingNumber=false;

}

int Assembler::getNotChar(char c){
  if(c>='0' && c<='9') return 15-(c-'0');
  else if(c>='a' && c<='f')return 5-(c-'a');
  else if(c>='A' && c<='F')return 5-(c-'A');

  else return -1;
  
}
int Assembler::getChar(char c){
  if(c>='0' && c<='9') return (c-'0');
  else if(c>='a' && c<='f')return (10+c-'a');
  else if(c>='A' && c<='F')return (10+c-'A');

  else return -1;
  
}
void Assembler::mediumPass(){
  sectionTable->start();
}
void Assembler::print(){
  ofstream out(output_file,ios::out);
  //out<<"----Symbol Table----"<<endl;
  out<<"#.symtab"<<endl;
  symbolTable->print(out);
  sectionTable->print(out);
}
int Assembler::processOperandSecondPass(string buffer,string& reg,int& number,string& symbol_name){
                      reg="";
                      string b=buffer;
                      number=-1;
                      symbol_name="";
                      if(b.back()==',') b.pop_back();
                      if(b[0]=='$') {b.erase(0,1);  }
                      bool readingReg=false; 
                      int i=0;
                      int val=0;
                      bool flag=false;
                      if(b[0]=='['){
                        readingReg=true;
                        for(i=0;i<b.size();i++) {
                          char c=b[i];
                          if(!flag &&c>='0' && c<='9') {
                            string a="";
                            a.push_back(c);
                            val*=10;
                            val+=stoi(a);
                          }
                          else if(!flag &&c=='p') {reg="F";flag=true;}
                          else if(!flag &&c=='s'){reg="E",flag=true;}

                          if(b[i]=='+') break;
                        }
                        if(!flag) reg=getHexChar(val);
                        //cout<<b<<" "<<i<<endl;
                        int x=b.size();
                        b.erase(0,i+1);
                        if(b[0]==' ') b.erase(0,1);

                        if(i!=x) {
                          b.pop_back();
                          //b.erase(0,3);
                          if((b[0]>='0' && b[0]<='9')) {
                           // that=true;
                            if(b[1]=='x'){
                            number=convertToDecimal(b);
                            }
                            else {number=stoul(b);}
                          }
                          else symbol_name=b;
                          return -1;

                        }
                      }
                      else{
                         if((b[0]>='0' && b[0]<='9')) {
                            if(b[1]=='x'){
                              number=convertToDecimal(b);

                            }
                            else number=stoul(b);

                          }
                          else symbol_name=b;
                      }
                      readingOperand=false;
                      SectionTable::Section s=sectionTable->getSection(currentsection);

                      if(strcmp(symbol_name.c_str(),"") ){
                        int value;
                        value=symbolTable->getValue(symbol_name);
                        SymbolTable::Bind bind=symbolTable->getBind(symbol_name);
                        int sz=s.getNextFreeSpace();
                        //if(!s.lits->exists(value)) s.lits->addLiteral(value,4,sz);
                        //sectionTable->addLiteral(sectionCounter,value,4,sz);
                        if(s.lits.getLocation(symbol_name)==0){
                        sectionTable->addLiteral(s.ind,value,4,sz,b);
                        SymbolTable::Bind bind=symbolTable->getBind(symbol_name);
                        if(bind==SymbolTable::GLOBAL)
                        sectionTable->addRelocation(sectionCounter,sz,symbol_name,0);
                        else 
                        sectionTable->addRelocation(sectionCounter,sz,s.name,value);
                      }
                        return(sectionTable->getSection(currentsection).lits.getLocation(symbol_name)-location_counter-4);
                      }
                      else if(number!=-1){
                        int num=number;
                        if(!readingReg) number=-1;
                        return(s.lits.getLocation(num)-location_counter-4);
                      }
                      else return -1;
}
int Assembler::processSymbolSecondPass(string buffer){
                      string b=buffer;
                      if(b.back()==',') b.pop_back();
                      readingSymbols=false;
                       
                      int value;
                      value=symbolTable->getValue(b);
                      SectionTable::Section s=sectionTable->getSection(currentsection);
                      int sz=s.getNextFreeSpace();
                      //if(!s.lits->exists(value)) s.lits->addLiteral(value,4,sz);
                      //sectionTable->addLiteral(currentsection,value,4,sz);

                      if(s.lits.getLocation(b)==0){
                 
                       
                        sectionTable->addLiteral(s.ind,value,4,sz,b);
                        SymbolTable::Bind bind=symbolTable->getBind(b);
                        if(strcmp(symbolTable->getSection(b).c_str(),"BSS") && bind==SymbolTable::GLOBAL)
                        sectionTable->addRelocation(s.ind,sz,b,0);
                        else if(strcmp(symbolTable->getSection(b).c_str(),"BSS"))
                        sectionTable->addRelocation(s.ind,sz,s.name,value);
                      }

                      return(sectionTable->getSection(currentsection).lits.getLocation(b)-location_counter-4);
                    
}
int Assembler::processLiteralSecondPass(string buffer){
  string b=buffer;
  if(b.back()==',') b.pop_back();
  int number;
  if(b[1]=='x') number=convertToDecimal(b);
  else number=stoul(b);
  readingNumber=false;
  SectionTable::Section s=sectionTable->getSection(sectionCounter);

  return (s.lits.getLocation(number)-location_counter-4);
}

string Assembler::getHex3(int number, bool& error){
  string ret;
  error=false;
  if(number>0){
    for(int i=0;i<3;i++){
      int num=number%16;
      char c=getHexChar(num);
      string s="";
      s.push_back(c);
      ret.insert(0,s);
      number=number>>4;
    }
  }
  else{
    number=-number;
    bool add=true;
    for(int i=0;i<3;i++){
      int num=number%16;
      char c=getHexCharInverted(num);
      if(add){
        if(c=='F') c='0';
        else {c++;add=false;}
      }
      string s="";
      s.push_back(c);
      ret.insert(0,s);
      number=number>>4;
    }
  }
  if(number!=0) error=true;
  return ret;
}
void Assembler::addPoolCode(){
  // Retrieve the current section from the section table using the sectionCounter
  SectionTable::Section s = sectionTable->getSection(sectionCounter);
  
  // Iterate through all literals stored in the current section's literal table
  for(LiteralTable::Literal& l : s.lits.lits){
    
    // Get the integer value of the literal
    int val = l.value;
    // Convert the integer value to a hexadecimal string of 4 bytes (8 hex characters)
    string s = getHex(val, 4);

    // Process the hexadecimal string in chunks of 2 characters (1 byte)
    while(s.size() > 0){
      // Extract the first 2 characters (representing one byte) from the string
      string sub = s.substr(0, 2);
      // Remove the processed characters from the original string
      s.erase(0, 2);
      // Add the byte to the section's code table
      sectionTable->addCode(sectionCounter, sub);
    }

    // Update the location counter by the size of the current literal
    location_counter += l.size;
  }
}


