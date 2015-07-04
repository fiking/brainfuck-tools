#include <fstream>
#include <iostream>
#include <regex>
#include "gmock/gmock.h"
#include "BrainfuckVM.h"

void Debugger(BrainfuckVM &vm, std::string program);

void UsageAndExit();

void PrintVMState(BrainfuckVM &vm);

int main(int argc, char **argv) {
/*
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();*/

    if (argc != 3 && argc != 4) {
        UsageAndExit();
    }

    std::ifstream program_file(argv[2]);
    if (!program_file) {
        std::cout << "Unknown program file " << argv[1];
        exit(EXIT_FAILURE);
    }

    std::string program;
    program_file.seekg(0, std::ios::end);
    program.resize(program_file.tellg());
    program_file.seekg(0, std::ios::beg);
    program_file.read(&program[0], program.size());
    program_file.close();

    BrainfuckVM vm;
    std::string mode = argv[1];
    if (mode == "-i") {
        vm.Interpret(program);
    } else if (mode == "-d") {
        Debugger(vm, program);
    } else if (mode == "-j") {
        vm.JIT(program);
    } else {
        UsageAndExit();
    }
}

void UsageAndExit() {
    std::cout << "USAGE:" << std::endl
    << "BrainfuckVM { -i | -d | -j } program.bf [inputs.txt]" << std::endl;
    exit(EXIT_SUCCESS);
}

void Debugger(BrainfuckVM &vm, std::string program) {
    std::cout << "Brainfuck Debugger" << std::endl <<
    "'exit' to exit" << std::endl << std::endl;

    vm.Start(program);
    std::string debug_input;

    while (!vm.ProgramIsFinished()) {
        PrintVMState(vm);
        std::cout << "> ";
        std::cin >> debug_input;

        if (std::cin.eof()) {
            debug_input = "r";
            std::cout << "r";
        }

        std::regex pattern_step(R"([sS](\d+)?)");
        std::regex pattern_go(R"([gG](-)?(\d+))");
        std::regex pattern_dump(R"([dD](\d+)(-(\d+))?)");
        std::smatch matches;

        if (std::regex_search(debug_input, matches, pattern_step)) {
            if (matches.size() > 1 && matches[1].matched)
                vm.Step(static_cast<unsigned>(std::stoi(matches[1])));
            else
                vm.Step(1);
        } else if (debug_input == "r" || debug_input == "R") {
            vm.GoUntil(0, true);
        } else if (std::regex_search(debug_input, matches, pattern_go)) {
            unsigned target_ip = 0;
            if (matches.size() > 2 && matches[2].matched) {
                target_ip = static_cast<unsigned>(std::stoi(matches[2]));
                if (matches[1].matched) {
                    vm.GoUntil(target_ip, true);
                } else {
                    vm.GoUntil(target_ip);
                }
            }
        } else if (std::regex_search(debug_input, matches, pattern_dump)) {
            if (matches[1].matched) {
                if (matches[2].matched && matches[3].matched) {
                    // dump array between these positions
                    unsigned start = static_cast<unsigned>(std::stoi(matches[1]));
                    unsigned end = static_cast<unsigned>(std::stoi(matches[3]));
                    for (auto i = start; i <= end; i++) {
                        std::cout << (int) vm.GetMemory().at(i) << " ";
                    }
                    std::cout << std::endl;
                } else {
                    // dump array at given position
                    std::cout << (int) vm.GetMemory().at(static_cast<unsigned>(std::stoi(matches[1]))) << std::endl;
                }

                std::cout << std::endl;
            }
        } else if (debug_input == "exit") {
            exit(EXIT_SUCCESS);
        }
    }
}

void PrintVMState(BrainfuckVM &vm) {
    std::cout << "IP = " << vm.GetInstrPointer() << "\t";
    std::cout << "DP = " << vm.GetDataPointer() << std::endl;
    vm.PrintFormattedLocation();
    std::cout << "MEM:\t[";
    std::vector<char> mem = vm.GetMemory();
    mem.resize(10);
    for (int i: mem) {
        std::cout << i << ", ";
    }

    std::cout << "...]" << std::endl;
}