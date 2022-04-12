#include <json-c/json.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>
#include <uuid/uuid.h>

#include "defs.h"
#include "utils.h"

typedef enum { OP_AUTHENTICATE, OP_FRAME, OP_CLOSE } rpc_op;

static int32_t sock = -1;

void rpc_connect() {
    struct sockaddr_un addr;
    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("[ERROR] socket failed");
        exit(1);
    }
    memset(&addr, 0, sizeof addr);
    addr.sun_family = AF_UNIX;

    const char *path = TMP_PATH;
    strncpy(addr.sun_path, path, strlen(path));
    strncat(addr.sun_path, "/discord-ipc-0", 15);
    printf("[INFO] connecting to %s\n", addr.sun_path);

    if (connect(sock, (struct sockaddr *) &addr, sizeof addr) == -1) {
        perror("[ERROR] connect failed");
        exit(1);
    }
}

void rpc_send(rpc_op op, const char *data) {
    int32_t len = strlen(data);
    uint8_t op_b[4], len_b[4];
    INT_TO_BYTES(op, op_b);
    INT_TO_BYTES(len, len_b);

    uint8_t *payload = malloc(8L + len);
    memcpy(payload, op_b, 4L);
    memcpy(payload + 4, len_b, 4L);
    memcpy(payload + 8, data, len);

    LOG("[DEBUG] sending ");
    LOG_B(op_b);
    LOG_B(len_b);
    LOG("%s\n", data);
    if (send(sock, payload, 8L + len, 0) == -1) {
        free(payload);
        perror("[ERROR] send failed");
        exit(1);
    }
    free(payload);
}

void rpc_recv(uint8_t *op, uint8_t *len, json_object **obj) {
    if (recv(sock, op, 4L, 0) == -1) {
        perror("[ERROR] recv failed");
        exit(1);
    }
    LOG("[DEBUG] received ");
    LOG_B(op);

    if (recv(sock, len, 4L, 0) == -1) {
        fflush(stderr);
        perror("[ERROR] recv failed");
        exit(1);
    }
    uint32_t l = BYTES_TO_INT(len);
    LOG_B(len);

    char *response = malloc(l);
    if (recv(sock, response, l, 0) == -1) {
        fflush(stderr);
        perror("[ERROR] recv failed");
        exit(1);
    }
    LOG("%s\n", response);

    *obj = JSON_PARSE(response);
    free(response);
}

void rpc_handshake() {
    json_object *r_obj, *s_obj = JSON_NEW();
    JSON_ADD(s_obj, v, 1, int);
    JSON_ADD(s_obj, client_id, CLIENT_ID, string);

    rpc_send(OP_AUTHENTICATE, JSON_TO_STRING(s_obj));
    JSON_FREE(s_obj);

    uint8_t op[4], len[4];
    rpc_recv(op, len, &r_obj);

    const char *evt = JSON_GET(r_obj, evt, string);
    if (!STR_EQ(evt, "READY")) {
        const char *msg = JSON_GET(r_obj, message, string);
        JSON_FREE(r_obj);
        fprintf(stderr, "[ERROR] received evt: %s, message: %s\n", evt, msg);
        exit(2);
    }
    JSON_FREE(r_obj);
}

void rpc_set_activity() {
    uint8_t op[4], len[4], uuid[16];

    char *nonce = malloc(37L);
    uuid_generate_random(uuid);
    uuid_unparse_upper(uuid, nonce);

    json_object *r_obj, *s_obj = JSON_NEW();
    JSON_ADD(s_obj, cmd, "SET_ACTIVITY", string);
    JSON_ADD(s_obj, nonce, nonce, string);

    json_object *activity = JSON_NEW();
    JSON_ADD_S(activity, details, DETAILS);
    JSON_ADD_S(activity, state, STATE);

    json_object *timestamps = JSON_NEW();
    JSON_ADD(timestamps, start, TIME, int);
    JSON_ADD_OBJ(activity, timestamps, timestamps);

    json_object *assets = JSON_NEW();
    JSON_ADD_S(assets, large_image, LARGE_IMAGE);
    JSON_ADD_S(assets, large_text, LARGE_TEXT);
    JSON_ADD_S(assets, small_image, SMALL_IMAGE);
    JSON_ADD_S(assets, small_text, SMALL_TEXT);
    JSON_ADD_OBJ(activity, assets, assets);

    json_object *args = JSON_NEW();
    JSON_ADD(args, pid, PID, int);
    JSON_ADD_OBJ(args, activity, activity);
    JSON_ADD_OBJ(s_obj, args, args);

    rpc_send(OP_FRAME, JSON_TO_STRING(s_obj));
    JSON_FREE(s_obj);
    free(nonce);

    rpc_recv(op, len, &r_obj);
    const char *evt = JSON_GET(r_obj, evt, string);
    if (evt != NULL && STR_EQ(evt, "ERROR")) {
        json_object *data = JSON_GET_OBJ(r_obj, data);
        const char *msg = JSON_GET(data, message, string);
        JSON_FREE(r_obj);
        fprintf(stderr, "[ERROR] received message: %s\n", msg);
        exit(3);
    }
    JSON_FREE(r_obj);
}

static void rpc_disconnect() {
    if (sock != -1) {
        rpc_send(OP_CLOSE, "");
        close(sock);
    }
}

static void quit(__attribute__((unused)) int32_t sig) {
    fflush(stderr);
    exit(0);
}

int main() {
    atexit(rpc_disconnect);
    signal(SIGINT, quit);
    rpc_connect();
    rpc_handshake();
    rpc_set_activity();
    while (1) {}
}
