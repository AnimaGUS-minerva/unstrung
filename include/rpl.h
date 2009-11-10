#ifndef _RPL_H_

#define ND_RPL_MESSAGE 0x9B

enum ND_RPL_CODE {
        ND_RPL_DAG_IS=0x01,
        ND_RPL_DAG_IO=0x02,
        ND_RPL_DAO   =0x04
};

enum ND_RPL_DIO_FLAGS {
        ND_RPL_DIO_GROUNDED = 0x80,
        ND_RPL_DIO_DATRIG   = 0x40,
        ND_RPL_DIO_DASUPPORT= 0x20,
        ND_RPL_DIO_RES4     = 0x10,
        ND_RPL_DIO_RES3     = 0x08,
        ND_RPL_DIO_PRF_MASK = 0x07,  /* 3-bit preference */
};

struct nd_rpl_dio {
        u_int8_t rpl_flags;
        u_int8_t rpl_seq;
        u_int8_t rpl_instanceid;
        u_int8_t rpl_dagrank;
        u_int8_t rpl_dagid[16];
};

        


#define _RPL_H_
#endif /* _RPL_H_ */
