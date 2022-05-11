#ifndef __EMSFILE_H__
#define __EMSFILE_H__

// Header (0z30)
// Array of pointers to EMSItem  (array size = item_count)
// Padding to next 0x10 
// EMSItem[item_count]
// Data1 (item)
// odata1 (data_count number of offsets to EMSData1)
// Padding to next 0x10
// array2 (count2 number of offsets to EMSData2)

typedef struct
{
	uint32_t signature; // 0
	uint16_t endianess_check; // 4
	uint16_t unk_06; // 6     observed value 0x30
	uint32_t unk_08; // 8  observed value 0x1
	uint32_t item_count; // 0xC   
	uint32_t unk_10; // 0x10  observed value 0x15
	uint32_t header_size; // 0x14 
	uint32_t data_count; // 0x18
	uint32_t offset_odata1; // 0x1C
	uint32_t offset_odata2; // 0x20
	uint32_t offset_odata3; // 0x24
	uint32_t pad[2]; // 0x28;
	// size 0x30
} __attribute__((packed)) EMSHeader;

typedef struct
{
	uint32_t unk_00; // 0  observed value 0
	uint32_t child_offset; // 4     can be 0
	uint32_t sibling_offset // 8  can be 0
	uint32_t previous_offset; // 0xC NOT 0
	uint32_t data1_offset; // 0x10 NOT 0
	uint32_t id; // 0x14   id or index to something
	uint32_t pad[2]; // 0x18   should be 0
	char name[32]; // 0x20
	// size 0x40
} __attribute__((packed)) EMSItem;

typedef struct
{
	uint32_t unk_00; // maybe some flags
	uint32_t data2_offset; // 4
	uint32_t data3_offset; // 8
	uint32_t data4_offset; // 0xC  can be 0. 
	uint32_t unk_10[2]; // 0x10  two zeros...
	uint32_t id; // 0x18, some id or index...
	uint32_t unk_1C; // 0x1C zero
	uint32_t num_textures; // 0x20
	float unk_24; // 0x24
	float unk_28; // 0x28
	float unk_2C; // 0x2C
	uint32_t unk_30; // 0x30  zero
	char name[32]; // 0x34
	uint32_t pad[3]; // 0x54  zero
	// size 0x60
} __attribute__((packed)) EMSData1;

typedef struct
{
	uint32_t unk_00[2]; // 0   zeroes...
	uint16_t count_subdata1; // 4   1 seen, watch for a different value
	uint16_t count_subdata2 // 6   1 seen, watch for a different value
	uint32_t unk_0C[3]; // 0x0C  3 zeroes...
	//
	// offsets_subdata1[count_subdata1]
	// offsets_subdata2[count_subdata2]
	
	// size variable
} __attribute__((packed)) EMSData2;

typedef struct
{
	uint32_t unk_00[4]; // Apparently 4 zeroes...
} __attribute__((packed)) EMSSubData1;

typedef struct
{
	uint32_t unk_00[2]; // 0 Apparently 2 zeroes
	float unk_08[3]; // 8
	uint32_t unk_14[3]; // 0x14 Apprently 3 zeroes
} __attribute__((packed)) EMSSubData2;

typedef struct
{
	// size variable
} __attribute__((packed)) EMSData3;

typedef struct
{
	uint32_t unk_00; // Some number, non zero
	uint32_t unk_04[11]; // 4   apparently all 0
	// size 0x30
} __attribute__((packed)) EMSData4;

#endif
