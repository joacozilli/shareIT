#include "cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "log.h"
#include "network.h"
#include "server.h"
#include "utils.h"
#include "peer.h"
#include "files.h"
#include "str.h"

/**
 * Auxiliar function to map each command to a CMD type.
 */
CMD command_mapping(char* str) {
    if (!strcmp(str, "help"))
        return HELP;
    if (!strcmp(str, "neighbors"))
        return NEIGHBORS;
    if (!strcmp(str, "peek"))
        return PEEK;
    if (!strcmp(str, "search"))
        return SEARCH;
    if (!strcmp(str, "download"))
        return DOWNLOAD;
    
    return UNDEFINED;
}


void cmd_help() {
    printf("COMMAND NAME                         DESCRIPTION\n");
    printf("help                                 show this message\n");
    printf("neighbors                            list all known peers\n");
    printf("peek [PEER_NAME]                     see all files shared byt peer PEER_NAME\n");
    printf("download [FILE_NAME] [PEER_NAME]     download file FILE_NAME from peer PEER_NAME\n");
    printf("--------------------------------------------------------------------------------\n");
}

/**
 * function to be used in cmd_peek() for mapping the array of files peeked.
 */
void* print_peeked_file(void* file, void* context) {
    if(context == NULL) {}; // just to calm down the compiler. Context arg is not used.
    char* f = (char*) file;
    Array parsed = parse_input(f, " ");
    if (parsed == NULL)
        return file;
    
    if (array_size(parsed) != 2) {
        array_destroy(parsed);
        return file;
    }
    
    char file_name_buff[FILE_NAME_SPACE];
    char file_size_buff[FILE_SIZE_SPACE];

    int file_name_len = strlen(array_idx(parsed, 0));
    strncpy(file_name_buff, array_idx(parsed, 0), file_name_len);
    for (int i = file_name_len; i < FILE_NAME_SPACE-1; file_name_buff[i] = ' ', i++);
    file_name_buff[FILE_NAME_SPACE-1] = '\0';

    int file_size_len = strlen(array_idx(parsed, 1));
    strncpy(file_size_buff, array_idx(parsed, 1), file_size_len);
    for (int i = file_size_len; i < FILE_SIZE_SPACE-1; file_size_buff[i] = ' ', i++);
    file_size_buff[FILE_SIZE_SPACE-1] = '\0';

    printf("%s | %s bytes\n", file_name_buff, file_size_buff);
    array_destroy(parsed);
    return file;
}

void cmd_peek(char* peer_name, conc_AVL peers) {
    struct _peer p;
    p.name = peer_name;
    peer ret = concurrent_avl_search_by(peers, &p, peer_compare_names);
    if (ret == NULL) {
        printf("[ERROR] the peer doesn't exist.\n");
        return;
    }
    int newsocket = create_tcp_client_socket(ret->port, ret->ip);
    if (newsocket < 0) {
        printf("[ERROR] unable to connect to peer.\n");
        return;
    }

    uint16_t msg_len, msg_len_network_order;
    int nbytes;
    char* msg = PEEK_REQUEST_MSG;
    msg_len = strlen(msg);
    msg_len_network_order = htons(msg_len);
    nbytes = send_tcp_message(newsocket, (char*) &msg_len_network_order, HEADER_LENGTH);
    if (nbytes < 2) {
        printf("[ERROR] unable to send peek reequest to %s.\n", peer_name);
        log_error("unable to send tcp message header");
        return;
    }
    nbytes = send_tcp_message(newsocket, msg, msg_len);
    if (nbytes < msg_len) {
        printf("[ERROR] unable to send peek reequest to %s.\n", peer_name);
        log_error("unable to send tcp message body");
        return;
    }

    log_info("sent peek request to peer of name %s", peer_name);

    nbytes = recv_tcp_message(newsocket, (char*) &msg_len_network_order, HEADER_LENGTH);
    msg_len = ntohs(msg_len_network_order);
    Array files = array_create(100, str_copy, str_delete, NULL);
    char buffer[1024];
    while (msg_len != END_OF_REQUEST) {
        recv_tcp_message(newsocket, buffer, msg_len);
        buffer[msg_len] = '\0';
        array_add(files, buffer);
        nbytes = recv_tcp_message(newsocket, (char*) &msg_len_network_order, HEADER_LENGTH);
        msg_len = ntohs(msg_len_network_order);
    }
    log_info("peek request to peer of name %s has been completed", peer_name);
    if (array_size(files) > 0) {
        char name_col[FILE_NAME_SPACE];
        char size_col[FILE_SIZE_SPACE];

        strcpy(name_col, "FILE NAME");
        int name_col_len = strlen(name_col);
        for (int i = name_col_len; i < FILE_NAME_SPACE-1; name_col[i] = ' ', i++);
        name_col[FILE_NAME_SPACE-1] = '\0';    
        
        strcpy(size_col, "FILE SIZE");
        int size_col_len = strlen(size_col);
        for (int i = size_col_len; i < FILE_SIZE_SPACE-1; size_col[i] = ' ', i++);
        size_col[FILE_SIZE_SPACE-1] = '\0';

        printf("%s %s\n", name_col, size_col);

        array_map(files, print_peeked_file, NULL);

    }
    array_destroy(files);
}


void* download_file(void* _file_name, void* context) {
    char* file_name = (char*) _file_name;

    download_file_context cont = (download_file_context) context;
    int fd = cont->fd;
    conc_AVL my_files = cont->files;
    
    char msg[1024];
    snprintf(msg, 1024, "DOWNLOAD_REQUEST %s", file_name);
    u_int16_t msg_len = strlen(msg);
    u_int16_t msg_len_network_order = htons(msg_len);

    int nbytes = send_tcp_message(fd, (char*) &msg_len_network_order, HEADER_LENGTH);
    if (nbytes < 2) {
        printf("[ERROR] unable to send download request for file %s.\n", file_name);
        log_error("unable to send tcp message header");
        return _file_name;
    }

    nbytes = send_tcp_message(fd, msg, msg_len);

    if (nbytes < msg_len) {
        printf("[ERROR] unable to send download request for file %s.\n", file_name);
        log_error("unable to send tcp message body");
        return _file_name; 
    }
    u_int8_t answer;
    nbytes = recv_tcp_message(fd, (char*) &answer, sizeof answer);

    if (answer == FILE_NOT_FOUND_CODE)
        printf("[ERROR] peer said it doesn't have file %s. Its download has been aborted.\n", file_name);

    else if (answer == FILE_FOUND_CODE) {
        uint32_t file_size_network_order;
        uint32_t file_size;

        nbytes = recv_tcp_message(fd, (char*) &file_size_network_order, sizeof file_size_network_order);
        if (nbytes < (int) sizeof file_size_network_order) {
            printf("[ERROR] unable to download file %s.\n", file_name);
            log_error("couldn't read the size of file");
            return _file_name;
        }

        file_size = ntohl(file_size_network_order);
        uint32_t total = 0;
        char path[1024];
        snprintf(path, 1024, "./share/%s", file_name);
        FILE* new = fopen(path, "wb");

        char buffer[FILE_TRANSFER_CHUNK_SIZE];

        printf("file %s is of size %d bytes.\nStarting download...\n", file_name, file_size);
        while (total < file_size) {
            int bytes_to_read = file_size - total < FILE_TRANSFER_CHUNK_SIZE \
                                                    ? file_size - total \
                                                    : FILE_TRANSFER_CHUNK_SIZE;

            nbytes = recv_tcp_message(fd, buffer, bytes_to_read);
            if (nbytes <= 0) {
                printf("[ERROR] there was an unexpected error while download file %s.\n", file_name);
                log_error("a chunk was unable to arrive during a file transfer");
                fclose(new);
                return _file_name;
            }

            fwrite(buffer, 1, nbytes, new);
            total += nbytes;
        }
        printf("Download completed. %d bytes were received in total.\n", total);
        fclose(new);

        struct _file_info new_file;
        new_file.name = file_name;
        new_file.size = file_size;
        new_file.path = path;
        concurrent_avl_insert(my_files, &new_file);
    }

    return _file_name;
}

void cmd_download(Array files_to_dl, char* peer_name, conc_AVL peers, conc_AVL my_files) {
    struct _peer p;
    p.name = peer_name;
    peer ret = concurrent_avl_search_by(peers, &p, peer_compare_names);
    if (ret == NULL) {
        printf("[ERROR] the peer doesn't exist.\n");
        return;
    }

    int fd = create_tcp_client_socket(ret->port, ret->ip);
    if (fd < 0) {
        printf("[ERROR] unable to connect to peer.\n");
        return;
    }

    struct _download_file_context cont;
    cont.fd = fd;
    cont.files = my_files;

    array_map(files_to_dl, download_file, (void*) &cont);
    close(fd);
}

void run_command(Array input, cli_args s) {
    if (input == NULL)
        return;
    conc_AVL peers = s->peers;
    conc_AVL my_files = s->files;
    CMD cmd = command_mapping(array_idx(input, 0));
    switch (cmd)
    {
    case HELP:
        cmd_help();
        break;

    case NEIGHBORS:
        concurrent_avl_print(peers);
        break;
    
    case PEEK:
        if (array_size(input) != 2) {
            printf("[ERROR] invalid number of arguments.\n");
            return;
        }
        char* peer_name = array_idx(input, 1);
        cmd_peek(peer_name, peers);
        break;
    
    case DOWNLOAD:
        if (array_size(input) != 3) {
            printf("[ERROR] invalid number of arguments.\n");
            return;
        }

        /* want to add the possibility of requesting download of multiple files
        from a peer, that is way an array is used.
        */
        Array files_to_dl = array_create(10, str_copy, str_delete, NULL);

        if (array_size(input) == 3) {
            peer_name = array_idx(input, 2);
            char* file_name = array_idx(input, 1);         
            array_add(files_to_dl, file_name);
        }
        cmd_download(files_to_dl, peer_name, peers, my_files);
        array_destroy(files_to_dl);
        break;
    
    case UNDEFINED:
        printf("[ERROR] undefined command.\n");
        break;
    default:
        break;
    }
}



void* start_cli(void* arg) {

    cli_args s = (cli_args) arg;
    printf("welcome!!\n");
    cmd_help();
    while(1) {
        printf("--> ");
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
        if (input == NULL)
            continue;
        run_command(input, s);
        array_destroy(input);
    }
    return NULL;
}