## Router Path Calculator - Dijkstra algorithm

#### for Dijkstra and Dijkstra NlogN:
1. Installing Python 3: Make sure Python 3 is installed on your system.

2. Run the Python script from the command line `./Dijkstra` and `./DijkstraNlogN`

3. In your terminal, enter the Network Topology Information following by this format:
    `X
    Y
    Z
    LINKSTATE
    X-Z 1 X,Y
    X-Y 5 
    Y-Z 3 X,Z
    UPDATE
    X-Z -1 X,Y
    Y-Z 9 Y,Z
    END`

    3.1 Node information: each line represents the name of a node until LINKSTATE is encountered to start reading link status information.

    3.2 Link status information: each line represents the connection relationship between a node and its weight until UPDATE is encountered to start reading update information.

    3.3 Update information: each line represents the update of link status until END is encountered.

    OR

    Using input redirection:
        command `./Dijkstra < test_case.txt`
        The content of "test_case.txt" can be verified by users

4. The script outputs the node's neighbor table, link-state database (LSDB), and routing table

#### for the script for Dijkstra:
`test_dijkstra_performance` is the script for testing dijkstra algorithms' performance. 
`test_case1.txt`, `test_case2.txt`, `test_case3.txt`, `test_case4.txt` are the test cases for testing the performance

1. Installing Python 3: Make sure Python 3 is installed on your system.

2. run the command `python3 test_dijkstra_performance.py` in your terminal.

3. The script will automatically show the result of the # of nodes, # of links and execution time.
