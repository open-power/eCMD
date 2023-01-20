//IBM_PROLOG_BEGIN_TAG
/* 
 * Copyright 2003,2017 IBM International Business Machines Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//IBM_PROLOG_END_TAG


//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <PNORInstruction.H>
#include <arpa/inet.h>
#include <iomanip>
#include <sstream>
#include <algorithm>

#ifdef CRONUS_SERVER_SIDE
#if !defined(NO_GFW)
  #include <libffs2.h>
#endif
#endif

#ifdef OTHER_USE
#include <OutputLite.H>
extern OutputLite out;
#else
#include <CronusData.H>
#endif

extern bool global_server_debug;

/*****************************************************************************/
/* PNORInstruction Implementation *********************************************/
/*****************************************************************************/
PNORInstruction::PNORInstruction(void) : Instruction(),
deviceString(""),
partitionEntry(""),
partitionOffset(0),
blockSize(0),
pnorFlags(0)
{
  version = 0x1;
  type = PNOR;
}

PNORInstruction::PNORInstruction(InstructionCommand i_command,
                               uint32_t i_partitionOffset, uint32_t i_blockSize,
                               std::string i_partitionEntry, std::string i_deviceString,
                               uint32_t i_pnorFlags,
                               uint32_t i_flags, ecmdDataBuffer * i_data) : Instruction(),
partitionOffset(i_partitionOffset),
blockSize(i_blockSize),
pnorFlags(i_pnorFlags)
{
    version = 0x1;
    type = PNOR;
   
    flags = i_flags;
    command = i_command;
 
    partitionEntry = i_partitionEntry;
    deviceString = i_deviceString;

    if(i_data != NULL) {
        i_data->shareBuffer(&data);
    }
}

PNORInstruction::~PNORInstruction(void) 
{
}

uint32_t PNORInstruction::setup(InstructionCommand i_command,
                               uint32_t i_partitionOffset, uint32_t i_blockSize,
                               std::string i_partitionEntry, std::string i_deviceString,
                               uint32_t i_pnorFlags,
                               uint32_t i_flags, ecmdDataBuffer * i_data) 
{
    command = i_command;
    flags = i_flags;
    partitionOffset = i_partitionOffset;
    blockSize = i_blockSize;
    pnorFlags = i_pnorFlags;

    partitionEntry = i_partitionEntry;
    deviceString = i_deviceString;

    if(i_data != NULL) 
    {
        i_data->shareBuffer(&data);
    }
    version = 0x1;
    return 0;
}

uint32_t PNORInstruction::execute(ecmdDataBuffer & o_data, InstructionStatus & o_status, Handle ** io_handle) {
    uint32_t rc = 0;

    /* set the version of this instruction in the status */
    o_status.instructionVersion = version;

#if defined(CRONUS_SERVER_SIDE)
    /* check for any previous errors to report back */
    if (error) {
        rc = o_status.rc = error;
        return rc;
    }

    switch(command) 
    {
        /*
         * GET LIST OPERATION
         */
        case PNORGETLIST:
        {
#if defined(TESTING)
            printf("PNORGETLIST\n");
#endif
            std::string ffsDevice;
            ffsDevice = "/dev/mtdblock/sfc." + deviceString;

#if defined(TESTING)
            printf("ffsData = ffs_open( \"%s\", 0x%X );\n", ffsDevice.c_str(), partitionOffset );
#else
 #if !defined(NO_GFW)
            off_t ffsDeviceOffset = partitionOffset;
            ffs_t *ffs = NULL;
            ffs = ffs_open( ffsDevice.c_str(), ffsDeviceOffset );

            if ( ffs == NULL )
            {
                std::ostringstream osse;
                osse << "PNORInstruction::execute SERVER DEBUG: Unable to open device " << ffsDevice << std::endl;
                o_status.errorMessage.append(osse.str());
                osse.clear(); osse.str(std::string());
                osse << "PNORInstruction::execute SERVER DEBUG: Listing partitions in image failed" << std::endl;
                o_status.errorMessage.append(osse.str());
                rc = o_status.rc = SERVER_PNOR_DEVICE_FAIL_OPEN;
                break;
            }

            ffs_hdr_t *ffsHdr = ffs->hdr;
 #endif
#endif

            Pnor_ffs_hdr *objHolder = new Pnor_ffs_hdr();

#if defined(TESTING)
            printf("transposing ffsData to internal Pnor_ffs_hdr object holder\n");

            objHolder->magic = 0x1;
            objHolder->version = 0x2;
            objHolder->size = 1;
            objHolder->entry_size = 0x80;
            objHolder->entry_count = 2;
            objHolder->block_size = 0x1000;
            objHolder->block_count = 0x4000;
            objHolder->checksum = 0x3;
            Pnor_ffs_entry *objEntry = new Pnor_ffs_entry();
            objEntry->base = 0x1fff;
            objEntry->size = 4096;
            objEntry->pid = 0;
            objEntry->id = 3;
            objEntry->type = 1;
            objEntry->flags = 0x0;
            objEntry->actual = 4096;
            objEntry->checksum = 0x43;
            objEntry->name = "HBBtest";
            (objHolder->entries).push_back(objEntry);
            objEntry = new Pnor_ffs_entry();
            objEntry->base = 0x0fff;
            objEntry->size = 8192;
            objEntry->pid = 0;
            objEntry->id = 2;
            objEntry->type = 3;
            objEntry->flags = 0x1;
            objEntry->actual = 8192;
            objEntry->checksum = 0x51;
            objEntry->name = "PNORtest";
            (objHolder->entries).push_back(objEntry);
#else
 #if !defined(NO_GFW)
            objHolder->magic = ffsHdr->magic;
            objHolder->version = ffsHdr->version;
            objHolder->size = ffsHdr->size;
            objHolder->entry_size = ffsHdr->entry_size;
            objHolder->entry_count = ffsHdr->entry_count;
            objHolder->block_size = ffsHdr->block_size;
            objHolder->block_count = ffsHdr->block_count;
            objHolder->checksum = ffsHdr->checksum;

            for( uint32_t idx = 0; idx < objHolder->entry_count; idx++ )
            {
                Pnor_ffs_entry *objEntry = new Pnor_ffs_entry();
                ffs_entry *ffsEntry = &(ffsHdr->entries[idx]);

                objEntry->base = ffsEntry->base;
                objEntry->size = ffsEntry->size;
                objEntry->pid = ffsEntry->pid;
                objEntry->id = ffsEntry->id;
                objEntry->type = ffsEntry->type;
                objEntry->flags = ffsEntry->flags;
                objEntry->actual = ffsEntry->actual;
                objEntry->checksum = ffsEntry->checksum;
                objEntry->name = ffsEntry->name;

                (objHolder->entries).push_back(objEntry);
            }
 #endif
#endif

            uint32_t flattensize = objHolder->flattenSize();
            uint32_t *flattenedffsData = new uint32_t[flattensize];
            rc = objHolder->flatten( (uint8_t*) flattenedffsData, flattensize );

            if ( rc ) 
            {
                std::ostringstream osse;
                osse << "PNORInstruction::execute - flattenffsData failure" << std::endl;
                o_status.errorMessage.append(osse.str());
                rc = o_status.rc = SERVER_PNOR_ERROR;
                return rc;
            }
#if defined(TESTING)
            printf("Flattened objHolder of size %d\n", flattensize);
#endif

            rc = o_data.setWordLength( flattensize );
            if ( rc ) 
            {
                std::ostringstream osse;
                osse << "PNORInstruction::execute - o_data.setWordLength(" << flattensize << ") failure" << std::endl;
                o_status.errorMessage.append(osse.str());
                rc = o_status.rc = SERVER_PNOR_ERROR;
                return rc;
            }
#if defined(TESTING)
            printf("o_data.getBitLength is %d\n", o_data.getBitLength());
#endif

            rc = o_data.insert( (uint8_t *)flattenedffsData, 0, (flattensize * 32), 0 );
            if ( rc ) 
            {
                std::ostringstream osse;
                osse << "PNORInstruction::execute - o_data.insert failure" << std::endl;
                o_status.errorMessage.append(osse.str());
                rc = o_status.rc = SERVER_PNOR_ERROR;
                return rc;
            }
#if defined(TESTING)
            printf("o_data.insert completed\n");
            printf("\n%s\n", o_data.genHexLeftStr().c_str() );
#else
 #if !defined(NO_GFW)
            rc = ffs_close( ffs );
            if ( rc ) 
            {
                std::ostringstream osse;
                osse << "PNORInstruction::execute(" << InstructionCommandToString( command )<< ") - ffs_close failure rc(" << rc << ")" << std::endl;
                o_status.errorMessage.append(osse.str());
                rc = o_status.rc = SERVER_PNOR_DEVICE_FAIL_CLOSE;
                break;
            }
 #endif
#endif
            delete [] flattenedffsData;
            delete objHolder;

            rc = o_status.rc = SERVER_COMMAND_COMPLETE;
            break;
        }

        /*
         * GET OPERATION
         */
        case PNORGET:
        {
#if defined(TESTING)
            printf("PNORGET\n");
#endif

            std::string ffsDevice;
            ffsDevice = "/dev/mtdblock/sfc." + deviceString;

#if defined(TESTING)
            printf("ffsData = ffs_open( \"%s\", 0x%X );\n", ffsDevice.c_str(), partitionOffset );
#else
 #if !defined(NO_GFW)
            off_t ffsDeviceOffset = partitionOffset;
            ffs_t *ffsData = NULL;
            ffsData = ffs_open( ffsDevice.c_str(), ffsDeviceOffset );

            if ( ffsData == NULL )
            {
                std::ostringstream osse;
                osse << "PNORInstruction::execute SERVER DEBUG: Unable to open device " << ffsDevice << std::endl;
                o_status.errorMessage.append(osse.str());
                osse.clear(); osse.str(std::string());
                osse << "PNORInstruction::execute SERVER DEBUG: Listing partitions in image failed" << std::endl;
                o_status.errorMessage.append(osse.str());
                rc = o_status.rc = SERVER_PNOR_DEVICE_FAIL_OPEN;
                break;
            }
 #endif
#endif

#if defined(TESTING)
            printf("iterate over ffsData\n");
#else
 #if !defined(NO_GFW)
            //partitionEntry must match a partition entry within the ffsData, find it
            ffs_hdr_t *ffsHdr = ffsData->hdr;
            ffs_entry *myFoundEntry = NULL;
            for( uint32_t idx = 0; idx < ffsHdr->entry_count; idx++ )
            {
                ffs_entry *ffsEntry = &(ffsHdr->entries[idx]);
                std::string entryName = ffsEntry->name;
                if ( entryName == partitionEntry )
                {
                    myFoundEntry = ffsEntry;
                    break;
                }
            }
            
            if ( myFoundEntry == NULL )
            {
                std::ostringstream osse;
                osse << "PNORInstruction::execute Unable to find a matching partition with name " << partitionEntry << std::endl;
                o_status.errorMessage.append(osse.str());
                rc = o_status.rc = SERVER_PNOR_PARTITION_NOT_FOUND;
                break;
            }
 #endif
#endif

#if defined(TESTING)
            printf("matched partition, now get data\n");
#else
 #if !defined(NO_GFW)
            // myFoundEntry points to the specific entry we want to read out
            if (global_server_debug) printf("myFoundEntry->type=%d, myFoundEntry->checksum=%.16X\n", myFoundEntry->type, myFoundEntry->checksum );
            if ( myFoundEntry->type == FFS_TYPE_LOGICAL )
            {
                std::ostringstream osse;
                osse << "PNORInstruction::execute Unable to get data from a logical partition for name " << partitionEntry << std::endl;
                o_status.errorMessage.append(osse.str());
                rc = o_status.rc = SERVER_PNOR_PARTITION_TYPE_INVALID;
                break;
            }
            else if ( myFoundEntry->actual == 0 )
            {
                std::ostringstream osse;
                osse << "PNORInstruction::execute Size of partition is 0, not reading anything out for name " << partitionEntry << " size(" << myFoundEntry->actual << ")" << std::endl;
                o_status.errorMessage.append(osse.str());
                rc = o_status.rc = SERVER_PNOR_PARTITION_EMPTY;
                break;
            }
            else
            {
                uint32_t bufferSize = ffsHdr->block_size * ffsData->buf_count;
                uint32_t *buffer = new uint32_t[bufferSize];
                // uses partitionOffset
                uint32_t total = 0;
                uint32_t size = myFoundEntry->actual;
                off_t offset = 0;

                if (global_server_debug) printf("bufferSize(%d), total(%d), size(%d), offset(%d)\n", bufferSize, total, size, (uint32_t) offset);

                while ( 0 < size )
                {
                    size_t count = std::min( bufferSize, size );

                    if (global_server_debug) printf("name(%s), offset(%d), count(%d), size(%d), total(%d)\n", partitionEntry.c_str(), (uint32_t) offset, count, size, total);

                    uint32_t bytesRead = ffs_entry_read( ffsData, partitionEntry.c_str(), buffer, offset, count ); 

                    if (global_server_debug) printf("post ffs_entry_read bytesRead=%d\n", bytesRead);
                    if ( bytesRead < 0 ) 
                    {
                        std::ostringstream osse;
                        osse << "PNORInstruction::execute failure in ffs_entry_read " << partitionEntry << " offset(" << offset << ") count(" << count << ")" << std::endl;
                        o_status.errorMessage.append(osse.str());
                        rc = o_status.rc = SERVER_PNOR_READ_ERROR;
                        break;
                    }
                    else if ( bytesRead != count )
                    {
                        std::ostringstream osse;
                        osse << "PNORInstruction::execute failure to read enough data from partition " << partitionEntry << " expected(" << count << ") bytesRead(" << bytesRead << ")" << std::endl;
                        o_status.errorMessage.append(osse.str());
                        rc = o_status.rc = SERVER_PNOR_READ_ERROR;
                        break;
                    }

                    o_data.growBitLength( o_data.getBitLength() + (count*32) );
                    o_data.insert( buffer, total*32, count*32, 0 );

                    size -= count; 
                    total += count;
                    offset += count;
                }

                // always attempt to close and only fallout on close if it fails
                // otherwise fallout on rc generated earlier
                rc = ffs_close( ffsData );
                if (global_server_debug) printf("post ffs_close rc=%d\n", rc);
                if ( rc ) 
                {
                    std::ostringstream osse;
                    osse << "PNORInstruction::execute(" << InstructionCommandToString( command )<< ") - ffs_close failure rc(" << rc << ")" << std::endl;
                    o_status.errorMessage.append(osse.str());
                    rc = o_status.rc = SERVER_PNOR_DEVICE_FAIL_CLOSE;
                    break;
                }
            }
 #endif
#endif
            // make sure to set rc to COMMAND_COMPLETE if not already set
            // otherwise leave rc as is so it can be returned
            if ( rc == 0 )
            {
                rc = o_status.rc = SERVER_COMMAND_COMPLETE;
            }
            break;
        }

        /*
         * PUT OPERATION
         */
        case PNORPUT:
        {
#if defined(TESTING)
            printf("PNORPUT\n");
#endif

            std::string ffsDevice;
            ffsDevice = "/dev/mtdblock/sfc." + deviceString;

#if defined(TESTING)
            printf("ffsData = ffs_open( \"%s\", 0x%X );\n", ffsDevice.c_str(), partitionOffset );
#else
 #if !defined(NO_GFW)
            off_t ffsDeviceOffset = partitionOffset;
            ffs_t *ffsData = NULL;
            ffsData = ffs_open( ffsDevice.c_str(), ffsDeviceOffset );

            if ( ffsData == NULL )
            {
                std::ostringstream osse;
                osse << "PNORInstruction::execute SERVER DEBUG: Unable to open device " << ffsDevice << std::endl;
                o_status.errorMessage.append(osse.str());
                osse.clear(); osse.str(std::string());
                osse << "PNORInstruction::execute SERVER DEBUG: Listing partitions in image failed" << std::endl;
                o_status.errorMessage.append(osse.str());
                rc = o_status.rc = SERVER_PNOR_DEVICE_FAIL_OPEN;
                break;
            }
 #endif
#endif

#if defined(TESTING)
            printf("iterate over ffsData\n");
#else
 #if !defined(NO_GFW)
            //partitionEntry must match a partition entry within the ffsData, find it
            ffs_hdr_t *ffsHdr = ffsData->hdr;
            ffs_entry *myFoundEntry = NULL;
            for( uint32_t idx = 0; idx < ffsHdr->entry_count; idx++ )
            {
                ffs_entry *ffsEntry = &(ffsHdr->entries[idx]);
                std::string entryName = ffsEntry->name;
                if ( entryName == partitionEntry )
                {
                    myFoundEntry = ffsEntry;
                    break;
                }
            }
            
            if ( myFoundEntry == NULL )
            {
                std::ostringstream osse;
                osse << "PNORInstruction::execute Unable to find a matching partition with name " << partitionEntry << std::endl;
                o_status.errorMessage.append(osse.str());
                rc = o_status.rc = SERVER_PNOR_PARTITION_NOT_FOUND;
                break;
            }
 #endif
#endif

#if defined(TESTING)
            printf("matched partition, now get data\n");
#else
 #if !defined(NO_GFW)
            // myFoundEntry points to the specific entry we want to read out
            if (global_server_debug) printf("myFoundEntry->type=%d, myFoundEntry->checksum=%.16X\n", myFoundEntry->type, myFoundEntry->checksum );
            if ( myFoundEntry->type == FFS_TYPE_LOGICAL )
            {
                std::ostringstream osse;
                osse << "PNORInstruction::execute Unable to get data from a logical partition for name " << partitionEntry << std::endl;
                o_status.errorMessage.append(osse.str());
                rc = o_status.rc = SERVER_PNOR_PARTITION_TYPE_INVALID;
                break;
            }
            else if ( myFoundEntry->actual == 0 )
            {
                std::ostringstream osse;
                osse << "PNORInstruction::execute Size of partition is 0, not reading anything out for name " << partitionEntry << " size(" << myFoundEntry->actual << ")" << std::endl;
                o_status.errorMessage.append(osse.str());
                rc = o_status.rc = SERVER_PNOR_PARTITION_EMPTY;
                break;
            }
            else
            {
                uint32_t bufferSize = ffsHdr->block_size * ffsData->buf_count;
                if ( blockSize != 0 )
                {
                    bufferSize = blockSize;
                }
                uint32_t *buffer = new uint32_t[bufferSize];
                
                uint32_t total = 0;
                uint32_t size = myFoundEntry->actual;
                off_t offset = 0;   

                if (global_server_debug) printf("bufferSize(%d), total(%d), size(%d), offset(%d), o_data.getWordLength()(%d)\n", bufferSize, total, size, (uint32_t) offset, o_data.getWordLength());

                if ( pnorFlags & INSTRUCTION_PNOR_FLAG_ERASE_PARTITION )
                {
                    // get first word from o_data (most likely 0)
                    uint32_t myWord = o_data.getWord(0);
                    // set the data buffer to the size of the partition
                    o_data.setWordLength(size);
                    // set each word in the buffer to myWord
                    for( uint32_t idx=0; idx < size; idx++ )
                    {
                        o_data.setWord( idx, myWord );
                    }
                    if (global_server_debug) printf("erasing buffer with value (%d) for each word\n", myWord);
                } 

                if ( o_data.getWordLength() != size )
                {
                    std::ostringstream osse;
                    osse << "PNORInstruction::execute size mismatch for " << partitionEntry << " size(" << size << ") is not equal to input size(" << o_data.getWordLength() << ")" << std::endl;
                    o_status.errorMessage.append(osse.str());
                    rc = o_status.rc = SERVER_PNOR_WRITE_ERROR;
                    break;
                }

                while ( 0 < size )
                {
                    size_t count = std::min( bufferSize, size );

                    if (global_server_debug) printf("name(%s), offset(%d), count(%d), size(%d), total(%d)\n", partitionEntry.c_str(), (uint32_t) offset, count, size, total);

                    // copy data from the ecmdDataBuffer into buffer for count (uint32_t's)
                    memset( buffer, 0x0, bufferSize );
                    o_data.extract( buffer, total * 32, count * 32 );

                    uint32_t bytesWritten = ffs_entry_write( ffsData, partitionEntry.c_str(), buffer, offset, count ); 

                    uint32_t fsyncRc = ffs_fsync( ffsData );

                    if (global_server_debug) printf("post ffs_entry_write bytesWritten=%d\n", bytesWritten);
                    if ( bytesWritten < 0 ) 
                    {
                        std::ostringstream osse;
                        osse << "PNORInstruction::execute failure in ffs_entry_write " << partitionEntry << " offset(" << offset << ") count(" << count << ")" << std::endl;
                        o_status.errorMessage.append(osse.str());
                        rc = o_status.rc = SERVER_PNOR_WRITE_ERROR;
                        break;
                    }
                    else if ( bytesWritten != count )
                    {
                        std::ostringstream osse;
                        osse << "PNORInstruction::execute failure to read enough data from partition " << partitionEntry << " expected(" << count << ") bytesWritten(" << bytesWritten << ")" << std::endl;
                        o_status.errorMessage.append(osse.str());
                        rc = o_status.rc = SERVER_PNOR_WRITE_ERROR;
                        break;
                    }
                    if ( fsyncRc < 0 )
                    {
                        std::ostringstream osse;
                        osse << "PNORInstruction::execute failure in ffs_fsync " << partitionEntry << " fsyncRc(" << fsyncRc << ")" << std::endl;
                        o_status.errorMessage.append(osse.str());
                        rc = o_status.rc = SERVER_PNOR_FSYNC_ERROR;
                        break;
                    }
                    
                    size -= count;
                    total += count;
                    offset += count;
                }
                // always attempt to close and only fallout on close if it fails
                // otherwise fallout on rc generated earlier
                rc = ffs_close( ffsData );
                if (global_server_debug) printf("post ffs_close rc=%d\n", rc);
                if ( rc ) 
                {
                    std::ostringstream osse;
                    osse << "PNORInstruction::execute(" << InstructionCommandToString( command )<< ") - ffs_close failure rc(" << rc << ")" << std::endl;
                    o_status.errorMessage.append(osse.str());
                    rc = o_status.rc = SERVER_PNOR_DEVICE_FAIL_CLOSE;
                    break;
                }
            } 
 #endif
#endif

            // make sure to set rc to COMMAND_COMPLETE if not already set
            // otherwise leave rc as is so it can be returned
            if ( rc == 0 )
            {
                rc = o_status.rc = SERVER_COMMAND_COMPLETE;
            }
            break;
        }
        default:
            rc = o_status.rc = SERVER_COMMAND_NOT_SUPPORTED;
            break;
    }

#endif // CRONUS_SERVER_SIDE

  return rc;
}

uint32_t PNORInstruction::flatten(uint8_t * o_data, uint32_t i_len) const {
    uint32_t rc = 0;
    uint32_t * o_ptr = (uint32_t *) o_data;
    
    if (i_len < flattenSize()) {
        out.error("PNORInstruction::flatten", "i_len %d bytes is too small to flatten\n", i_len);
        rc = 1;
    } else {
        // clear memory
        memset(o_data, 0, flattenSize());
        o_ptr[0] = htonl(version);
        o_ptr[1] = htonl(command);
        o_ptr[2] = htonl(type);
        o_ptr[3] = htonl(flags);
        o_ptr[4] = htonl(partitionOffset);
        o_ptr[5] = htonl(blockSize);
        o_ptr[6] = htonl(pnorFlags);
        uint32_t dataSize = data.flattenSize();
        o_ptr[7] = htonl(dataSize);
        uint32_t deviceStringSize = deviceString.size() + 1;
        o_ptr[8] = htonl(deviceStringSize);
        uint32_t partitionEntrySize = partitionEntry.size() + 1;
        o_ptr[9] = htonl(partitionEntrySize);
        data.flatten((uint8_t *) (o_ptr + 10), dataSize);
        if (deviceString.size() > 0) 
        {
            strcpy( ((char *)(o_ptr + 10)) + dataSize, deviceString.c_str() );
        } 
        else 
        {
            memset( ((char *)(o_ptr + 10)) + dataSize, 0x0, 1 );
        }
        if (partitionEntry.size() > 0) 
        {
            strcpy( ((char *)(o_ptr + 10)) + dataSize + deviceStringSize, partitionEntry.c_str() );
        } 
        else 
        {
            memset( ((char *)(o_ptr + 10)) + dataSize + deviceStringSize, 0x0, 1 );
        }
    }
    return rc;
}

uint32_t PNORInstruction::unflatten(const uint8_t * i_data, uint32_t i_len) {
    uint32_t rc = 0;
    uint32_t * i_ptr = (uint32_t *) i_data;

    version = ntohl(i_ptr[0]);
    if( (version == 0x1) ) {
        command = (InstructionCommand) ntohl(i_ptr[1]);
        type = (InstructionType) ntohl(i_ptr[2]);
        flags = ntohl(i_ptr[3]);
        partitionOffset = ntohl(i_ptr[4]);
        blockSize = ntohl(i_ptr[5]);
        pnorFlags = ntohl(i_ptr[6]);
        uint32_t dataSize = ntohl(i_ptr[7]);
        uint32_t deviceStringSize = ntohl(i_ptr[8]);
        rc = data.unflatten((uint8_t *) (i_ptr + 10), dataSize);
        if (rc) {
            error = rc;
        }
        deviceString = ((char *)(i_ptr + 10)) + dataSize;
        partitionEntry = ((char *)(i_ptr + 10)) + dataSize + deviceStringSize;
    } else {
        error = rc = SERVER_UNKNOWN_INSTRUCTION_VERSION;
    }
    return rc;
}

uint32_t PNORInstruction::flattenSize(void) const {
    return (3 * sizeof(uint32_t)) + //all uint32_t vars to un/flatten part of this class
                                    //deviceSize, partitionOffset, blockSize
                                    //partitionEntrySize, partitionEntryOffset, dataFormat
           (3 * sizeof(uint32_t)) + //size holder for deviceString, partitionEntry and data
           (4 * sizeof(uint32_t)) + //all uint32_t vars to un/flatten part of the parent
                                    //version, command, type, flags
           1 + deviceString.size() + //size of string followed by deviceString
           1 + partitionEntry.size() + // size of string followed by partitionEntry
           data.flattenSize();
}

std::string PNORInstruction::dumpInstruction(void) const {
    std::ostringstream oss;
    oss << "PNORInstruction" << std::endl;
    oss << "version       : " << version << std::endl;
    oss << "command       : " << InstructionCommandToString(command) << std::endl;
    oss << "type          : " << InstructionTypeToString(type) << std::endl;
    oss << "flags         : " << InstructionFlagToString(flags) << std::endl;
    oss << "device        : " << deviceString << std::endl;
    oss << "partOffset    : " << "0x" << std::hex << std::setw(8) << std::setfill('0') << partitionOffset << std::endl;
    oss << "blockSize     : " << std::dec << std::setw(8) << std::setfill('0') << blockSize << std::endl;
    oss << "partEntry     : " << partitionEntry << std::endl;
    oss << "pnorFlags     : " << InstructionPnorFlagToString(pnorFlags) << std::endl;
    oss << "data length   : " << data.getBitLength() << std::endl;
    oss << "data          : " << data.genHexLeftStr() << std::endl;

    return oss.str();
}

std::string PNORInstruction::getInstructionVars(const InstructionStatus & i_status) const {
  std::ostringstream oss;

  oss << std::hex << std::setfill('0');
  oss << "rc: " << std::setw(8) << i_status.rc;
  oss << " device: " << deviceString;
  oss << " partOffset: " << std::setw(8) << partitionOffset;
  oss << " blockSize: " << blockSize;
  oss << " command: " << InstructionCommandToString(command);

  return oss.str();
}
