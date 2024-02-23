/*
mipsSimulator by Annika Damstedt, W24
Notes:
    *I couldn't figure out how to do negative integers using uint32_t
    (which is what was recommeneded to use in c++) or deal with overflow
    *There's a "%" at the end of my output when it doesn't end with a newline
    *Got jumps and strings working, but it doesn't feel like the way MIPS
    did it...
    *Couldn't figure out how to implement very hard I type instructions in
    the time I had left (also I've only ever used lw and sw)
*/

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>

using std::cout;
using std::endl;
using std::cin;

// R type instructions
void add(uint32_t &rs, uint32_t &rt, uint32_t &rd);
void addu(uint32_t &rs, uint32_t &rt, uint32_t &rd);
void and_(uint32_t &rs, uint32_t &rt, uint32_t &rd); 
void nor(uint32_t &rs, uint32_t &rt, uint32_t &rd);
void or_(uint32_t &rs, uint32_t &rt, uint32_t &rd);  
void slt(uint32_t &rs, uint32_t &rt, uint32_t &rd);
void sltu(uint32_t &rs, uint32_t &rt, uint32_t &rd);
void sll(uint32_t &shamt, uint32_t &rt, uint32_t &rd);
void srl(uint32_t &shamt, uint32_t &rt, uint32_t &rd);
void sub(uint32_t &rs, uint32_t &rt, uint32_t &rd);
void subu(uint32_t &rs, uint32_t &rt, uint32_t &rd);

// I type instructions
void addi(uint32_t &rs, uint32_t &rt, uint32_t &immediate);
void addiu(uint32_t &rs, uint32_t &rt, uint32_t &immediate);
void andi(uint32_t &rs, uint32_t &rt, uint32_t &immediate);
void lui(uint32_t &rt, uint32_t &immediate);
void ori(uint32_t &rs, uint32_t &rt, uint32_t &immediate);
void slti(uint32_t &rs, uint32_t &rt, uint32_t &immediate);
void sltiu(uint32_t &rs, uint32_t &rt, uint32_t &immediate);

// Very hard I type
void lw();
void sw();
void lhu();
void sh();
void lbu();
void sb();

int main() {
    // Variables
    uint32_t RF [32];
    uint32_t PC = -4;
    uint32_t currInstruction = 0x00000000;
    uint32_t currOp = 0x00000000;
    uint32_t currFunct = 0x00000000;
    uint32_t rs = 0x00000000;
    uint32_t rt = 0x00000000;
    uint32_t rd = 0x00000000;
    uint32_t shamt = 0x00000000;
    uint32_t immediate = 0x00000000;
    uint32_t address = 0x00000000;
    bool quit = false;
    bool isInstruction = false;
    uint32_t position [100]; // Limit number of labels to 100
    int posIndex = 0;
    int fileLength = 0;
    std::string strings [100]; // Limit number of strings to 100
    int strIndex = 0;
    bool text = false;

    RF[0] = 0x00000000;

    // Fix hexcode with cin
    std::cin.unsetf(std::ios::dec);
    std::cin.unsetf(std::ios::hex);
    std::cin.unsetf(std::ios::oct); 

    // Setting up reading from a file
    std::ifstream mipsFile ("mipsSimulator.txt");
    std::string currLine;
    int commentIndex;

    // Get file length
    mipsFile.seekg(0, mipsFile.end);
    fileLength = mipsFile.tellg();
    mipsFile.seekg(0, mipsFile.beg);
        
    while (mipsFile.tellg() < fileLength) {
        std::getline(mipsFile, currLine);

        // Ignore comments
        commentIndex = currLine.find("#");
        if (commentIndex != -1) {
            currLine = currLine.substr(0, commentIndex);
        }
        
        if (currLine.find(".text") != -1) {
            text = true;
        }
        if (text == false) {
            // Get data
            if (currLine.find(":") != -1) {
                if (currLine.find(".asciiz") != -1) {
                    strings[strIndex] = currLine.substr(currLine.find("\"") + 1, currLine.length() - currLine.find("\"") - 2);
                    if (strings[strIndex].find("\\n")) {
                        strings[strIndex].replace(strings[strIndex].find("\\n"), 2, "\n");
                    }
                    strIndex++;
                }
            }
        } else {
            // Remove whitespace
            currLine.erase(std::remove_if(currLine.begin(), currLine.end(), ::isspace), currLine.end());
            // Get labels
            if (currLine.find(":") != -1) {
                position[posIndex] = mipsFile.tellg();
                posIndex++;
            }
        }
    }
    mipsFile.seekg(0, mipsFile.beg);


    while (quit == false && mipsFile.good()) {        
        std::getline(mipsFile, currLine);

        // Ignore comments
        commentIndex = currLine.find("#");
        if (commentIndex != -1) {
            currLine = currLine.substr(0, commentIndex);
        }
        // Remove whitespace
        currLine.erase(std::remove_if(currLine.begin(), currLine.end(), ::isspace), currLine.end());
        // Get hex
        if (currLine.substr(0, 2) == "0x") {
            currInstruction = static_cast<std::uint32_t>(std::stoul(currLine, nullptr, 16));
            isInstruction = true;
        } else {
            isInstruction = false;
        }

        if (currLine != "") {
            // cout << "Current line: " << currLine << endl;
        }

        if (isInstruction == true) {
            PC += 4;

            // Check syscall
            if (currInstruction == 0x0000000C) {
                if (RF[2] == 1) {
                    // Print int
                    cout << RF[4];
                }

                if (RF[2] == 4) {
                    // Print string
                    if (RF[4] <= strIndex) {
                        cout << strings[RF[4]];
                    }
                }

                if (RF[2] == 5) {
                    // Read int
                    cin >> RF[2];
                }

                if (RF[2] == 8) {
                    // Read string 
                    cin >> strings[strIndex];
                    RF[2] = strIndex;
                    strIndex++;
                }

                if (RF[2] == 10) {
                    // Quit!
                    quit = true;
                }
            } else {
                // Get opcode
                currOp = currInstruction >> 26;
                // cout << "   Opcode: " << currOp << endl;

                if (currOp != 0x2 && currOp != 0x3) {
                    // Get rs
                    rs = (currInstruction << 6) >> 27; 
                    // cout << "   rs: " << rs << endl;

                    // Get rt
                    rt = (currInstruction << 11) >> 27;
                    // cout << "   rt: " << rt << endl;

                    if (currOp == 0x0) {
                        // cout << "   R type instruction!" << endl;

                        // Get rd
                        rd = (currInstruction << 16) >> 27;
                        // cout << "   rd: " << rd << endl;

                        // Get shamt
                        shamt = (currInstruction << 21) >> 27;
                        // cout << "   shamt: " << shamt << endl;

                        // Get function
                        currFunct = (currInstruction << 26) >> 26;
                        // cout << "   funct: " << currFunct << endl; 
                    } else {
                        // cout << "   I type instruction!" << endl;

                        // Get immediate
                        immediate = (currInstruction << 16) >> 16; // Second number not right
                        // cout << "   Immediate: " << immediate << endl;
                    }
                } else {
                    // cout << "   J type instruction!" << endl;

                    // Get address
                    address = (currInstruction << 6) >> 6;
                    // cout << "   address: " << address << endl;
                }

                // ----------------------CHECK INSTRUCTIONS----------------------

                // R type
                if (currOp == 0x0) {
                    // Check for add
                    if (currFunct == 0x20) {
                        add (RF[rs], RF[rt], RF[rd]);
                        // cout << "   add result: " << RF[rd] << endl;
                    }

                    // Check for addu
                    if (currFunct == 0x21) {
                        addu(RF[rs], RF[rt], RF[rd]);
                        // cout << "   addu result: " << RF[rd] << endl;
                    }

                    // Check for and
                    if (currFunct == 0x24) {
                        and_(RF[rs], RF[rt], RF[rd]);
                        // cout << "   and result: " << RF[rd] << endl;
                    }

                    // Check for nor
                    if (currFunct == 0x27) {
                        nor(RF[rs], RF[rt], RF[rd]);
                        // cout << "   nor result: " << RF[rd] << endl;
                    }

                    // Check for or
                    if (currFunct == 0x25) {
                        or_(RF[rs], RF[rt], RF[rd]);
                        // cout << "   or result: " << RF[rd] << endl;
                    }

                    // Check for slt
                    if (currFunct == 0x2A) {
                        slt(RF[rs], RF[rt], RF[rd]);
                        // cout << "   slt result: " << RF[rd] << endl;
                    }

                    // Check for sltu
                    if (currFunct == 0x2B) {
                        sltu(RF[rs], RF[rt], RF[rd]);
                        // cout << "   sltu result: " << RF[rd] << endl;
                    }

                    // Check for sll
                    if (currFunct == 0x00) {
                        sll(shamt, RF[rt], RF[rd]);
                        // cout << "   sll result: " << RF[rd] << endl;
                    }

                    // Check for srl
                    if (currFunct == 0x02) {
                        srl(shamt, RF[rt], RF[rd]);
                        // cout << "   srl result: " << RF[rd] << endl;
                    }

                    // Check for sub
                    if (currFunct == 0x22) {
                        sub(RF[rs], RF[rt], RF[rd]);
                        // cout << "   sub result: " << RF[rd] << endl;
                    }

                    // Check for subu
                    if (currFunct == 0x23) {
                        subu(RF[rs], RF[rt], RF[rd]);
                        // cout << "   subu result: " << RF[rd] << endl;
                    }

                    // Check for jr
                    if (currFunct == 0x08) {
                        // cout << "jr jump successful" << endl;
                        mipsFile.seekg(RF[rs]);
                    }
                }
                
                // I type
                // Check for addi
                if (currOp == 0x8) {
                    addi(RF[rs], RF[rt], immediate);
                    // cout << "   addi result: " << RF[rt] << endl;
                }

                // Check for addiu
                if (currOp == 0x9) {
                    addiu(RF[rs], RF[rt], immediate);
                    // cout << "   addiu result: " << RF[rt] << endl;
                }

                // Check for andi
                if (currOp == 0xC) {
                    andi(RF[rs], RF[rt], immediate);
                    // cout << "   andi result: " << RF[rt] << endl;
                }

                // Check for lui
                if (currOp == 0xF) {
                    lui( RF[rt], immediate);
                    // cout << "   lui result: " << RF[rt] << endl;
                }

                // Check for ori
                if (currOp == 0xD) {
                    ori(RF[rs], RF[rt], immediate);
                    // cout << "   ori result: " << RF[rt] << endl;
                }

                // Check for slti
                if (currOp == 0xA) {
                    slti(RF[rs], RF[rt], immediate);
                    // cout << "   slti result: " << RF[rt] << endl;
                }

                // Check for sltiu
                if (currOp == 0xB) {
                    sltiu(RF[rs], RF[rt], immediate);
                    // cout << "   sltiu result: " << RF[rt] << endl;
                }

                // Check for beq
                if (currOp == 0x4) {
                    if (RF[rs] == RF[rt]) {
                        // cout << "beq jump successful" << endl;
                        // cout << "   beq result: branch" << endl;
                        mipsFile.seekg(position[immediate]);
                    }
                    // cout << "   beq result: no branch" << endl;
                }

                // Check for bne
                if (currOp == 0x5) {
                    if (RF[rs] != RF[rt]) {
                        // cout << "bne jump successful" << endl;
                        // cout << "   bne result: branch" << endl;
                        mipsFile.seekg(position[immediate]);
                    }
                    // cout << "   bne result: no branch" << endl;
                }

                // J type
                // Check for j
                if (currOp == 0x2) {
                    // cout << "j jump successful" << endl;
                    mipsFile.seekg(position[address]);
                }

                // Check for jal
                if (currOp == 0x3) {
                    // cout << "jal jump successful" << endl;
                    RF[31] = mipsFile.tellg();
                    mipsFile.seekg(position[address]);
                }
            }
        }
    }
}

// ----------------R type----------------
void add(uint32_t &rs, uint32_t &rt, uint32_t &rd) {
    // I don't think signed ints work properly, but I couldn't figure out how to get it to work
    rd = rs + rt;
    return;
}

void addu(uint32_t &rs, uint32_t &rt, uint32_t &rd) {
    rd = rs + rt;
    return;
}

void and_(uint32_t &rs, uint32_t &rt, uint32_t &rd) {
    rd = rs & rt;
    return;
}

void nor(uint32_t &rs, uint32_t &rt, uint32_t &rd) {
    rd = rs | rt;
    rd = ~rd;
    return;
}

void or_(uint32_t &rs, uint32_t &rt, uint32_t &rd) {
    rd = rs | rt;
    return;
}  

void slt(uint32_t &rs, uint32_t &rt, uint32_t &rd) {
    if (rs < rt) {
        rd = 1;
    } else {
        rd = 0;
    }
    return;
}

void sltu(uint32_t &rs, uint32_t &rt, uint32_t &rd) {
    if (rs < rt) {
        rd = 1;
    } else {
        rd = 0;
    }
    return;
}

void sll(uint32_t &shamt, uint32_t &rt, uint32_t &rd) {   
    rd = rt << shamt;
    return;
}

void srl(uint32_t &shamt, uint32_t &rt, uint32_t &rd) {
    rd = rt >> shamt;
    return;
}

void sub(uint32_t &rs, uint32_t &rt, uint32_t &rd) {
    rd = rs - rt;
    return;
}

void subu(uint32_t &rs, uint32_t &rt, uint32_t &rd) {
    rd = rs - rt;
    return;
}


// ----------------I type----------------
void addi(uint32_t &rs, uint32_t &rt, uint32_t &immediate) {
    rt = rs + immediate;
    return;
}

void addiu(uint32_t &rs, uint32_t &rt, uint32_t &immediate) {
    rt = rs + immediate;
    return;
}

void andi(uint32_t &rs, uint32_t &rt, uint32_t &immediate) {
    rt = rs & immediate;
    return;
}

void lui(uint32_t &rt, uint32_t &immediate) {
    rt = immediate << 16;
    return;
}

void ori(uint32_t &rs, uint32_t &rt, uint32_t &immediate) {
    rt = rs | immediate;
    return;
}

void slti(uint32_t &rs, uint32_t &rt, uint32_t &immediate) {
    if (rs < immediate) {
        rt = 1;
    } else {
        rt = 0;
    }
    return;
}

void sltiu(uint32_t &rs, uint32_t &rt, uint32_t &immediate) {
    if (rs < immediate) {
        rt = 1;
    } else {
        rt = 0;
    }
    return;
}

