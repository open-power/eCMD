//IBM_PROLOG_BEGIN_TAG
//IBM_PROLOG_END_TAG

#include <ServerSBEFIFOInstruction.H>
#include <OutputLite.H>
#include <stdio.h>
#include <errno.h>

#include <adal_sbefifo.h>

uint32_t ServerSBEFIFOInstruction::sbefifo_open(Handle ** handle, InstructionStatus & o_status)
{
    int rc = 0;

    char device[50];
    char errstr[200];

    /* already have a handle lets reuse it */
    if(*handle != NULL)
        return rc;

    /* We need to open the device*/
    snprintf(device, 50, "/dev/sbefifo%s", deviceString.c_str());
    errno = 0;

    if (flags & INSTRUCTION_FLAG_SERVER_DEBUG)
    {
        snprintf(errstr, 200, "SERVER_DEBUG : adal_sbefifo_open(%s, O_RDWR | O_EXCL)\n", device);
        o_status.errorMessage.append(errstr);
    }

#ifdef TESTING
    TEST_PRINT("*handle = adal_sbefifo_open(%s, O_RDWR | O_EXCL);\n", device);
    *handle = (Handle *) 0x1;
#else
    *handle = (Handle *) adal_sbefifo_open(device, O_RDWR | O_EXCL);
#endif

    if (flags & INSTRUCTION_FLAG_SERVER_DEBUG)
    {
        snprintf(errstr, 200, "SERVER_DEBUG : adal_sbefifo_open() *handle = %p\n", *handle);
        o_status.errorMessage.append(errstr);
    }

    if (*handle == NULL)
    {
        snprintf(errstr, 200, "SBEFIFOInstruction::sbefifo_open Problem opening SBEFIFO device %s : errno %d\n", device, errno);
        o_status.errorMessage.append(errstr);
        return SERVER_INVALID_SBEFIFO_DEVICE;
    }

    return rc;
}

void ServerSBEFIFOInstruction::sbefifo_ffdc_and_reset(Handle ** handle, InstructionStatus & o_status)
{
    // system_sbefifo_ffdc_extract
    //  append to o_status.errorMessage
    // system_sbefifo ffdc_unlock
    // system_sbefifo_reset

    uint32_t rc = sbefifo_close(*handle);
    *handle = NULL;
    if (rc) o_status.rc = SERVER_SBEFIFO_CLOSE_FAIL;
}

uint32_t ServerSBEFIFOInstruction::sbefifo_close(Handle * handle)
{
#ifdef TESTING
    TEST_PRINT("adal_sbefifo_close((adal_t *) handle);\n");
    return 0;
#else
    return adal_sbefifo_close((adal_t *) handle);
#endif
}

ssize_t ServerSBEFIFOInstruction::sbefifo_submit(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status, uint32_t & o_reply_wordcount)
{
            ssize_t rc = 0;

            adal_sbefifo_request l_request;
            adal_sbefifo_reply l_reply;

            l_request.data = ecmdDataBufferImplementationHelper::getDataPtr(&data);
            l_request.wordcount = data.getWordLength();
            l_request.status = 0;

            l_reply.data = ecmdDataBufferImplementationHelper::getDataPtr(&o_data);
            l_reply.wordcount = o_data.getWordLength();
            l_reply.status = 0;

#ifdef TESTING
            TEST_PRINT("adal_sbefifo_submit((adal_t *) i_handle, &l_request, &l_reply, %d);\n", timeout);
            rc = 0;
#else
            rc = adal_sbefifo_submit((adal_t *) i_handle, &l_request, &l_reply, timeout);
#endif

            if (flags & INSTRUCTION_FLAG_SERVER_DEBUG)
            {
                char errstr[200];
                snprintf(errstr, 200, "SERVER_DEBUG : adal_sbefifo_submit() rc = %u\n", rc);
                o_status.errorMessage.append(errstr);
                snprintf(errstr, 200, "SERVER_DEBUG : l_request.status = %08X\n", (uint32_t) l_request.status);
                o_status.errorMessage.append(errstr);
                snprintf(errstr, 200, "SERVER_DEBUG : l_request.wordcount = %zu\n", l_request.wordcount);
                o_status.errorMessage.append(errstr);
                snprintf(errstr, 200, "SERVER_DEBUG : l_reply.status = %08X\n", (uint32_t) l_reply.status);
                o_status.errorMessage.append(errstr);
                snprintf(errstr, 200, "SERVER_DEBUG : l_reply.wordcount = %zu\n", l_reply.wordcount);
                o_status.errorMessage.append(errstr);
            }

            o_status.data.setBitLength(64);
            o_status.data.setWord(0, (uint32_t) l_request.status);
            o_status.data.setWord(1, (uint32_t) l_reply.status);

            o_reply_wordcount = l_reply.wordcount;
            return rc;
}