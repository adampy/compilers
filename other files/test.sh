#!/bin/bash

readonly OUTPUT_FILE="lexerTokenOutput.txt";

gcc -g lexer.c -o lexer;
if [[ $? -eq 0 ]]; then
    printf "\033[36mBuild\033[0m: \033[32mPASSED\033[0m\n";
    successful_tests+=1;
else 
    printf "\033[36mBuild\033[0m: \033[31mFAILED\033[0m\n";
fi

echo -e "\033[33m----- STANDARD TESTING -----\033[0m";
total_tests=0;
successful_tests=0;

# Find all jack files, run them, and use diff
for i in *.jack; do
    if [[ $i =~ ^(Custom) ]]; then
        continue;
    fi

    printf "\033[36m$i\033[0m: ";
    ./lexer $i && diff ${OUTPUT_FILE} ${i}_tokens.txt;
    if [[ $? -eq 0 ]]; then
        printf "\033[32mPASSED\033[0m\n";
        ((successful_tests++));
    else 
        printf "\033[31mFAILED\033[0m\n";
    fi
    ((total_tests++));
done

echo -e "\033[33m----- CUSTOM TESTING -----\033[0m";
for i in Custom*.jack; do
    printf "\033[36m$i\033[0m: ";
    ./lexer $i && diff ${OUTPUT_FILE} ${i}_tokens.txt;
    if [[ $? -eq 0 ]]; then
        printf "\033[32mPASSED\033[0m\n";
        ((successful_tests++));
    else 
        printf "\033[31mFAILED\033[0m\n";
    fi
    ((total_tests++));
done

printf "\n\033[33m----- ${successful_tests}/${total_tests} TESTS PASSED -----\033[0m\n";
