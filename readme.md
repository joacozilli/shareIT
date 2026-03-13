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
When node is executed, generates id and sends broadcast name_request. If inside the next 5 seconds an invalid name is received,
repeat. Otherwise, node starts. It must perodically send a HELLO message to local broadcast with its name and ip and port
where it is awaiting requests.

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
    - x wants to download files F1,...,Fm from n_i
    - x creates new tcp socket and connects to n_i's listening tcp socket (ip and port known)
    - for j=1...m
        - x sends download request to n_i for Fj and awaits answer from n_i.
        - if Fj doesn't exist, n_i informs it and continue to next file.
        - if Fj exist, n_i informs it, rearm x's fd to FILE_TRANSFER type
          and starts to send chunk by chunk.
        - if file transfer finished, n_i rearm x's fd to a normal SOCKET_TCP_CLIENT.
          If j < m, continue, else x informs task completed and closes connection.

download request message structure:
    - DOWNLOAD_REQUEST [FILENAME]

if file doesn't exist, n_i answer structure:
    - NOT_FOUND [FILENAME]
if it exists:
    - FOUND [FILENAME]

