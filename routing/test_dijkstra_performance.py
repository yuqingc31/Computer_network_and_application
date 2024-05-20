import subprocess
import time

# read test case file
def load_test_data(file_name):
    with open(file_name, 'rb') as file:
        input_data = file.read()
    return input_data

# calculate the execution time of the script
def dijkstra_execution_time(script_name, input_data):
    start_time = time.time()
    
    subprocess.run(["python3", script_name], input=input_data, text=False, capture_output=True)
    
    end_time = time.time()

    execution_time = end_time - start_time

    return execution_time

# load test case file
test_case1 = load_test_data("test_case1.txt")
test_case2 = load_test_data("test_case2.txt")
test_case3 = load_test_data("test_case3.txt")
test_case4 = load_test_data("test_case4.txt")

case1_time = dijkstra_execution_time("Dijkstra", test_case1)
case2_time = dijkstra_execution_time("Dijkstra", test_case2)
case3_time = dijkstra_execution_time("Dijkstra", test_case3)
case4_time = dijkstra_execution_time("Dijkstra", test_case4)
print("For Dijkstra with N * N complexity")
print("Number of Nodes     Number of Links     Execution Time (N*N) ")
print("4                   4                   ", case1_time)
print("20                  36                  ", case2_time)
print("30                  33                  ", case3_time)
print("40                  38                  ", case4_time)

case1_nlogn_time = dijkstra_execution_time("DijkstraNlogN", test_case1)
case2_nlogn_time = dijkstra_execution_time("DijkstraNlogN", test_case2)
case3_nlogn_time = dijkstra_execution_time("DijkstraNlogN", test_case3)
case4_nlogn_time = dijkstra_execution_time("DijkstraNlogN", test_case4)
print("For Dijkstra with N log N complexity")
print("Number of Nodes     Number of Links     Execution Time (N log N)")
print("4                   4                   ", case1_nlogn_time)
print("20                  36                  ", case2_nlogn_time)
print("30                  33                  ", case3_nlogn_time)
print("40                  38                  ", case4_nlogn_time)