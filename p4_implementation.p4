/* -*- P4_16 -*- */

#include <core.p4>
#include <tna.p4>

/*************************************************************************
 ************* C O N S T A N T S    A N D   T Y P E S  *******************
**************************************************************************/

#define IPV4        0x0800 // ETHERTYPE_IPV4
#define UDP         0x11  // PROTO_UDP
#define TCP         0x06  // PROTO_TCP

/*************************************************************************
 ***********************  H E A D E R S  *********************************
 *************************************************************************/

/*  Define all the headers the program will recognize             */
/*  The actual sets of headers processed by each gress can differ */

/* Standard ethernet header */
header ethernet_h {
    bit<48>   dst_addr;
    bit<48>   src_addr;
    bit<16>   ether_type;
}

header ipv4_h {
    bit<4>   version;
    bit<4>   ihl;
    bit<8>   diffserv;
    bit<16>  total_len;
    bit<16>  identification;
    bit<3>   flags;
    bit<13>  frag_offset;
    bit<8>   ttl;
    bit<8>   protocol;
    bit<16>  hdr_checksum;
    bit<32>  src_addr;
    bit<32>  dst_addr;
}

header l4port_h {
    bit<16> src_port;
    bit<16> dst_port;
}

header tcp_h {
    bit<32>  seq_no;
    bit<32>  ack_no;
    bit<4>   data_offset;
    bit<4>   res;
    bit<8>   flags;
    bit<16>  window;
    bit<16>  checksum;
    bit<16>  urgent_ptr;
}

header udp_h {
    bit<16>  len;
    bit<16>  checksum;
}

struct my_ingress_headers_t {
    ethernet_h   ethernet;
    ipv4_h       ipv4;
    l4port_h     ports;
    tcp_h        tcp;
    udp_h        udp;
}

struct my_ingress_metadata_t {
    bit<32> index_sketch1;
    bit<32> index_sketch2;
    bit<32> index_sketch3;
    bit<8>  random;
}

/* -*- P4_16 -*- */

/*************************************************************************
 **************  I N G R E S S   P R O C E S S I N G   *******************
 *************************************************************************/

    /***********************  P A R S E R  **************************/
parser IngressParser(packet_in        pkt,
    /* User */    
    out my_ingress_headers_t          hdr,
    out my_ingress_metadata_t         meta,
    /* Intrinsic */
    out ingress_intrinsic_metadata_t  ig_intr_md)
{
    /* This is a mandatory state, required by Tofino Architecture */
    state start {
        pkt.extract(ig_intr_md);
        pkt.advance(PORT_METADATA_SIZE);
        transition parse_ethernet;
    }

    state parse_ethernet {
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.ether_type) {
            IPV4 :  parse_ipv4;
            default: reject;
        }
    }

    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        transition select(hdr.ipv4.frag_offset, hdr.ipv4.protocol) {
            ( 0, TCP  ) : parse_tcp;
            ( 0, UDP  ) : parse_udp;
            default : reject;
        }
    }

    state parse_tcp {
        pkt.extract(hdr.ports);
        pkt.extract(hdr.tcp);
        transition accept;
    }
    
    state parse_udp {
        pkt.extract(hdr.ports);
        pkt.extract(hdr.udp);
        transition accept;
    }

}
/*************************************************************************
 **************  I N G R E S S   P R O C E S S I N G   *******************
 *************************************************************************/

    /***************** M A T C H - A C T I O N  *********************/

control Ingress(
    /* User */
    inout my_ingress_headers_t hdr,
    inout my_ingress_metadata_t meta,
    /* Intrinsic */
    in ingress_intrinsic_metadata_t ig_intr_md,
    in ingress_intrinsic_metadata_from_parser_t ig_prsr_md,
    inout ingress_intrinsic_metadata_for_deparser_t ig_dprsr_md,
    inout ingress_intrinsic_metadata_for_tm_t ig_tm_md)
{
    CRCPolynomial<bit<32>>(
        coeff    = 0x04C11DB7,
        reversed = true,
        msb      = false,
        extended = false,
        init     = 0xFFFFFFFF,
        xor      = 0xFFFFFFFF) poly1;
    Hash<bit<32>>(HashAlgorithm_t.CUSTOM, poly1) hash_algo1;

    CRCPolynomial<bit<32>>(
        coeff    = 0x741B8CD7,
        reversed = true,
        msb      = false,
        extended = false,
        init     = 0xFFFFFFFF,
        xor      = 0xFFFFFFFF) poly2;
    Hash<bit<32>>(HashAlgorithm_t.CUSTOM, poly2) hash_algo2;

    CRCPolynomial<bit<32>>(
        coeff    = 0xDB710641,
        reversed = true,
        msb      = false,
        extended = false,
        init     = 0xFFFFFFFF,
        xor      = 0xFFFFFFFF) poly3;
    Hash<bit<32>>(HashAlgorithm_t.CUSTOM, poly3) hash_algo3;

    action do_hash1() {
        meta.index_sketch1 = hash_algo1.get({
                hdr.ipv4.src_addr,
                hdr.ipv4.dst_addr,
                hdr.ipv4.protocol,
                hdr.ports.src_port,
                hdr.ports.dst_port
            });
    }

    action do_hash2() {
        meta.index_sketch2 = hash_algo2.get({
                hdr.ipv4.src_addr,
                hdr.ipv4.dst_addr,
                hdr.ipv4.protocol,
                hdr.ports.src_port,
                hdr.ports.dst_port
            });
    }

    action do_hash3() {
        meta.index_sketch3 = hash_algo3.get({
                hdr.ipv4.src_addr,
                hdr.ipv4.dst_addr,
                hdr.ipv4.protocol,
                hdr.ports.src_port,
                hdr.ports.dst_port
            });
    }

    Random<bit<8>>() random;

    action getrandom() {
        meta.random = random.get();
    }

    action send(PortId_t port) {
        ig_tm_md.ucast_egress_port = port;
    }

    action drop() {
        ig_dprsr_md.drop_ctl = 1;
    }

    table simple_fwd {
        key = {
            ig_intr_md.ingress_port : exact;
        }
        actions = {
            send; drop; NoAction;
        }
        const default_action = NoAction();
        size = 64;
    }

    // N表示计数周期，此处N=131072
    Register<bit<32>, _>(1,0x1FFFF) N;

    RegisterAction<bit<32>, _, bit<32>>(N)
    getSub = {
        void apply(inout bit<32> register_data, out bit<32> result) {
            if(register_data == 0x1FFFF){
                register_data = 0x0000;
            }else{
                register_data = register_data + 1;
            }
            // result = (bit<2>)(register_data & 0x3);
            result = register_data;
        }
    };

    // 子周期1：0表示旧数据，1表示新数据
    Register<bit<1>, _>(1) flag1;

    RegisterAction<bit<1>, _, bit<1>>(flag1)
    setFlag1 = {
        void apply(inout bit<1> register_data) {
            register_data = 0x0;
        }
    };

    RegisterAction<bit<1>, _, bit<1>>(flag1)
    revFlag1 = {
        void apply(inout bit<1> register_data) {
            register_data = 0x1;
        }
    };

    // 子周期2：0表示旧数据，1表示新数据
    Register<bit<1>, _>(1) flag2;

    RegisterAction<bit<1>, _, bit<1>>(flag2)
    setFlag2 = {
        void apply(inout bit<1> register_data) {
            register_data = 0x0;
        }
    };

    RegisterAction<bit<1>, _, bit<1>>(flag2)
    revFlag2 = {
        void apply(inout bit<1> register_data) {
            register_data = 0x1;
        }
    };

    // 子周期3：0表示旧数据，1表示新数据
    Register<bit<1>, _>(1) flag3;

    RegisterAction<bit<1>, _, bit<1>>(flag3)
    setFlag3 = {
        void apply(inout bit<1> register_data) {
            register_data = 0x0;
        }
    };

    RegisterAction<bit<1>, _, bit<1>>(flag1)
    revFlag3 = {
        void apply(inout bit<1> register_data) {
            register_data = 0x1;
        }
    };

    // 子周期4：0表示旧数据，1表示新数据
    Register<bit<1>, _>(1) flag4;

    RegisterAction<bit<1>, _, bit<1>>(flag4)
    setFlag4 = {
        void apply(inout bit<1> register_data) {
            register_data = 0x0;
        }
    };

    RegisterAction<bit<1>, _, bit<1>>(flag4)
    revFlag4 = {
        void apply(inout bit<1> register_data) {
            register_data = 0x1;
        }
    };

    Register<bit<16>, _>(80000) sketch11;
    Register<bit<16>, _>(80000) sketch12;
    Register<bit<16>, _>(80000) sketch13;

    Register<bit<16>, _>(80000) sketch21;
    Register<bit<16>, _>(80000) sketch22;
    Register<bit<16>, _>(80000) sketch23;

    Register<bit<16>, _>(80000) sketch31;
    Register<bit<16>, _>(80000) sketch32;
    Register<bit<16>, _>(80000) sketch33;

    Register<bit<16>, _>(80000) sketch41;
    Register<bit<16>, _>(80000) sketch42;
    Register<bit<16>, _>(80000) sketch43;

    RegisterAction<bit<16>, _, bit<1>>(sketch11)
    setValue11_1 = {
        void apply(inout bit<16> value) {
            value = value + 1;
        }
    };

    RegisterAction<bit<16>, _, bit<1>>(sketch11)
    setValue11_0 = {
        void apply(inout bit<16> value) {
            if(value < 0xFF){
                value = value + 1;
            }else{
                value = value;
            }
        }
    };

    RegisterAction<bit<16>, _, bit<1>>(sketch12)
    setValue12_1 = {
        void apply(inout bit<16> value) {
            value = value + 1;
        }
    };

    RegisterAction<bit<16>, _, bit<1>>(sketch12)
    setValue12_0 = {
        void apply(inout bit<16> value) {
            if(value < 0xFF){
                value = value + 1;
            }else{
                value = value;
            }
        }
    };

    RegisterAction<bit<16>, _, bit<1>>(sketch13)
    setValue13_1 = {
        void apply(inout bit<16> value) {
            value = value + 1;
        }
    };

    RegisterAction<bit<16>, _, bit<1>>(sketch13)
    setValue13_0 = {
        void apply(inout bit<16> value) {
            if(value < 0xFF){
                value = value + 1;
            }else{
                value = value;
            }
        }
    };

    RegisterAction<bit<16>, _, bit<1>>(sketch21)
    setValue21_1 = {
        void apply(inout bit<16> value) {
            value = value + 1;
        }
    };

    RegisterAction<bit<16>, _, bit<1>>(sketch21)
    setValue21_0 = {
        void apply(inout bit<16> value) {
            if(value < 0xFF){
                value = value + 1;
            }else{
                value = value;
            }
        }
    };

    RegisterAction<bit<16>, _, bit<1>>(sketch22)
    setValue22_1 = {
        void apply(inout bit<16> value) {
            value = value + 1;
        }
    };

    RegisterAction<bit<16>, _, bit<1>>(sketch22)
    setValue22_0 = {
        void apply(inout bit<16> value) {
            if(value < 0xFF){
                value = value + 1;
            }else{
                value = value;
            }
        }
    };

    RegisterAction<bit<16>, _, bit<1>>(sketch23)
    setValue23_1 = {
        void apply(inout bit<16> value) {
            value = value + 1;
        }
    };

    RegisterAction<bit<16>, _, bit<1>>(sketch23)
    setValue23_0 = {
        void apply(inout bit<16> value) {
            if(value < 0xFF){
                value = value + 1;
            }else{
                value = value;
            }
        }
    };

    RegisterAction<bit<16>, _, bit<1>>(sketch31)
    setValue31_1 = {
        void apply(inout bit<16> value) {
            value = value + 1;
        }
    };

    RegisterAction<bit<16>, _, bit<1>>(sketch31)
    setValue31_0 = {
        void apply(inout bit<16> value) {
            if(value < 0xFF){
                value = value + 1;
            }else{
                value = value;
            }
        }
    };

    RegisterAction<bit<16>, _, bit<1>>(sketch32)
    setValue32_1 = {
        void apply(inout bit<16> value) {
            value = value + 1;
        }
    };

    RegisterAction<bit<16>, _, bit<1>>(sketch32)
    setValue32_0 = {
        void apply(inout bit<16> value) {
            if(value < 0xFF){
                value = value + 1;
            }else{
                value = value;
            }
        }
    };

    RegisterAction<bit<16>, _, bit<1>>(sketch33)
    setValue33_1 = {
        void apply(inout bit<16> value) {
            value = value + 1;
        }
    };

    RegisterAction<bit<16>, _, bit<1>>(sketch33)
    setValue33_0 = {
        void apply(inout bit<16> value) {
            if(value < 0xFF){
                value = value + 1;
            }else{
                value = value;
            }
        }
    };

    RegisterAction<bit<16>, _, bit<1>>(sketch41)
    setValue41_1 = {
        void apply(inout bit<16> value) {
            value = value + 1;
        }
    };

    RegisterAction<bit<16>, _, bit<1>>(sketch41)
    setValue41_0 = {
        void apply(inout bit<16> value) {
            if(value < 0xFF){
                value = value + 1;
            }else{
                value = value;
            }
        }
    };

    RegisterAction<bit<16>, _, bit<1>>(sketch42)
    setValue42_1 = {
        void apply(inout bit<16> value) {
            value = value + 1;
        }
    };

    RegisterAction<bit<16>, _, bit<1>>(sketch42)
    setValue42_0 = {
        void apply(inout bit<16> value) {
            if(value < 0xFF){
                value = value + 1;
            }else{
                value = value;
            }
        }
    };

    RegisterAction<bit<16>, _, bit<1>>(sketch43)
    setValue43_1 = {
        void apply(inout bit<16> value) {
            value = value + 1;
        }
    };

    RegisterAction<bit<16>, _, bit<1>>(sketch43)
    setValue43_0 = {
        void apply(inout bit<16> value) {
            if(value < 0xFF){
                value = value + 1;
            }else{
                value = value;
            }
        }
    };

    apply {
        if (hdr.ipv4.isValid() && (hdr.tcp.isValid() || hdr.udp.isValid())) {
            do_hash1();
            do_hash2();
            do_hash3();
            getrandom();
            simple_fwd.apply();
            bit<32> cycleN = getSub.execute(0);
            bit<2> subN = (bit<2>)(cycleN & 0x03);
            if(subN == 0x0){
                if(cycleN == 0x0000){
                    setFlag4.execute(0);
                    revFlag1.execute(0);
                }
                if(meta.random == 0x11){
                    setValue11_1.execute((bit<16>)meta.index_sketch1);
                    setValue12_1.execute((bit<16>)meta.index_sketch2);
                    setValue13_1.execute((bit<16>)meta.index_sketch3);
                }else{
                    setValue11_0.execute((bit<16>)meta.index_sketch1);
                    setValue12_0.execute((bit<16>)meta.index_sketch2);
                    setValue13_0.execute((bit<16>)meta.index_sketch3);
                }  
            }else if(subN == 0x1){
                if(cycleN == 0x8000){
                    setFlag1.execute(0);
                    revFlag2.execute(0);
                }
                if(meta.random == 0x11){
                    setValue21_1.execute((bit<16>)meta.index_sketch1);
                    setValue22_1.execute((bit<16>)meta.index_sketch2);
                    setValue23_1.execute((bit<16>)meta.index_sketch3);
                }else{
                    setValue21_0.execute((bit<16>)meta.index_sketch1);
                    setValue22_0.execute((bit<16>)meta.index_sketch2);
                    setValue23_0.execute((bit<16>)meta.index_sketch3);
                }
            }else if(subN == 0x2){
                if(cycleN == 0x10000){
                    setFlag2.execute(0);
                    revFlag3.execute(0);
                } 
                if(meta.random == 0x11){
                    setValue31_1.execute((bit<16>)meta.index_sketch1);
                    setValue32_1.execute((bit<16>)meta.index_sketch2);
                    setValue33_1.execute((bit<16>)meta.index_sketch3);
                }else{
                    setValue31_0.execute((bit<16>)meta.index_sketch1);
                    setValue32_0.execute((bit<16>)meta.index_sketch2);
                    setValue33_0.execute((bit<16>)meta.index_sketch3);
                }
            }else if(subN == 0x3){
                if(cycleN == 0x18000){
                    setFlag3.execute(0);
                    revFlag4.execute(0);
                }
                if(meta.random == 0x11){
                    setValue41_1.execute((bit<16>)meta.index_sketch1);
                    setValue42_1.execute((bit<16>)meta.index_sketch2);
                    setValue43_1.execute((bit<16>)meta.index_sketch3);
                }else{
                    setValue41_0.execute((bit<16>)meta.index_sketch1);
                    setValue42_0.execute((bit<16>)meta.index_sketch2);
                    setValue43_0.execute((bit<16>)meta.index_sketch3);
                }
            }
        }else{
            drop();
        }
    }
}

    /*********************  D E P A R S E R  ************************/

control IngressDeparser(
    packet_out pkt,
    /* User */
    inout my_ingress_headers_t hdr,
    in my_ingress_metadata_t meta,
    /* Intrinsic */
    in ingress_intrinsic_metadata_for_deparser_t ig_dprsr_md)
{
    apply {
        pkt.emit(hdr);
    }
}



struct my_egress_headers_t {
    ethernet_h   ethernet;
    ipv4_h       ipv4;
    l4port_h     ports;
    tcp_h        tcp;
    udp_h        udp;
}

struct my_egress_metadata_t {
}

/*************************************************************************
 ****************  E G R E S S   P R O C E S S I N G   *******************
 *************************************************************************/

    /***********************  P A R S E R  **************************/

parser EgressParser(
    packet_in pkt,
    /* User */
    out my_egress_headers_t hdr,
    out my_egress_metadata_t meta,
    /* Intrinsic */
    out egress_intrinsic_metadata_t eg_intr_md)
{
    /* This is a mandatory state, required by Tofino Architecture */
    state start {
        pkt.extract(eg_intr_md);
        transition meta_init;
    }

    state meta_init {
        transition parse_ethernet;
    }

    state parse_ethernet {
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.ether_type) {
            IPV4 :  parse_ipv4;
            default: reject;
        }
    }

    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        transition select(hdr.ipv4.frag_offset, hdr.ipv4.protocol) {
            ( 0, TCP  ) : parse_tcp;
            ( 0, UDP  ) : parse_udp;
            default : reject;
        }
    }

    state parse_tcp {
        pkt.extract(hdr.ports);
        pkt.extract(hdr.tcp);
        transition accept;
    }
    
    state parse_udp {
        pkt.extract(hdr.ports);
        pkt.extract(hdr.udp);
        transition accept;
    }
}

/*************************************************************************
 ****************  E G R E S S   P R O C E S S I N G   *******************
 *************************************************************************/

    /***************** M A T C H - A C T I O N  *********************/

control Egress(
    /* User */
    inout my_egress_headers_t hdr,
    inout my_egress_metadata_t meta,
    /* Intrinsic */    
    in egress_intrinsic_metadata_t eg_intr_md,
    in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
    inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
    inout egress_intrinsic_metadata_for_output_port_t eg_oport_md)
{
    apply {

    }
}


    /*********************  D E P A R S E R  ************************/

control EgressDeparser(
    packet_out pkt,
    /* User */
    inout my_egress_headers_t hdr,
    in my_egress_metadata_t meta,
    /* Intrinsic */
    in egress_intrinsic_metadata_for_deparser_t eg_dprsr_md)
{
    apply {
        pkt.emit(hdr);
    }
}

/************ F I N A L   P A C K A G E ******************************/
Pipeline(
    IngressParser(),
    Ingress(),
    IngressDeparser(),
    EgressParser(),
    Egress(),
    EgressDeparser()
) pipe;

Switch(pipe) main;