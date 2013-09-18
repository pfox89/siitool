/* sii.h
 *
 * 2013-09-04 Frank Jeschke <fjeschke@synapticon.com>
 */

#ifndef SII_H
#define SII_H

#include <unistd.h>
#include <stdint.h>

#define SII_VERSION_MAJOR  0
#define SII_VERSION_MINOR  0

#define MBOX_EOE    0x0002
#define MBOX_COE    0x0004
#define MBOX_FOE    0x0008
#define MBOX_SOE    0x0010
#define MBOX_VOE    0x0020

enum eSection {
	SII_CAT_NOP
	,SII_PREAMBLE
	,SII_STD_CONFIG
	,SII_CAT_STRINGS = 10
	,SII_CAT_DATATYPES = 20 /* future use */
	,SII_CAT_GENERAL = 30
	,SII_CAT_FMMU = 40
	,SII_CAT_SYNCM = 41
	,SII_CAT_TXPDO = 50
	,SII_CAT_RXPDO = 51
	,SII_CAT_DCLOCK = 60/* future use */
	,SII_END = 0xff
};

struct _sii_preamble {
	uint16_t pdi_ctrl;
	uint16_t pdi_conf;
	uint16_t sync_impulse;
	uint16_t pdi_conf2;
	uint16_t alias;
	uint8_t reserved[4]; /* shall be zero */
	uint16_t checksum; /* checksum with x^8 + x^2 + x^1 + 1; initial value 0xff */
};

struct _sii_stdconfig {
	uint32_t vendor_id;
	uint32_t product_id;
	uint32_t revision_id;
	uint32_t serial;
	uint8_t reserveda[8]; /* shall be zero */
	/* bootstrap mailbox settings */
	uint16_t bs_rec_mbox_offset;
	uint16_t bs_rec_mbox_size;
	uint16_t bs_snd_mbox_offset;
	uint16_t bs_snd_mbox_size;
	/* standard mailbox settings */
	uint16_t std_rec_mbox_offset;
	uint16_t std_rec_mbox_size;
	uint16_t std_snd_mbox_offset;
	uint16_t std_snd_mbox_size;
	//uint16_t mailbox_protocol;  /* bitmask of supported mailbox protocols */
	union {
		uint16_t word;
		struct {
			uint16_t res1:1;
			uint16_t eoe:1;
			uint16_t coe:1;
			uint16_t foe:1;
			uint16_t soe:1;
			uint16_t voe:1;
			uint16_t res2:10;
		} bit;
	} mailbox_protocol;
	uint8_t reservedb[66]; /* shall be zero */
	uint16_t eeprom_size;
	uint16_t version;
};

struct _sii_general {
	uint8_t groupindex; /* index to string in string section */
	uint8_t imageindex;
	uint8_t orderindex;
	uint8_t nameindex;
	/* CoE Details: each 0 - not enabled, 1 enabled */
	uint8_t coe_enable_sdo;
	uint8_t coe_enable_sdo_info;
	uint8_t coe_enable_pdo_assign;
	uint8_t coe_enable_pdo_conf;
	uint8_t coe_enable_upload_start;
	uint8_t coe_enable_sdo_complete;
	uint8_t foe_enabled;
	uint8_t eoe_enabled;
	uint8_t flag_safe_op:1;
	uint8_t flag_notLRW:1;
	uint8_t reserved:6;
	int16_t current_ebus;
	/* physical ports
	 * values: 0x00  not used
	 *         0x01  MII
	 *         0x02  reserved
	 *         0x03  EBUS
	 */
	uint16_t phys_port_0:4;
	uint16_t phys_port_1:4;
	uint16_t phys_port_2:4;
	uint16_t phys_port_3:4;
	uint8_t reservedb[14]; /* shall be zero */
};

struct _fmmu_entry {
	uint8_t usage;
	/* no content of sii entry */
	int id;
	struct _fmmu_entry *next;
	struct _fmmu_entry *prev;
};

struct _sii_fmmu {
	int count;
	struct _fmmu_entry *list;
};

struct _syncm_entry {
	uint16_t phys_address;
	uint16_t length;
	uint8_t control;
	uint8_t status; /* don't care */
	uint8_t enable;
	uint8_t type;
	/* no content of sii entry */
	int id;
	struct _syncm_entry *next;
	struct _syncm_entry *prev;
};

struct _sii_syncm {
	int count;
	struct _syncm_entry *list;
};

#define SII_UNDEF_PDO  0
#define SII_RX_PDO     1
#define SII_TX_PDO     2

struct _pdo_entry {
	uint16_t index;
	uint8_t subindex;
	uint8_t string_index;
	uint8_t data_type;
	uint8_t bit_length;
	uint32_t flags; /* for future use - set to 0 */
	/* no content of sii entry */
	int count;
	struct _pdo_entry *next;
	struct _pdo_entry *prev;
};

struct _sii_pdo {
	uint16_t index;
	uint8_t entries;
	uint8_t syncmanager; /* related syncmanager */
	uint8_t dcsync;
	uint8_t name_index; /* index to string */
	uint16_t flags; /* for future use - set to 0 */
	/* no content of sii entry */
	int type; /* rx or tx pdo */
	struct _pdo_entry *list;
	struct _sii_pdo *next;
	struct _sii_pdo *prev;
};

struct _sii_pdo_list {
	int count;
	struct _sii_pdo *head;
	struct _sii_pdo *current;
};

struct _sii_dclock {
	uint16_t reserved1:8; /* shall be zero */
	uint16_t cyclic_op_enabled:1;
	uint16_t sync0_active:1;
	uint16_t sync1_active:1;
	uint16_t reserved2:5;
	uint16_t sync_pulse;
	uint8_t reserved3[10];
	uint8_t int0_status:1;
	uint8_t reserved4:7;
	uint8_t int1_status:1;
	uint8_t reserved5:7;
	uint32_t cyclic_op_starttime;
	uint32_t sync0_cycle_time;
	uint32_t sync1_cycle_time;
	uint16_t latch0_pos_edge:1;
	uint16_t latch0_neg_edge:1;
	uint16_t reserved6:14;
	uint16_t latch1_pos_edge:1;
	uint16_t latch1_neg_edge:1;
	uint16_t reserved7:14;
	uint8_t reserved8[4];
	uint8_t latch0_pos_event:1;
	uint8_t latch0_neg_event:1;
	uint8_t reserved9:6;
	uint8_t latch1_pos_event:1;
	uint8_t latch1_neg_event:1;
	uint8_t reserved10:6;
	uint32_t latch0_pos_edge_value;
	uint8_t reserved11[4];
	uint32_t latch0_neg_edge_value;
	uint8_t reserved12[4];
	uint32_t latch1_pos_edge_value;
	uint8_t reserved13[4];
	uint32_t latch1_neg_edge_value;
	uint8_t reserved14[4];
};

#if 0 /* obsoleted by struct sii */
typedef struct _sii_info {
	struct _sii_preamble *preamble;
	struct _sii_stdconfig *stdconfig;
	char **strings; /* array of strings from string section */
	/* struct _sii_datatypes; -- not yet available */
	struct _sii_general *general;
	struct _sii_fmmu *fmmu; /* contains linked list with each fmmu */
	struct _sii_syncm *syncmanager; /* contains linked list with each syncmanager */
	struct _sii_pdo_list *pdolist; /* contains list of pdoentries */
	struct _sii_dclock *distributedclock;
} SiiInfo;
#endif

/* a single category */
struct _sii_cat {
	uint16_t type:15;
	uint16_t vendor:1;
	uint16_t size;
	void *data;
	struct _sii_cat *next;
};

struct _sii {
	struct _sii_preamble preamble;
	struct _sii_stdconfig config;
	struct _sii_cat *head;
	struct _sii_cat *current;
	/* meta information */
	uint8_t *rawbytes;
	int rawvalid;
};

typedef struct _sii SiiInfo;

/**
 * @brief Init a empty data structure
 *
 * @return empty sii object
 */
SiiInfo *sii_init(void);

SiiInfo *sii_init_string(const unsigned char *eeprom, size_t size);
SiiInfo *sii_init_file(const char *filename);
void sii_release(SiiInfo *sii);

/**
 * \brief Generate binary sii file
 *
 * \param *sii  pointer to sii structure
 * \param outfile  output filename
 * \return 0 success; otherwise failure
 */
int sii_generate(SiiInfo *sii, const char *outfile);

/**
 * \brief Sanity check of current config
 */
int sii_check(SiiInfo *sii);

#endif /* SII_H */