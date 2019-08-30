g++ -std=c++11 main.cpp -o test
./test > test_results_new.txt
diff -s -y -Z --color=always test_results_new.txt test_results_org.txt