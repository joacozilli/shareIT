#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "server.h"
#include "cJSON.h"



/**
 * auxiliar function to check for errors in config.
 * type 0 --> string value
 * type 1 --> int value
 */
void check_field_error(cJSON* field, char* field_name, int type) {
    if(!field) {
        log_error("%s field undefined in config file", field_name);
        fclose(log_file);
        exit(EXIT_FAILURE);
    }
    if ((type == 0 && !cJSON_IsString(field)) || (type == 1 && !cJSON_IsNumber(field))) {
        log_error("invalid value for %s field in config file", field_name);
        fclose(log_file);
        exit(EXIT_FAILURE);
    }
    return;
}




int main() {
    char config_buffer[2048];
    FILE* config_file = fopen("config.json", "r");
    if (!config_file) {
        log_errno("error with fopen (unable to open config file)");
        return EXIT_FAILURE;
    }
    fread(config_buffer, sizeof(char) , sizeof config_buffer, config_file);
    fclose(config_file);

    log_file = fopen("shareit.log", "w");
    if (!log_file) {
        log_errno("error with fopen (unable to open log file)");
        return EXIT_FAILURE;
    }

    cJSON* json = cJSON_Parse(config_buffer);
    if (json == NULL) {
        log_error("unable to parse json string");
        return EXIT_FAILURE;
    }
    cJSON *json_srv_name = cJSON_GetObjectItemCaseSensitive(json, "srv_name");
    check_field_error(json_srv_name, "srv_name", 0);

    cJSON *json_srv_port = cJSON_GetObjectItemCaseSensitive(json, "srv_port");
    check_field_error(json_srv_port, "srv_port", 1);

    cJSON *json_srv_ip = cJSON_GetObjectItemCaseSensitive(json, "srv_ip");
    check_field_error(json_srv_ip, "srv_ip", 0);

    cJSON *json_broadcast_port = cJSON_GetObjectItemCaseSensitive(json, "broadcast_port");
    check_field_error(json_broadcast_port, "broadcast_port", 1);

    cJSON *json_broadcast_ip = cJSON_GetObjectItemCaseSensitive(json, "broadcast_ip");
    check_field_error(json_broadcast_ip, "broadcast_ip", 0);

    char* srv_name = strdup(json_srv_name->valuestring);
    int srv_port = json_srv_port->valueint;
    char* srv_ip = strdup(json_srv_ip->valuestring);
    int broadcast_port = json_broadcast_port->valueint;
    char* broadcast_ip = json_broadcast_ip->valuestring;

    cJSON_Delete(json);

    

    start_node(srv_port, srv_ip, broadcast_port, broadcast_ip, srv_name, "./share");
    
    return 0;
}