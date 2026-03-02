# shareIT


# instalation guide


# usage guide

# Idea
File sharing system for local network with a peer-to-peer system, where each node is completely independent.
The idea is to facilitate the task of sharing files from one computer to others in the same network.

Each node must:
    - announce its existence in the network (with a broadcast).
    - be able to recognize other nodes.
    - offer a set of files to the other nodes.
    - be able to download specific files from other nodes.
    - be able to send a file to all the other nodes.
    - recieve files other nodes send to it.
    - all this tasks must be done concurrently



# Communication Protocol


# descovering mechanism
When a node is executed, it starts to perodically send a HELLO message to local broadcast with its name and ip and port where it is
awaiting requests. To allow the execution of several nodes in the same machine, a unique identifier will be the pair (ip, port).


Simultaneously, when the node receives a hello message from other node, it confirms that such node still exists or adds it to its
set of known nodes if it is a new one (saving port and ip).

The node offers a set of files, which other nodes can see and request downloads. They must be handled concurrently.
At the same time, it can see known-nodes's set of files and request downloads to them.
Also, the node can send a file to all known nodes, and all this nodes must receive it.


Let N = {n_1,...,n_k} be the set of known nodes by node x. Hello messages are sent every 2 seconds (for example). If x hasn't
received a hello message from n_i in the last 6 seconds (3 hello's tolerance), x forgets about it. To achieve this:
    - Store N in a table were each node has a counter.
    - Every 2 seconds, iterate table and add 1 to counter (use timefd_create with epoll).
    - If counter reaches 3, forget node.
    - If hello message received, reset node's counter to 0.

Suppose we store N in an avl tree
- linear mapping (add 1 to all counters) and iteration
- counter reset/add new node in lg(n)
- forget node (delete from avl) in lg(n)


download request:
    - x wants to download file F from n_i
    - x creates new tcp socket and connects to n_i's listening tcp socket (ip and port known)
    - x sends download request to n_i
