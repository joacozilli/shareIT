#include "cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "network.h"
#include "server.h"
#include "utils.h"
#include "peer.h"
#include "str.h"

/**
 * Auxiliar function to map each command to a CMD type.
 */
CMD command_mapping(char* str) {
    if (!strcmp(str, "help"))
        return HELP;
    if (!strcmp(str, "peek"))
        return PEEK;
    if (!strcmp(str, "search"))
        return SEARCH;
    if (!strcmp(str, "download"))
        return DOWNLOAD;
    
    return UNDEFINED;
}


void cmd_help() {

}


void cmd_peek(char* peer_name, conc_AVL peers) {
    struct _peer p;
    p.name = peer_name;
    peer ret = concurrent_avl_search_by(peers, &p, peer_compare_names);
    if (ret == NULL) {
        printf("[ERROR] the peer doesn't exist\n");
        return;
    }
    int newsocket = create_tcp_client_socket(ret->port, ret->ip);
    if (newsocket) {
        printf("[ERROR] unable to connect to peer\n");
        return;
    }
    int msg_len, msg_len_network_order, nbytes;
    char* msg = PEEK_REQUEST_MSG;
    msg_len = strlen(msg);
    msg_len_network_order = htons(msg_len);
    nbytes = send_tcp_message(newsocket, (char*) &msg_len_network_order, HEADER_LENGTH);
    if (nbytes < 0) {
        return;
    }
    nbytes = send_tcp_message(newsocket, msg, msg_len);
    if (nbytes < 0) {
        return;
    }

    nbytes = recv_tcp_message(newsocket, (char*) &msg_len_network_order, HEADER_LENGTH);
    msg_len = ntohs(msg_len_network_order);
    Array files = array_create(100, str_copy, str_delete, str_print);
    char buffer[1024];
    while (msg_len != END_OF_REQUEST) {
        recv_tcp_message(newsocket, buffer, msg_len);
        array_add(files, buffer);
        nbytes = recv_tcp_message(newsocket, (char*) &msg_len_network_order, HEADER_LENGTH);
        msg_len = ntohs(msg_len_network_order);
    }
    array_print(files);
}

void cmd_download() {

}

void run_command(Array input, cli_args s) {
    if (input == NULL)
        return;
    conc_AVL files = s->files;
    conc_AVL peers = s->peers;
    CMD ret = command_mapping(array_idx(input, 0));
    switch (ret)
    {
    case HELP:
        /* code */
        break;

    case PEEK:
        if (array_size(input) != 2) {
            printf("invalid number of arguments\n");
            return;
        }
        char* peer_name = array_idx(input, 1);
        cmd_peek(peer_name, peers);

        break;
    default:
        break;
    }
}



void* start_cli(void* arg) {

    cli_args s = (cli_args) arg;

    while(1) {
        printf("-->");
        fflush(stdout);
        char buffer[512];
        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
            continue;

        size_t len = strlen(buffer);
        if (len > 0)
            buffer[len-1] = '\0';
        if (len == 0)
            continue;
        Array input = parse_input(buffer, " ");
        run_command(input, s);
        array_destroy(input);
    }
    return NULL;
}