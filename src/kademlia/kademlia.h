#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#include "../list/list.h"
#endif

typedef unsigned char kademlia_key[32];
typedef char kademlia_ip[64];
typedef char kademlia_port[5 + 1];

typedef struct kademlia_peer_triple kademlia_peer_triple;
struct kademlia_peer_triple {
    kademlia_ip ip;
    kademlia_port port;
    kademlia_key key;
};

list1_typedef(kademlia_peer_triple, kademlia_peer_triple);
typedef list1_handle_kademlia_peer_triple k_bucket;

#define DEFAULT_K_REPLICATION_PARAMETER 20

typedef struct kademlia_contact_table kademlia_contact_table;
struct kademlia_contact_table {
    int k; // system wide replication parameter
    k_bucket bucket[8 * sizeof(kademlia_key)];
};

typedef enum kademlia_rpc_id kademlia_rpc_id;
enum kademlia_rpc_id {
    KADEMLIA_RPC_PING,
    KADEMLIA_RPC_STORE,
    KADEMLIA_RPC_FIND_NODE_QUERY,
    KADEMLIA_RPC_FIND_NODE_RESPONSE,
    KADEMLIA_RPC_FIND_VALUE_QUERY,
    KADEMLIA_RPC_FIND_VALUE_RESPONSE,
};

typedef struct kademlia_packet_common kademlia_packet_common;
struct kademlia_packet_common {
    kademlia_rpc_id rpc_id;
    kademlia_key sender_key;    
};

typedef struct kademlia_packet_ping kademlia_packet_ping;
struct kademlia_packet_ping {
    kademlia_packet_common common;
};

typedef struct kademlia_packet_store kademlia_packet_store;
struct kademlia_packet_store {
    kademlia_packet_common common;
    kademlia_peer_triple store;
};

typedef struct kademlia_packet_find_node_query kademlia_packet_find_node_query;
struct kademlia_packet_find_node_query {
    kademlia_packet_common common;
    kademlia_key query;
};

typedef struct kademlia_packet_find_node_response kademlia_packet_find_node_response;
struct kademlia_packet_find_node_response {
    kademlia_packet_common common;
    kademlia_peer_triple node[];
};

typedef struct kademlia_packet_find_value_query kademlia_packet_find_value_query;
struct kademlia_packet_find_value_query {
    kademlia_packet_common common;
    kademlia_key query;
};

typedef struct kademlia_packet_find_value_response kademlia_packet_find_value_response;
struct kademlia_packet_find_value_response {
    kademlia_packet_common common;
    kademlia_peer_triple value[];
};

