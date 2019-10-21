#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../src/common/utils.h"
#include "../src/common/base58.h"
#include "../src/btc/btc_transaction.h"
#include "../src/common/rlp.h"
#include "../src/eth/eth_transaction.h"
#include "../src/eth_m/eth_m_transaction.h"
#include "../src/common/utils.h"
#include "../src/eos/eos_transaction.h"

static int btc_hasher(void * param, uint8_t * data, size_t len) {
    uint8_t hex[512] = {0};
    int l = buffer_to_hex(data, len, hex, 512);
    printf("btc_hasher: %s\n", hex);
    return 0;
}

int test_btc(int argc, char *argv[]){
    // char hex[64 + 1] = {0};
    // char buffer[20] = {0x23, 0x15, 0x15, 0xf0, 0x9a, 0x55, 0x0c, 0x18, 0x27, 0x51, 0x97, 0x96, 0x25, 0xba, 0x96, 0xba, 0xc0, 0xab, 0x6e, 0x93};

    // buffer_to_hex(buffer, 20, hex, 64);
    // printf("hex: %s\n", hex);

    // hex_to_buffer(hex, 64, buffer, 20);
    // buffer_to_hex(buffer, 20, hex, 64);
    // printf("hex: %s\n", hex);

    // printf("\n");

    // char b58[64 + 1] = {0};
    // int len = encode_base58(buffer, 20, b58, 65);
    // printf("b58: %s  %d\n", b58, len);

    // len = decode_base58(b58, len, buffer, 20);
    // buffer_to_hex(buffer, 20, hex, 65);
    // printf("hex: %s %d\n", hex, len);

    char * raw = NULL;
    raw = "0100000004128a2f38c0bfe3197904be4f9c93457947a5d18096e585b5a0f872a8f6fe396c00000000fc004730440220517db696f1ef2ed7ffbbead9da045875f33e540c067d4bcd6b447e5f03f5901d02202a6f5c6ce91fbf668e5d4ff86f2c6e74d616c26f398aab5a42d49c4d5225a1d201473044022033c8cf2ce0272c73583a6e53b9eb8aecdc3ae657b7af76e8a346428b14e2ed1a02204e6c1fcb59860ef95bc489dbaef3f11bc26584af3712121837514f5e33462e61014c695221032f15aca1f145b2c54a7ef074ab7754c2fc6ed67693db7024a5fa18905811949521030d407c94c749f2ea9f569147f5b849b874fd7fe961afde7508ed14af70245792210309786bccdf40b59a43be50027408fad3979a3a28449a4c86f052889b15a2388853aeffffffffb596f8aa1be076fca9ecb8699bf9bec96657d91d1be0a2b723b723e61048af4b00000000fdfe0000483045022100a1d5ad5e86324bf18ad09e79372db3212ae5830656ab68840ab205de7c5e031002203ae236f3ee58922864ed2efca294e6a7192dd9088a745c914fb61218e1ef5d4301483045022100f122b71b0f9312f94c6f618c79a1859af6bbc2a3ca8fe84d50103def08ba0be80220388ed2a69d4bb73d5c40311a5715d19f6a67049cb48ac91abcca24c47775ff4e014c695221032f15aca1f145b2c54a7ef074ab7754c2fc6ed67693db7024a5fa18905811949521030d407c94c749f2ea9f569147f5b849b874fd7fe961afde7508ed14af70245792210309786bccdf40b59a43be50027408fad3979a3a28449a4c86f052889b15a2388853aeffffffff2b4186518b38648d533e385a2ec705bf4952b848a70c9de38224fa29715c00fa00000000fdfd0000483045022100c68ec9c0735ac0c42cbbd76fc6aa3047989c05f5bcefb4ff05fe75c687c6288f022043db4e0cfbacffdaf7dce2f818800fb45b8834c1413f6c07c41d1002e740c0fd01473044022060789debd30dc91d3a0af5f470745c0c5bdae3cad8cdb9b7f0e2bf0348d390000220512fe5cdb6860a7a4bd6fbb52c3415d6ef747635ca4ab6cd78deaece13ef2604014c695221032f15aca1f145b2c54a7ef074ab7754c2fc6ed67693db7024a5fa18905811949521030d407c94c749f2ea9f569147f5b849b874fd7fe961afde7508ed14af70245792210309786bccdf40b59a43be50027408fad3979a3a28449a4c86f052889b15a2388853aeffffffff11b37c8a89390c85cf25aa86c2f432aa95e901a05db4df4e234b69d870c994ed01000000fdfe0000483045022100ac29c8038f9517aca7f798e219e5bba4d65f564561adf73829c50c9c178595fa02200e7924b28216dba11531723d6ea689f0c644576b27a25b0f2a1b19d8a262807e01483045022100f6ba34962d9ddbb95e8f4c0694f4f17263ab72c62dfb141a76c462cbc41de9260220683fb82e05381c40a44a741bbeef055233aa299d604dbb01a27d549b41044eb8014c695221032f15aca1f145b2c54a7ef074ab7754c2fc6ed67693db7024a5fa18905811949521030d407c94c749f2ea9f569147f5b849b874fd7fe961afde7508ed14af70245792210309786bccdf40b59a43be50027408fad3979a3a28449a4c86f052889b15a2388853aeffffffff02010000000000000017a91444b733537a711be317e7ae5323f6ea31a0b85fa18738d50d000000000017a914454f89a46457d765af69b3ebdfdfe5f6a98a28088700000000";
    if (argc > 1) {
        raw = argv[1];
    }

    if (!raw) {
        printf("raw tx is required\n");
        exit(1);
    }

    char buffer[2048] = {0};
    int len;
    len = hex_to_buffer(raw, strlen(raw), buffer, 2048);

    btc_tx_t tx;
    btc_tx_init(&tx, buffer, len);

    printf("tx: version=%d lock_time=%d has_witnesses=%d input_len=%d output_len=%d\n", tx.version, tx.lock_time, tx.has_witnesses, tx.input_len, tx.output_len);

    char script[512 + 1] = {0};
    char hash[64 + 1] = {0};

    for (int i = 0; i < tx.input_len; i++) {
        btc_tx_input_t input;
        btc_tx_get_input(&tx, i, &input);
        len = buffer_to_hex(input.hash, 32, hash, 64);
        hash[len] = 0;
        len = buffer_to_hex(input.script, input.script_len, script, 512);
        script[len] = 0;
        printf("input #%d: hash=%s index=%d script=%s\n", i, hash, input.index, script);
    }

    for (int i = 0; i < tx.output_len; i++) {
        btc_tx_output_t output;
        btc_tx_get_output(&tx, i, &output);
        len = buffer_to_hex(output.script, output.script_len, script, 512);
        script[len] = 0;
        printf("output #%d: value=%d script=%s\n", i, output.value, script);
    }

    int32_t value = 999;
    char b[32] = {0};
    len = encode_var_int(b, value);
    len = buffer_to_hex(b, len, hash, 64);
    hash[len] = 0;
    printf("encode_var_int: %s\n", hash);
    decode_var_int(b, &value);
    printf("buffer_to_hex: %d\n", value);

    uint8_t sig_hash[512] = {0};
    uint8_t sig_hash_hex[512] = {0};
    char * pre_script_hex = "76a914f2cacab36df7e0ebf4e39024478380403309aed088ac";
    uint8_t pre_script[512] = {0};

    len = hex_to_buffer(pre_script_hex, 50, pre_script, 512);
    btc_tx_hash_for_signature(&tx, 0, SIGHASH_ALL, pre_script, len, &btc_hasher, (void*)sig_hash);
    len = buffer_to_hex(sig_hash, len, sig_hash_hex, 512);
    sig_hash_hex[len] = 0;
    printf("sig_hash #%d: %s\n", 0, sig_hash_hex);

    return 0;    
}

int test_rlp() {
    char * raw = "f86b3585033d660e6682520894267d8233bdc52e5c455fb58afd62f3feacaf30c68712e683063480008026a098674ed864f843a71cefad236de39b08583e4e56421015ebf4b71f0478d303d3a007907a9f0225a99af3e5d04076cd238876d12e391a0ee0a92f1e663bffa4c2b8";
    char buffer[2048] = {0};
    int buffer_len = hex_to_buffer(raw, strlen(raw), buffer, sizeof(buffer));
    int rlp_length;
    int ret = rlp_get_array_length(buffer, buffer_len, &rlp_length);

    printf("rlp_get_array_length: %d\n", rlp_length);

    char data[128];
    int data_len;
    char hex[1024] = {0};

    for (int i = 0; i < rlp_length; i++) {
        ret = rlp_get_array_item(buffer, buffer_len, i, data, &data_len, sizeof(data));

        memset(hex, 0, sizeof(hex));
        buffer_to_hex(data, data_len, hex, sizeof(hex));
        printf("rlp_get_array_item: %d %s %d\n", i, hex, ret);
    }

    eth_tx_t tx;
    eth_tx_init(&tx, buffer, buffer_len);
    ret = eth_tx_get_to(&tx, data, &data_len, sizeof(data));

    memset(hex, 0, sizeof(hex));
    buffer_to_hex(data, data_len, hex, sizeof(hex));
    printf("eth_tx_get_to: %s %d\n", hex, ret);

    return 0;
}

int test_eos() {
    char * raw = "aca376f206b8fc25a6ed44dbdc66547c36c6c33e3a119ffbeaef943642f0e90640147a5da9ce76e6c0dc000000000100a6823403ea3055000000572d3ccdcd0110c2555867acb24300000000a8ed32322410c2555867acb24320c4555867acb243010000000000000004454f530000000003313233000000000000000000000000000000000000000000000000000000000000000000";
    char buffer[2048] = {0};
    int buffer_len = hex_to_buffer(raw, strlen(raw), buffer, sizeof(buffer));

    int ret;

    eos_tx_t tx;
    ret = eos_tx_init(&tx, buffer, buffer_len);
    printf("eos_tx_init: ret=%d action_len=%d\n", ret, tx.action_len);

    eos_tx_action_t action;
    ret = eos_tx_get_action(&tx, 0, &action);
    printf("eos_tx_get_action: ret=%d account=%p name=%p authorization=%d data=%d\n", ret, action.account, action.name, action.authorization_len, action.data_len);
    
    int len;
    char name[16];

    memset(name, 0, sizeof(name));
    ret = eos_decode_name(action.account, name, &len);
    printf("eos_tx_get_action: account=%s len=%d\n", name, len);

    memset(name, 0, sizeof(name));
    ret = eos_decode_name(action.name, name, &len);
    printf("eos_tx_get_action: name=%s len=%d\n", name, len);

    eos_tx_action_transfer_t transfer;
    eos_tx_get_action_transfer(&action, &transfer);
    printf("eos_tx_get_action_transfer: ret=%d from=%p to=%p amount=%ld symbol=%p memo=%d\n", ret, transfer.from, transfer.to, transfer.amount, transfer.symbol, transfer.memo_len);

    memset(name, 0, sizeof(name));
    ret = eos_decode_name(transfer.from, name, &len);
    printf("eos_tx_get_action_transfer: from=%s len=%d\n", name, len);

    memset(name, 0, sizeof(name));
    ret = eos_decode_name(transfer.to, name, &len);
    printf("eos_tx_get_action_transfer: to=%s len=%d\n", name, len);

    memset(name, 0, sizeof(name));
    ret = eos_decode_symbol(transfer.symbol, name, &len);
    printf("eos_tx_get_action_transfer: symbol=%s len=%d\n", name, len);

    return 0;
}

void test_eth_m_parse() {
    eth_m_tx_t tx;
    uint8_t hex[] = "455448455212a7e460c63ba02d237f333db295b62b1bd8dd0100000000000000000000000000000000000000000000000000000000001e848000000000000000000000000000000000000000000000000000000000005e4dcc000000000000000000000000000000000000000000000000000000000000000002";
    uint8_t buffer[sizeof(hex) / 2] = {0};
    uint8_t len = hex_to_buf(hex, sizeof(hex), buffer, sizeof(buffer));
    assert(buffer[0] == 0x45);
    assert(buffer[1] == 0x54);
    assert(buffer[120] == 0x00);
    assert(buffer[121] == 0x02);

    eth_m_tx_init(&tx, buffer, len);
    eth_m_tx_update(&tx);

    uint8_t *address =NULL;
    assert(eth_m_tx_get_address(&tx, &address) == 20);
    uint8_t expect_address[] = {0x12, 0xa7, 0xe4, 0x60, 0xc6, 0x3b, 0xa0, 0x2d, 0x23, 0x7f, 0x33, 0x3d, 0xb2, 0x95, 0xb6, 0x2b, 0x1b, 0xd8, 0xdd, 0x01};
    assert(memcmp(address, expect_address, sizeof(expect_address)) == 0);

    uint8_t *value =NULL;
    assert(eth_m_tx_get_value(&tx, &value) == 32);
    assert(value[29] == 0x1e);
    assert(value[30] == 0x84);
    assert(value[31] == 0x80);

    printf("test_eth_m_parse passed!\n");
}

void test_eth_m_token_parse() {
    eth_m_tx_t tx;
    uint8_t hex[] = "455243323012a7e460c63ba02d237f333db295b62b1bd8dd010000000000000000000000000000000000000000000000000000000000000001414a06fb01ddbe5deb2d4af5bd4f94b02d93a52700000000000000000000000000000000000000000000000000000004a817c8000000000000000000000000000000000000000000000000000000000000000004";
    uint8_t buffer[sizeof(hex) / 2] = {0};
    uint8_t len = hex_to_buf(hex, sizeof(hex), buffer, sizeof(buffer));
    assert(buffer[0] == 0x45);
    assert(buffer[1] == 0x52);
    assert(buffer[139] == 0x00);
    assert(buffer[140] == 0x04);

    eth_m_tx_init(&tx, buffer, len);
    eth_m_tx_update(&tx);

    uint8_t *address = NULL;
    assert(eth_m_token_tx_get_address(&tx, &address) == 20);
    uint8_t expect_address[] = {0x12,0xa7,0xe4,0x60,0xc6,0x3b,0xa0,0x2d,0x23,0x7f,0x33,0x3d,0xb2,0x95,0xb6,0x2b,0x1b,0xd8,0xdd,0x01};
    assert(memcmp(address, expect_address, sizeof(expect_address)) == 0);

    uint8_t *value = NULL;
    assert(eth_m_token_tx_get_value(&tx, &value) == 32);
    assert(value[31] == 1);
    assert(value[30] == 0);

    uint8_t *token_addr = NULL;
    assert(eth_m_token_tx_get_token_addr(&tx, &token_addr) == 20);
    uint8_t expect_token_addr[] = {0x41,0x4a,0x06,0xfb,0x01,0xdd,0xbe,0x5d,0xeb,0x2d,0x4a,0xf5,0xbd,0x4f,0x94,0xb0,0x2d,0x93,0xa5,0x27};
    assert(memcmp(token_addr, expect_token_addr, sizeof(expect_token_addr)) == 0);

    printf("test_eth_m_token_parse passed!\n");
}

int main(int argc, char *argv[]) {
    test_btc(argc, argv);
    test_eos();
    test_eth_m_parse();
    test_eth_m_token_parse();
}
