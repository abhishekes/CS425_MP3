#!/bin/bash

echo "30 MB and scarce pattern" 
echo "30 MB and scarce pattern\n" &> 30MB_results_SCARCE_MACHINE.txt
echo "Reading 1\n" &> 30MB_results_SCARCE_MACHINE.txt 
time ./dgrep_client SCARCE_MACHINE &> 30MB_results_SCARCE_MACHINE_1.txt
echo "Reading 2\n" &> 30MB_results_SCARCE_MACHINE.txt 
time ./dgrep_client SCARCE_MACHINE &> 30MB_results_SCARCE_MACHINE_2.txt
echo "Reading 3\n" &> 30MB_results_SCARCE_MACHINE.txt 
time ./dgrep_client SCARCE_MACHINE &> 30MB_results_SCARCE_MACHINE_3.txt
echo "Reading 4\n" &> 30MB_results_SCARCE_MACHINE.txt  
time ./dgrep_client SCARCE_MACHINE &> 30MB_results_SCARCE_MACHINE_4.txt
echo "Reading 5\n" &> 30MB_results_SCARCE_MACHINE.txt 
time ./dgrep_client SCARCE_MACHINE &> 30MB_results_SCARCE_MACHINE_5.txt

echo "30 MB and frequent pattern"
echo "30 MB and frequent pattern\n" &> 30MB_results_FREQUENT.txt
echo "Reading 1\n" &> 30MB_results_FREQUENT.txt
time ./dgrep_client FREQUENT &> 30MB_results_FREQUENT_1.txt
echo "Reading 2\n" &> 30MB_results_FREQUENT.txt
time ./dgrep_client FREQUENT &> 30MB_results_FREQUENT_2.txt
echo "Reading 3\n" &> 30MB_results_FREQUENT.txt
time ./dgrep_client FREQUENT &> 30MB_results_FREQUENT_3.txt
echo "Reading 4\n" &> 30MB_results_FREQUENT.txt
time ./dgrep_client FREQUENT &> 30MB_results_FREQUENT_4.txt
echo "Reading 5\n" &> 30MB_results_FREQUENT.txt
time ./dgrep_client FREQUENT &> 30MB_results_FREQUENT_5.txt


echo "30 MB and regex pattern"
echo "30 MB and frequent pattern\n" &> 30MB_results_REGEX.txt
echo "Reading 1\n" &> 30MB_results_FREQUENT.txt
time ./dgrep_client "FREQUENT_*MAC*_*" &> 30MB_results_REGEX_1.txt
echo "Reading 2\n" &> 30MB_results_FREQUENT.txt
time ./dgrep_client "FREQUENT_*MAC*_*" &> 30MB_results_REGEX_2.txt
echo "Reading 3\n" &> 30MB_results_FREQUENT.txt
time ./dgrep_client "FREQUENT_*MAC*_*" &> 30MB_results_REGEX_3.txt
echo "Reading 4\n" &> 30MB_results_FREQUENT.txt
time ./dgrep_client "FREQUENT_*MAC*_*" &> 30MB_results_REGEX_4.txt
echo "Reading 5\n" &> 30MB_results_FREQUENT.txt
time ./dgrep_client "FREQUENT_*MAC*_*" &> 30MB_results_REGEX_5.txt

echo "30 MB and scarce pattern" 
echo "30 MB and scarce pattern\n" &> 30MB_results_SCARCE_MACHINE.txt
echo "Reading 1\n" &> 30MB_results_SCARCE_MACHINE.txt
time ./dgrep_client SCARCE_MACHINE &> 30MB_results_SCARCE_MACHINE.txt
echo "Reading 2\n" &> 30MB_results_SCARCE_MACHINE.txt
time ./dgrep_client SCARCE_MACHINE &> 30MB_results_SCARCE_MACHINE.txt
echo "Reading 3\n" &> 30MB_results_SCARCE_MACHINE.txt
time ./dgrep_client SCARCE_MACHINE &> 30MB_results_SCARCE_MACHINE.txt
echo "Reading 4\n" &> 30MB_results_SCARCE_MACHINE.txt
time ./dgrep_client SCARCE_MACHINE &> 30MB_results_SCARCE_MACHINE.txt
echo "Reading 5\n" &> 30MB_results_SCARCE_MACHINE.txt
time ./dgrep_client SCARCE_MACHINE &> 30MB_results_SCARCE_MACHINE.txt

echo "30 MB and frequent pattern"
echo "30 MB and frequent pattern\n" &> 30MB_results_FREQUENT.txt
echo "Reading 1\n" &> 30MB_results_FREQUENT.txt
time ./dgrep_client FREQUENT &> 30MB_results_FREQUENT.txt
echo "Reading 2\n" &> 30MB_results_FREQUENT.txt
time ./dgrep_client FREQUENT &> 30MB_results_FREQUENT.txt
echo "Reading 3\n" &> 30MB_results_FREQUENT.txt
time ./dgrep_client FREQUENT &> 30MB_results_FREQUENT.txt
echo "Reading 4\n" &> 30MB_results_FREQUENT.txt
time ./dgrep_client FREQUENT &> 30MB_results_FREQUENT.txt
echo "Reading 5\n" &> 30MB_results_FREQUENT.txt
time ./dgrep_client FREQUENT &> 30MB_results_FREQUENT.txt

