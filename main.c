#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>

#include "log.h"
#include "server/server.h"
#include "cJSON.h"
#include "directories.h"
#include "array.h"
#include "str.h"


/**
 * auxiliar function to check for errors in config.
 * type 0 --> string value
 * type 1 --> int value
 * type 2 --> array value
 */
void check_field_error(cJSON* field, char* field_name, int type, cJSON* json) {
    if(!field) {
        log_error("%s field undefined in config file", field_name);
        fclose(log_file);
        cJSON_Delete(json);
        exit(EXIT_FAILURE);
    }
    if ((type == 0 && !cJSON_IsString(field)) || (type == 1 && !cJSON_IsNumber(field))
        || (type == 2 && !cJSON_IsArray(field))) {
        log_error("invalid value for %s field in config file", field_name);
        fclose(log_file);
        cJSON_Delete(json);
        exit(EXIT_FAILURE);
    }
    return;
}

/**
 * Read the json field 'shared' and return and array with the directories's paths.
 * Return NULL on error.
 */
Array get_shared_dirs(cJSON* json_shared) {
    Array shared_dirs = array_create(10, str_copy, str_delete, str_print);
    int size = cJSON_GetArraySize(json_shared);
    for (int i = 0; i < size; i++) {
        cJSON *item = cJSON_GetArrayItem(json_shared, i);

        if (!cJSON_IsString(item)) {
            log_error("invalid value for shared field in config file");
            return NULL;
        }    
        array_add(shared_dirs, item->valuestring);
    }
    return shared_dirs;
}



int main() {

    
    const char *homedir;

    if ((homedir = getenv("HOME")) == NULL)
        homedir = getpwuid(getuid())->pw_dir;

    char config_path[255];
    snprintf(config_path, sizeof config_path, "%s/.shareit/config/config.json", homedir);
    char log_path[255];
    snprintf(log_path, sizeof log_path, "%s/.shareit/log/shareit.log", homedir);

    char config_buffer[2048];
    FILE* config_file = fopen(config_path, "r");
    if (!config_file) {
        fprintf(stderr, "error with fopen (unable to open config file)\n");
        return EXIT_FAILURE;
    }
    fread(config_buffer, sizeof(char) , sizeof config_buffer, config_file);
    fclose(config_file);

    log_file = fopen(log_path, "w");
    if (!log_file) {
        fprintf(stderr, "error with fopen (unable to open log file)\n");
        return EXIT_FAILURE;
    }

    cJSON* json = cJSON_Parse(config_buffer);
    if (json == NULL) {
        log_error("unable to parse json string");
        return EXIT_FAILURE;
    }
    cJSON *json_srv_name = cJSON_GetObjectItemCaseSensitive(json, "srv_name");
    check_field_error(json_srv_name, "srv_name", 0, json);

    cJSON *json_srv_port = cJSON_GetObjectItemCaseSensitive(json, "srv_port");
    check_field_error(json_srv_port, "srv_port", 1, json);

    cJSON *json_srv_ip = cJSON_GetObjectItemCaseSensitive(json, "srv_ip");
    check_field_error(json_srv_ip, "srv_ip", 0, json);

    cJSON *json_broadcast_port = cJSON_GetObjectItemCaseSensitive(json, "broadcast_port");
    check_field_error(json_broadcast_port, "broadcast_port", 1, json);

    cJSON *json_broadcast_ip = cJSON_GetObjectItemCaseSensitive(json, "broadcast_ip");
    check_field_error(json_broadcast_ip, "broadcast_ip", 0, json);

    cJSON* json_shared = cJSON_GetObjectItemCaseSensitive(json, "shared");
    check_field_error(json_shared, "shared", 2, json);

    Array shared_dirs = get_shared_dirs(json_shared);
    if (!shared_dirs) {
        fclose(log_file);
        cJSON_Delete(json);
        exit(EXIT_FAILURE);
    }
    
    char* srv_name = strdup(json_srv_name->valuestring);
    int srv_port = json_srv_port->valueint;
    char* srv_ip = strdup(json_srv_ip->valuestring);
    int broadcast_port = json_broadcast_port->valueint;
    char* broadcast_ip = strdup(json_broadcast_ip->valuestring);

    cJSON_Delete(json);

    start_node(srv_port, srv_ip, broadcast_port, broadcast_ip, srv_name, shared_dirs);
    
    return 0;
}