#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include<sstream>
#include<algorithm>
#include<bitset>
using namespace std;
void removeSpaces(string str, string& instruction);
vector<string> parseCSV(string instruction);
struct blockObject {
    int address;
    string instruction; //value/data
};
struct dataObject {
    int address;
    string bits; //value/data
};
struct labelObject {
    int address;
    string label;
};
void generateBlocks(vector<blockObject>&,vector<labelObject>&, int);
int translate(string reg);
void translateInstruction(string, string);
bool beq(string, string&);
bool bne(string, string&);
bool blt(string, string&);
bool bge(string, string&);

int branch(vector<labelObject>, string, int, int);
int jalr(string, int*, int);
string jal(string, int*, int);
vector<dataObject> generateDataMemory(int&);
int stToInt(string);

void find_and_store_byte(vector<dataObject> &data, int address, string bits);
void find_and_load(vector<dataObject> data, int address, string& value);
void load_or_store(vector<dataObject>& data, int a[], string instruction);
void lui(string instruction, int a[]);
int regArray[32] = { 0 };
int rd, rs1, rs2;

void R_I_format(string registers, char format_type);
void R_format_functions(string instruction, int* register_operands);
void I_format_constants_functions(string instruction, int* register_operands);


void printAll(vector<blockObject> memory, vector<dataObject> memoryData, int* regArray, int);
int main() {
    vector<blockObject> memory;
    vector<labelObject> labels;
    int programCounter;
    //call data file function generator
    vector<dataObject> memoryData = generateDataMemory(programCounter);
    int startingAddress = programCounter;
    //cout << "Enter starting address\n";
    //cin >> startingAddress;
    generateBlocks(memory,labels, startingAddress);
    int i;
    for (i = 0; i < memory.size(); i++) {
        programCounter = memory[i].address;
        cout << "instruction executed : \t" << programCounter << "\t" << memory[i].instruction << "at i = " << i << "\n";
        if (memory[i].instruction.find("beq") != -1)
        {
            string label;
            if (beq(memory[i].instruction, label)) //if branching is required
                i = branch(labels, label, i, programCounter);
        }
        else if (memory[i].instruction.find("bne") != -1)
        {
            string label;
            if (bne(memory[i].instruction, label)) //if branching is required
                i = branch(labels, label, i, programCounter);
        }
        else if (memory[i].instruction.find("blt") != -1)
        {
            string label;
            if (blt(memory[i].instruction, label)) //if branching is required
                i = branch(labels, label, i, programCounter);
        }
        else if (memory[i].instruction.find("bge") != -1)
        {
            string label;
            if (bge(memory[i].instruction, label)) //if branching is required
                i = branch(labels, label, i, programCounter);
        }
        else if (memory[i].instruction.find("jalr") != -1)
        {
            int nextAddress;
            //if (i < memory.size() - 1)
              //  nextAddress = memory[i + 1].address;
            nextAddress = memory[i].address + 4;
            int jumpToAddress = jalr(memory[i].instruction, regArray, nextAddress);
            //i = branch(labels, label, i);
            int difference = jumpToAddress - programCounter;
            i += (difference / 4);
            i--;
        }
        else if (memory[i].instruction.find("jal") != -1)
        {
            int nextAddress;
            //if (i < memory.size() - 1)
              //  nextAddress = memory[i + 1].address;
            nextAddress = memory[i].address + 4;
            string label = jal(memory[i].instruction, regArray, nextAddress);
            i = branch(labels, label, i, programCounter);
        }
        //large before small
        else if (memory[i].instruction.find("lhu") != -1 || memory[i].instruction.find("lbu") != -1 ||
            memory[i].instruction.find("lw") != -1 || memory[i].instruction.find("sw") != -1 ||
            memory[i].instruction.find("sh") != -1 || memory[i].instruction.find("sb") != -1)
            {
                load_or_store(memoryData, regArray, memory[i].instruction);
            }
        else if (memory[i].instruction.find("lb") != -1 || memory[i].instruction.find("lh") != -1)
            {
                load_or_store(memoryData, regArray, memory[i].instruction);
            }
        else if (memory[i].instruction.find("lui") != -1)
            {
                lui(memory[i].instruction, regArray);
            }
        else if (memory[i].instruction.find("addi") != -1 || memory[i].instruction.find("sltiu") != -1 ||
            memory[i].instruction.find("xori") != -1 || memory[i].instruction.find("andi") != -1 ||
            memory[i].instruction.find("ori") != -1 || memory[i].instruction.find("slli") != -1 ||
            memory[i].instruction.find("srli") != -1 || memory[i].instruction.find("srai") != -1)
            {
                I_format_constants_functions(memory[i].instruction, regArray);
            }
        else  if (memory[i].instruction.find("slti") != -1)
            {
                I_format_constants_functions(memory[i].instruction, regArray);
            }
        else  if (memory[i].instruction.find("sltu") != -1)
            {
                R_format_functions(memory[i].instruction, regArray);
            }
        else if (memory[i].instruction.find("add") != -1 || memory[i].instruction.find("slt") != -1 ||
            memory[i].instruction.find("xor") != -1 || memory[i].instruction.find("and") != -1 ||
            memory[i].instruction.find("or") != -1 || memory[i].instruction.find("sll") != -1 ||
            memory[i].instruction.find("srl") != -1 || memory[i].instruction.find("sra") != -1||
            memory[i].instruction.find("sub") != -1)
            {
                R_format_functions(memory[i].instruction, regArray);
            }
        regArray[0] = 0;
        printAll(memory, memoryData, regArray, programCounter);
    }
    return 0;
}

//a function that takes the text file and generate a vector of blocks. each block contains a set of instructions and starts with its label
vector<dataObject> generateDataMemory(int &programCounter) {
    vector<dataObject> data;
    dataObject tempStack;
    ifstream myfile("data.txt");
    string line;
    vector<string> allLines;
    if (!myfile.is_open())
        cout << "Unable to open file";
    else
        while (getline(myfile, line))
            allLines.push_back(line);
    programCounter = stoi(allLines[0]);
    for (int i = 1; i < allLines.size(); i++) //loop over the lines
    {
        vector<string> temp = parseCSV(allLines[i]);
        int gotoAddress = stoi(temp[0]);
        string type = temp[1];
        //you have the type in (type), address in (gotoAddress)
        int k = 0;
        for (int j=2;j<temp.size();j++) //loop over the values in each line (after stating the type [b,hw,w])
        {
            int decimal = stToInt(temp[j]);
            //cout << "Decimal parsed: " << decimal << endl;
            bitset<32> binary;
            binary = decimal;
            //cout << "Binary parsed: " << binary << endl;
            string binaryString = binary.to_string();
            //cout << "Binary string: " << binaryString << endl;
            if (type == "b")
            {
                binaryString = binaryString.substr(24, 8);
                //cout << "Trimmed Binary string: " << binaryString << endl;
                find_and_store_byte(data, gotoAddress + k, binaryString);
                k++;
            }
            else if (type =="hw")
            {
                string binaryStringRight = binaryString.substr(24, 8);
                string binaryStringLeft  = binaryString.substr(16, 8);
                //cout << "Trimmed Binary string (RIGHT): " << binaryStringRight << endl;
                //cout << "Trimmed Binary string (LEFT): " << binaryStringLeft << endl;
                find_and_store_byte(data, gotoAddress + k, binaryStringRight);
                k++;
                find_and_store_byte(data, gotoAddress + k, binaryStringLeft);
                k++;
            }
            else
            {
                vector<string> binaryStringVector(4);
                binaryStringVector[0] = binaryString.substr(0, 8);
                binaryStringVector[1] = binaryString.substr(8, 8);
                binaryStringVector[2] = binaryString.substr(16, 8);
                binaryStringVector[3] = binaryString.substr(24, 8);
                //cout << "from left to right\n";
                /*for (int l=0;l< binaryStringVector.size();l++)
                    cout << "Trimmed Binary string at: " << l << "\t" << binaryStringVector[l] << endl;*/

                for (int a = binaryStringVector.size()-1; a >= 0; a--, k++)
                {
                    find_and_store_byte(data, gotoAddress + k, binaryStringVector[a]);
                    //cout << "done\n";
                }
            }
        }
    }    
    //cout << endl << endl << endl << "Memory Data:\n";
    //for (int i = 0; i < data.size(); i++)
    //    cout << data[i].address << "\t" << data[i].bits << endl;
    //cout << endl << endl;
    myfile.close();
    return data;
}

//a function that takes the text file and generate a vector of blocks. each block contains a set of instructions and starts with its label
void generateBlocks(vector<blockObject>& memory, vector<labelObject>& labels, int startingAddress){
    int currentAddress = startingAddress; //pointing towards the first empty space
    blockObject tempMemory;
    labelObject tempLabel;
    ifstream myfile("instructions.txt");
    string instruction;
    if (!myfile.is_open())
        cout << "Unable to open file";
    else
    {
        while (getline(myfile, instruction))
        {
            if (instruction.find(':') != -1)
            {
                tempLabel.address = currentAddress;
                tempLabel.label = instruction;
                labels.push_back(tempLabel);
                getline(myfile, instruction);
            }
            tempMemory.address = currentAddress;
            tempMemory.instruction = instruction;
            memory.push_back(tempMemory);
            currentAddress += 4;
        }
    }
    myfile.close();
}
bool beq(string instruction, string& label) {    
    vector<string> result = parseCSV(instruction);
    string r1 = result[1];
    string r2 = result[2];
    int r1int = translate(r1);
    int r2int = translate(r2);
    if (regArray[r1int] == regArray[r2int])
    {
        label = result[3]+':';
        return true;
    }
    return false;
}
bool bne(string instruction, string& label) {
    vector<string> result = parseCSV(instruction);
    string r1 = result[1];
    string r2 = result[2];
    int r1int = translate(r1);
    int r2int = translate(r2);
    if (regArray[r1int] != regArray[r2int])
    {
        label = result[3] + ':';
        return true;
    }
    return false;
}
bool blt(string instruction, string& label) {
    vector<string> result = parseCSV(instruction);
    string r1 = result[1];
    string r2 = result[2];
    int r1int = translate(r1);
    int r2int = translate(r2);
    if (regArray[r1int] < regArray[r2int])
    {
        label = result[3] + ':';
        return true;
    }
    return false;
}
bool bge(string instruction, string& label) {
    vector<string> result = parseCSV(instruction);
    string r1 = result[1];
    string r2 = result[2];
    int r1int = translate(r1);
    int r2int = translate(r2);
    if (regArray[r1int] >= regArray[r2int])
    {
        label = result[3] + ':';
        return true;
    }
    return false;
}
int jalr(string instruction, int* regArray, int nextAddress) {
    //some code to divide the instruction
    //store the register in some string, translate it and get its index
    replace(instruction.begin(), instruction.end(), ',', ' ');
    removeSpaces(instruction, instruction);
    replace(instruction.begin(), instruction.end(), ' ', ',');
    stringstream ss;
    vector<string> result;
    string substr;
    string offset;
    string base_reg;
    stringstream new_ss;
    vector<string> offset_reg;
    int counter = 0;
    ss.str(instruction);
    while (ss.good()) {
        getline(ss, substr, ',');
        result.push_back(substr);
    }
    new_ss.str(result[2]);
    while (new_ss.good()) {
        if (counter == 0) {

            getline(new_ss, substr, '(');
            offset_reg.push_back(substr);
            counter++;
            //cout << "this is position"<<endl;
        }
        else {
            getline(new_ss, substr, ')');
            offset_reg.push_back(substr);
        }
    }
    string rd = result[1];
    int index = translate(rd);
    regArray[index] = nextAddress;

    string rs1 = offset_reg[1];
    int temp= translate(rs1);
    int  goAddress = regArray[temp];
    offset = offset_reg[0];
    int ind = stoi(offset);
    goAddress += ind;
    //cout << "nextAddress: " << nextAddress << endl;
    //cout << "goAddress: " << goAddress << endl;
    return goAddress;
}
string jal(string instruction, int *regArray, int nextAddress ) {
    //some code to divide the instruction
    //store the register in some string, translate it and get its index
    //int index;
    //index = 5; //assumption
    //regArray[index] = nextAddress;
    //string label;
    ////label = instruction.label;
    //label = "L2:";

    vector<string> result = parseCSV(instruction);
    //cout << endl;
    //for (int i = 0; i < result.size(); i++)
    //    cout << result[i] << "\t";
    //cout << endl;

    string r1 = result[1];
    int r1int = translate(r1);
    regArray[r1int] = nextAddress;
    //cout << "address saved: " << nextAddress << endl;
    string label = result[2] + ':';
    return label;

}
int branch(vector<labelObject> labels, string label, int i, int programCounter) {
    for (int j = 0; j < labels.size(); j++)
        if (label == labels[j].label)
        {
            int gotoAddress = labels[j].address;
            int difference = gotoAddress - programCounter;
            i += (difference / 4);
            i--;
            return i;
        }
}
int translate(string reg) {
    if (reg == "x0" || reg == "zero") return 0;
    if (reg == "x1" || reg == "ra") return 1;
    if (reg == "x2" || reg == "sp") return 2;
    if (reg == "x3" || reg == "gp") return 3;
    if (reg == "x4" || reg == "tp") return 4;
    if (reg == "x5" || reg == "t0") return 5;
    if (reg == "x6" || reg == "t1") return 6;
    if (reg == "x7" || reg == "t2") return 7;
    if (reg == "x8" || reg == "s0" || reg == "fp") return 8;
    if (reg == "x9" || reg == "s1") return 9;
    if (reg == "x10" || reg == "a0") return 10;
    if (reg == "x11" || reg == "a1") return 11;
    if (reg == "x12" || reg == "a2") return 12;
    if (reg == "x13" || reg == "a3") return 13;
    if (reg == "x14" || reg == "a4") return 14;
    if (reg == "x15" || reg == "a5") return 15;
    if (reg == "x16" || reg == "a6") return 16;
    if (reg == "x17" || reg == "a7") return 17;
    if (reg == "x18" || reg == "s2") return 18;
    if (reg == "x19" || reg == "s3") return 19;
    if (reg == "x20" || reg == "s4") return 20;
    if (reg == "x21" || reg == "s5") return 21;
    if (reg == "x22" || reg == "s6") return 22;
    if (reg == "x23" || reg == "s7") return 23;
    if (reg == "x24" || reg == "s8") return 24;
    if (reg == "x25" || reg == "s9") return 25;
    if (reg == "x26" || reg == "s10") return 26;
    if (reg == "x27" || reg == "s11") return 27;
    if (reg == "x28" || reg == "t3") return 28;
    if (reg == "x29" || reg == "t4") return 29;
    if (reg == "x30" || reg == "t5") return 30;
    if (reg == "x31" || reg == "t6") return 31;
}
void removeSpaces(string str, string& instruction) {
    string nstr;

    //loop through the characters of the input string
    for (int i = 0; i < str.length(); ) {
        //check if character is white space
        if (str[i] == ' ') {
            /*
              *do not include the white space, if-
              *it is at the trailing or leading position
            */
            if (i == 0 || i == str.length() - 1) {
                i++;
                continue;
            }

            /*
              *if space is inbetween then skip it-
              *except the last occurrence
            */
            while (str[i + 1] == ' ')
                i++;
        }

        //concatenate the character to the new string
        nstr += str[i++];
    }

    instruction = nstr;
}
vector<string> parseCSV(string instruction) {
    replace(instruction.begin(), instruction.end(), ',', ' ');
    removeSpaces(instruction, instruction);
    replace(instruction.begin(), instruction.end(), ' ', ',');
    stringstream ss;
    ss.str(instruction);
    string substr;
    vector<string> result;
    while (ss.good()) {
        getline(ss, substr, ',');
        result.push_back(substr);
    }
    return result;
}
int stToInt(string s) {
	int x;
	if (s.size() == 1)
	{
		if (s[0] >= '0' && s[0] <= '9')
			x = s[0] - '0';
		else
			x = s[0];
	}
	else
	{
		stringstream obj(s);
		obj >> x;
	}
	return x;
}
void find_and_store_byte(vector<dataObject> &data, int address, string bits) {
    dataObject data_element;
    int counter = 0;
    for (int i = 0; i < data.size(); i++) {
        if (data[i].address == address) {
            data[i].bits = bits;
            counter++;
        }
    }
    if (counter == 0) {
        data_element.address = address;
        data_element.bits = bits;
        data.push_back(data_element);
    }
}


void load_or_store(vector<dataObject>& data, int a[], string instruction) {

    dataObject data_element;
    replace(instruction.begin(), instruction.end(), ',', ' ');
    removeSpaces(instruction, instruction);
    replace(instruction.begin(), instruction.end(), ' ', ',');


    stringstream ss;
    vector<string> result;
    vector<string> offset_reg;
    string substr;
    ss.str(instruction);

    while (ss.good()) {

        getline(ss, substr, ',');
        result.push_back(substr);

    }

    stringstream new_ss;
    new_ss.str(result[2]);
    int counter = 0;

    while (new_ss.good()) {
        if (counter == 0) {
            getline(new_ss, substr, '(');
            offset_reg.push_back(substr);
            counter++;
        }
        else {
            getline(new_ss, substr, ')');
            offset_reg.push_back(substr);
        }

    }

    int offset = stoi(offset_reg[0]);
    int base_reg = translate(offset_reg[1]);
    int destination_register = translate(result[1]);

    int address = a[base_reg] + offset;
    //cout << "this is address" << endl << address;
    if (data.empty()) { data_element.address = address; data.push_back(data_element); }

    if (result[0] == "lw") {

        //BEGINNING OF LOAD WORD INSTRUCTION //
        string number = "";
        string value="";
        for (int k = 3; k >= 0; k--) {
            find_and_load(data, address + k, value);
            number.append(value);
        }//end of outer loop


        //cout << endl << "this is the number" << endl << number << endl;
        auto temp = stoul(number, nullptr, 2);
        //cout << "numbero: " << number << endl;
        //cout << "tempo: " << temp << endl;
        int destination_register = translate(result[1]);
        a[destination_register] = temp;
        //cout << a[destination_register] << endl;

    } 	//END OF LOAD WORD INSTRUCTION




    else if (result[0] == "sw") {
        bitset<8> binaryrep;
        string store_bits;
        int temporary = a[destination_register];
        for (int k = 0; k <= 3; k++) {
            binaryrep = temporary;
            store_bits = binaryrep.to_string<char, char_traits<char>, allocator<char> >();
            find_and_store_byte(data, address + k, store_bits);
            temporary = temporary >> 8;
        } //end of for loop
    } // end of store word




    else if (result[0] == "lb" || result[0] == "lbu") {

        string the_byte;

        for (int i = 0; i < data.size(); i++) {

            if (data[i].address == address) { the_byte = data[i].bits; }

        }
        if (the_byte[0] == '1' && result[0] == "lb") the_byte.insert(0, 24, '1');
            auto num = stoul(the_byte, nullptr, 2);
        a[destination_register] = num;

    } //end of load byte signed/unsigned


    else if (result[0] == "lh" || result[0] == "lhu") {

        //BEGINNING OF LOAD HALF WORD INSTRUCTION //
        string number = "";
        string value;
        for (int k = 1; k >= 0; k--) {
            find_and_load(data, address + k, value);
            number.append(value);
        }//end of outer loop
        //cout << "this is number test" << endl << number << endl;

        if (number[0] == '1' && result[0] == "lh") {
            number.insert(0, 16, '1');
        } // end of if negative
        auto temp = stoul(number, nullptr, 2);
        int destination_register = translate(result[1]);
        a[destination_register] = temp;
        //cout << "this is sex" << endl << a[destination_register] << endl;


    } //end of load half word

    else if (result[0] == "sb") {
        string store_bits;
        bitset<8> binaryrep;
        int temporary = a[destination_register];

        binaryrep = temporary;
        for (int k = 0; k < 1; k++) {
            binaryrep = temporary;
            store_bits = binaryrep.to_string<char, char_traits<char>, allocator<char> >();
            find_and_store_byte(data, address + k, store_bits);
            temporary = temporary >> 8;
        } //end of for loop
    } // end of store byte

    else if (result[0] == "sh") {
        bitset<8> binaryrep;
        string store_bits;
        int temporary = a[destination_register];
        for (int k = 0; k <= 1; k++) {
            binaryrep = temporary;
            store_bits = binaryrep.to_string<char, char_traits<char>, allocator<char> >();
            find_and_store_byte(data, address + k, store_bits);
            temporary = temporary >> 8;
        } //end of for loop
    } // end of store word


}

void find_and_load(vector<dataObject> data, int address, string& value) {

    for (int i = 0; i < data.size(); i++) {
        if (data[i].address == address)
        {
            value = data[i].bits;
        }
    }
}
void lui(string instruction, int a[]) {

    replace(instruction.begin(), instruction.end(), ',', ' ');
    removeSpaces(instruction, instruction);
    replace(instruction.begin(), instruction.end(), ' ', ',');

    stringstream ss;
    vector<string> result;
    string substr;
    ss.str(instruction);

    while (ss.good()) {

        getline(ss, substr, ',');
        result.push_back(substr);

    }

    int immediate;
    int index;
    index = translate(result[1]);
    immediate = stoi(result[2]);
    a[index] = immediate;
    a[index] = a[index] << 12;

}

void printAll(vector<blockObject> memory, vector<dataObject> memoryData, int* regArray, int programCounter) {
    cout << "Program:\n";
    cout << "Instructions:\n";
    for (int i=0;i<memory.size();i++)
        cout<< memory[i].address << "\t" << memory[i].instruction << endl;

    cout << endl << endl << "Memory Data:\n";
    for (int i = 0; i < memoryData.size(); i++)
        cout << memoryData[i].address << "\t" << memoryData[i].bits << endl;

    cout << endl << endl << "program counter = " << programCounter << "\n\n\n";

    cout << endl << endl << "register array:\n";
    for (int i = 0; i < 32; i++)
        cout << "reg: " << i << "\t" << regArray[i] << endl;


    cout << endl << endl;
}


void R_I_format(string registers, char format_type)
{
    // string to store individual register operands from the "registers" string
    string temp = "";

    // A counter to indicate the current register operand for which the program is reading its number
    int current_register = 1;

    // A loop to scan the "registers" string
    for (int i = 0; i < registers.size(); i++)
    {
        if (registers[i] != ' ' && registers[i] != ',')
        {
            temp += registers[i];
            //	cout << temp << " ";
        }
        else if (!temp.empty())
        {
            if (current_register == 1)
            {
                rd = translate(temp);
            }
            else if (current_register == 2)
            {
                rs1 = translate(temp);
            }
            else if (current_register == 3 && format_type == 'R')
                rs2 = translate(temp);
            else
            {
                stringstream num(temp);
                num >> rs2;
            }

            temp.clear();
            ++current_register;
        }
    }

    // last iteration if to check for last register operand in the "registers" string
    if (!temp.empty())
    {
        {
            if (current_register == 1)
            {
                rd = translate(temp);
            }
            else if (current_register == 2)
            {
                rs1 = translate(temp);
            }
            else if (current_register == 3 && format_type == 'R')
                rs2 = translate(temp);
            else
            {
                stringstream num(temp);
                num >> rs2;
            }

            temp.clear();
            ++current_register;
        }
    }
}

// processing functions that are of R-format
void R_format_functions(string instruction, int* register_operands)
{
    // reseting variables needed to decompose instruction
    int position = 0;
    string keyword = "";
    string registers = "";

    // storing keyword instruction in "keyword" string (ex. "add")
    while (keyword.empty())
    {
        while (instruction[position] != ' ')
        {
            keyword += instruction[position];
            position++;
        }
        position++;
    }

    // storing registers segment of the assembly instruction in "registers" string
    for (int i = position; i < instruction.size(); i++)
    {
        registers += instruction[i];
    }

    // call R-I-format func. to store registers 
    R_I_format(registers, 'R');

    // Checking the different possible keyword instructions that are in R-format
    if (keyword == "add")
    {
        // perform addition operation
        register_operands[rd] = register_operands[rs1] + register_operands[rs2];
    }
    else if (keyword == "sub")
    {
        //		cout << rd << " " << rs1 << " " << rs2 << endl;
        register_operands[rd] = register_operands[rs1] - register_operands[rs2];
    }
    else if (keyword == "sll")
    {
        register_operands[rd] = register_operands[rs1] << register_operands[rs2];
    }
    else if (keyword == "slt")
    {
        if (register_operands[rs1] < register_operands[rs2])
            register_operands[rd] = 1;
        else
            register_operands[rd] = 0;
    }
    else if (keyword == "sltu")
    {
        unsigned int x = register_operands[rs1];
        unsigned int y = register_operands[rs2];
        //	cout << x << " " << y << " " << endl;
        if (x < y)
            register_operands[rd] = 1;
        else
            register_operands[rd] = 0;
    }
    else if (keyword == "xor")
    {
        register_operands[rd] = register_operands[rs1] ^ register_operands[rs2];
    }
    else if (keyword == "srl" || keyword == "sra")
    {
        register_operands[rd] = register_operands[rs1] >> register_operands[rs2];
    }
    else if (keyword == "or")
    {
        register_operands[rd] = register_operands[rs1] | register_operands[rs2];
    }
    else if (keyword == "and")
    {
        register_operands[rd] = register_operands[rs1] & register_operands[rs2];
    }
}

// processing functions that are of I-format (Constants, not offsets)
void I_format_constants_functions(string instruction, int* register_operands)
{
    // reseting variables needed to decompose instruction
    int position = 0;
    string keyword = "";
    string registers = "";

    // storing keyword instruction in "keyword" string (ex. "add")
    while (keyword.empty())
    {
        while (instruction[position] != ' ')
        {
            keyword += instruction[position];
            position++;
        }
        position++;
    }

    // storing registers segment of the assembly instruction in "registers" string
    for (int i = position; i < instruction.size(); i++)
    {
        registers += instruction[i];
    }

    // call R-I-format func. to store registers 
    R_I_format(registers, 'I');

    // Checking the different possible keyword instructions that are in I-format (constants)
    if (keyword == "addi")
    {
        // perform addition operation
        register_operands[rd] = register_operands[rs1] + rs2;
    }
    else if (keyword == "slti")
    {
        if (register_operands[rs1] < rs2)
            register_operands[rd] = 1;
        else
            register_operands[rd] = 0;
    }
    else if (keyword == "sltiu")
    {
        unsigned int x = register_operands[rs1];
        unsigned int y = rs2;
        if (x < y)
            register_operands[rd] = 1;
        else
            register_operands[rd] = 0;
    }
    else if (keyword == "xori")
    {
        register_operands[rd] = register_operands[rs1] ^ rs2;
    }
    else if (keyword == "ori")
    {
        register_operands[rd] = register_operands[rs1] | rs2;
    }
    else if (keyword == "andi")
    {
        register_operands[rd] = register_operands[rs1] & rs2;
    }
    else if (keyword == "slli")
    {
        register_operands[rd] = register_operands[rs1] << rs2;
    }
    else if (keyword == "srli" || keyword == "srai")
    {
        register_operands[rd] = register_operands[rs1] >> rs2;
    }
}


/*
fact:
addi sp, sp, -8
sw ra, 4(sp)
sw a0, 0(sp)
slti t0, a0, 1
beq t0, zero, L1
addi a0, zero, 1
addi sp, sp, 8
jalr zero, 0(ra)
L1: addi a0, a0, -1
jal ra, fact
addi t0, a0, 0
lw a0, 0(sp)
lw ra, 4(sp)
addi sp, sp, 8
add a0, a0, t0
jalr zero, 0(ra)
*/