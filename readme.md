# shareIT

shareIT is a simple file sharing system for a local network. It uses a completely decentralized P2P architecture,
making each node fully independent and the system highly scalable. It facilitates the task of transfering files between
computers connected to the same network.

# instalation guide
To install, you can simply clone the repository:

`git clone git@github.com:joacozilli/shareIT.git`

It is required to have `gcc` installed.

# usage guide

To use the program, first it is necessary to create a file named 'config.json' inside the program directory
(but outside 'include' and 'src' directories) where all the required information to run a node is specified.
This json file must have the following structure:
```
{
    "srv_name" : "your-node-name",
    "srv_port" : 00000,
    "srv_ip" : "your.node.ip",
    "broadcast_port" : 00000,
    "broadcast_ip" : "your.local.network.broadcast.ip"
}
```

Each node in the system must have a unique name with no spaces in-between (currently, the program DOES NOT check for this).
`srv_port` and `srv_ip` form the address of the node in the network; other peers will connect to this address
to send requests.

`broadcast_port` and `broadcast_ip`, as the names suggest, form the address for broadcasting messages
in the local network. It is used by the discovering mechanism to find and acknowlege other peers.
This address must be the same for all nodes in the same system.

Currently, the files desired to be shared by the node must be in the folder `share`, (subdirectories are not supported for now)

To compile the program, simply run:
`make build`

Once compile, then run the program with:
`./shareIT`

If node executed correctly, the cli will start. The following commands are supported:

- `neighbors` show all the known peers at the moment
- `peek <PEER_NAME>` see all files shared by peer PEER_NAME
- `download <FILE_NAME> <PEER_NAME>` download file FILE_NAME from peer PEER_NAME