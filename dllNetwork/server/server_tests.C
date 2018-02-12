//IBM_PROLOG_BEGIN_TAG
/* 
 * Copyright 2018 IBM International Business Machines Corp.
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

#include <stdlib.h>
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <arpa/inet.h>

#include <OutputLite.H>
OutputLite out;

#include <Instruction.H>
#include <FSIInstruction.H>
#include <TestableFSIInstruction.H>
#include <I2CInstruction.H>
#include <TestableI2CInstruction.H>

void test_i2c_reset(I2CInstruction & i, const Instruction::InstructionCommand c);

SCENARIO( "Basic Instruction Class", "[Instruction]" ) {

    GIVEN( "An Instruction Object" ) {
        Instruction i;

        THEN( "object is created correctly" ) {
            REQUIRE( i.getVersion() == 1 );
            REQUIRE( i.getType() == Instruction::NOINSTRUCTION );
            REQUIRE( i.getCommand() == Instruction::NOCOMMAND );
            //REQUIRE( i.getFlags() == 0 );
        }

        WHEN( "execute is called" ) {
            ecmdDataBuffer b;
            InstructionStatus is;
            Handle * h = NULL;
            uint32_t rc = i.execute(b, is, &h);

            THEN( "status contains info and return code" ) {
                REQUIRE( is.instructionVersion == i.getVersion() );
                REQUIRE( is.errorMessage.size() >= 1 );
                REQUIRE( is.rc == SERVER_UNKNOWN_INSTRUCTION );
                REQUIRE( rc == 0 );
            }
        }
    }
}

SCENARIO( "FSIInstruction Class", "[Instruction]" ) {

    GIVEN( "A FSIInstruction Object") {
        FSIInstruction i;

        THEN( "object is created correctly" ) {
            REQUIRE( i.getVersion() == 6 );
            REQUIRE( i.getType() == Instruction::FSI );
            REQUIRE( i.getCommand() == Instruction::NOCOMMAND );
            //REQUIRE( i.getFlags() == 0 );
        }

        WHEN( "execute is called with no command" ) {
            ecmdDataBuffer b;
            InstructionStatus is;
            Handle * h = NULL;
            uint32_t rc = i.execute(b, is, &h);

            THEN( "status contains info and return code" ) {
                REQUIRE( is.instructionVersion == i.getVersion() );
                REQUIRE( is.errorMessage.size() == 0 );
                REQUIRE( is.rc == SERVER_COMMAND_NOT_SUPPORTED );
                REQUIRE( rc == SERVER_COMMAND_NOT_SUPPORTED );
            }
        }

    /*
     * =========== Device String Format (flag & INSTRUCTION_FLAG_DEVSTR)
     * First Word:      version
     * Second Word:     command
     * Third Word:      flags
     * ======= // 64-bit address (flag & INSTRUCTION_FLAG_64BIT_ADDRESS)
     * XXXXX Word:      address64 upper 32 bits
     * XXXXX Word:      address64 lower 32 bits
     * ======= // 32-bit address
     * XXXXX Word:      address
     * XXXXX Word:      length
     * XXXXX Word:      deviceString size
     * XXXXX Word:      data size       // only used for LONGIN, SCOMIN, WRITESPMEM, SCOMIN_MASK, BULK_SCOMIN
     * XXXXX Word:      mask size       // only used for SCOMIN_MASK
     * =======
     * Multiple Words:  deviceString
     * Multiple Words:  data            // only used for LONGIN, SCOMIN, WRITESPMEM, SCOMIN_MASK, BULK_SCOMIN
     * Multiple Words:  mask            // only used for SCOMIN_MASK
     */
        WHEN( "setup(SCOMIN, 32) is called on the object" ) {
            std::string d = "empty";
            uint64_t a = 0x08010203;
            uint32_t l = 64;
            uint32_t f = 0;
            ecmdDataBuffer b(l);
            b.setWord(0, 0x1234ABCD);
            b.setWord(1, 0x5678FE90);

            uint32_t rc = i.setup(Instruction::SCOMIN, d, a, l, f, &b, NULL);

            THEN( "internal data is correct" ) {
                REQUIRE( i.getVersion() == 5 );
                REQUIRE( i.getType() == Instruction::FSI );
                REQUIRE( i.getCommand() == Instruction::SCOMIN );
                REQUIRE( i.getFlags() == (f | INSTRUCTION_FLAG_DEVSTR) );
            }

            WHEN( "flatten() is called" ) {
                uint32_t size = i.flattenSize();
                uint8_t flatten_data[size];

                uint32_t rc = i.flatten(flatten_data, size);

                THEN( "flatten_data is valid" ) {
                    REQUIRE( rc == 0 );
                    uint32_t * words = (uint32_t * ) flatten_data;
                    uint32_t offset = 0;
                    uint32_t dSize = d.size() + 1;
                    if (dSize % sizeof(uint32_t)) {
                        dSize += (sizeof(uint32_t) - (dSize % sizeof(uint32_t)));
                    }
                    REQUIRE( htonl(words[offset++]) == i.getVersion() );
                    REQUIRE( htonl(words[offset++]) == i.getCommand() );
                    REQUIRE( htonl(words[offset++]) == i.getFlags() );
                    REQUIRE( htonl(words[offset++]) == a );
                    REQUIRE( htonl(words[offset++]) == l );
                    REQUIRE( htonl(words[offset++]) == dSize );
                    REQUIRE( htonl(words[offset++]) == b.flattenSize());
                    char * d_test = ((char *)(words + offset));
                    REQUIRE( d_test == d );
                    ecmdDataBuffer b_test;
                    rc = b_test.unflatten((uint8_t *) (words + offset + (dSize / sizeof(uint32_t))), b.flattenSize());
                    REQUIRE( rc == 0 );
                    REQUIRE( b_test == b );
                }

                WHEN( "unflatten() is called" ) {
                    TestableFSIInstruction i_test;
                    uint32_t rc = i_test.unflatten(flatten_data, size);

                    THEN( "i_test is valid" ) {
                        REQUIRE( rc == 0 );
                        REQUIRE( i_test.getVersion() == i.getVersion() );
                        REQUIRE( i_test.getType() == i.getType() );
                        REQUIRE( i_test.getCommand() == i.getCommand() );
                        REQUIRE( i_test.getFlags() == i.getFlags() );
                        REQUIRE( i_test.getAddress() == a );
                        REQUIRE( i_test.getLength() == l );
                        REQUIRE( i_test.getDeviceString() == d );
                        const ecmdDataBuffer * b_test = i_test.getData();
                        REQUIRE( b_test != NULL );
                        REQUIRE( *b_test == b );
                        const ecmdDataBuffer * m_test = i_test.getMask();
                        REQUIRE( m_test == NULL );
                    }
                }
            }
        }

        WHEN( "setup(SCOMIN, 64) is called on the object" ) {
            std::string d = "empty";
            uint64_t a = 0x808010203;
            uint32_t l = 64;
            uint32_t f = 0;
            ecmdDataBuffer b(l);
            b.setWord(0, 0x1234ABCD);
            b.setWord(1, 0x5678FE90);

            uint32_t rc = i.setup(Instruction::SCOMIN, d, a, l, f, &b, NULL);

            THEN( "internal data is correct" ) {
                REQUIRE( i.getVersion() == 5 );
                REQUIRE( i.getType() == Instruction::FSI );
                REQUIRE( i.getCommand() == Instruction::SCOMIN );
                REQUIRE( i.getFlags() == (f | INSTRUCTION_FLAG_DEVSTR | INSTRUCTION_FLAG_64BIT_ADDRESS) );
            }

            WHEN( "flatten() is called" ) {
                uint32_t size = i.flattenSize();
                uint8_t flatten_data[size];

                uint32_t rc = i.flatten(flatten_data, size);

                THEN( "flatten_data is valid" ) {
                    REQUIRE( rc == 0 );
                    uint32_t * words = (uint32_t * ) flatten_data;
                    uint32_t offset = 0;
                    uint32_t dSize = d.size() + 1;
                    if (dSize % sizeof(uint32_t)) {
                        dSize += (sizeof(uint32_t) - (dSize % sizeof(uint32_t)));
                    }
                    REQUIRE( htonl(words[offset++]) == i.getVersion() );
                    REQUIRE( htonl(words[offset++]) == i.getCommand() );
                    REQUIRE( htonl(words[offset++]) == i.getFlags() );
                    REQUIRE( htonl(words[offset++]) == (a >> 32) );
                    REQUIRE( htonl(words[offset++]) == (0xFFFFFFFF & a) );
                    REQUIRE( htonl(words[offset++]) == l );
                    REQUIRE( htonl(words[offset++]) == dSize );
                    REQUIRE( htonl(words[offset++]) == b.flattenSize());
                    char * d_test = ((char *)(words + offset));
                    REQUIRE( d_test == d );
                    ecmdDataBuffer b_test;
                    rc = b_test.unflatten((uint8_t *) (words + offset + (dSize / sizeof(uint32_t))), b.flattenSize());
                    REQUIRE( rc == 0 );
                    REQUIRE( b_test == b );
                }

                WHEN( "unflatten() is called" ) {
                    TestableFSIInstruction i_test;
                    uint32_t rc = i_test.unflatten(flatten_data, size);

                    THEN( "i_test is valid" ) {
                        REQUIRE( rc == 0 );
                        REQUIRE( i_test.getVersion() == i.getVersion() );
                        REQUIRE( i_test.getType() == i.getType() );
                        REQUIRE( i_test.getCommand() == i.getCommand() );
                        REQUIRE( i_test.getFlags() == i.getFlags() );
                        REQUIRE( i_test.getAddress64() == a );
                        REQUIRE( i_test.getLength() == l );
                        REQUIRE( i_test.getDeviceString() == d );
                        const ecmdDataBuffer * b_test = i_test.getData();
                        REQUIRE( b_test != NULL );
                        REQUIRE( *b_test == b );
                        const ecmdDataBuffer * m_test = i_test.getMask();
                        REQUIRE( m_test == NULL );
                    }
                }
            }
        }

        WHEN( "setup(SCOMIN_MASK, 32) is called on the object" ) {
            std::string d = "empty";
            uint64_t a = 0x08010203;
            uint32_t l = 64;
            uint32_t f = 0;
            ecmdDataBuffer b(l);
            b.setWord(0, 0x1234ABCD);
            b.setWord(1, 0x5678FE90);
            ecmdDataBuffer m(l);
            m.setWord(0, 0x22225555);
            m.setWord(1, 0xAAAA0000);

            uint32_t rc = i.setup(Instruction::SCOMIN_MASK, d, a, l, f, &b, &m);

            THEN( "internal data is correct" ) {
                REQUIRE( i.getVersion() == 5 );
                REQUIRE( i.getType() == Instruction::FSI );
                REQUIRE( i.getCommand() == Instruction::SCOMIN_MASK );
                REQUIRE( i.getFlags() == (f | INSTRUCTION_FLAG_DEVSTR) );
            }

            WHEN( "flatten() is called" ) {
                uint32_t size = i.flattenSize();
                uint8_t flatten_data[size];

                uint32_t rc = i.flatten(flatten_data, size);

                THEN( "flatten_data is valid" ) {
                    REQUIRE( rc == 0 );
                    uint32_t * words = (uint32_t * ) flatten_data;
                    uint32_t offset = 0;
                    uint32_t dSize = d.size() + 1;
                    if (dSize % sizeof(uint32_t)) {
                        dSize += (sizeof(uint32_t) - (dSize % sizeof(uint32_t)));
                    }
                    REQUIRE( htonl(words[offset++]) == i.getVersion() );
                    REQUIRE( htonl(words[offset++]) == i.getCommand() );
                    REQUIRE( htonl(words[offset++]) == i.getFlags() );
                    REQUIRE( htonl(words[offset++]) == a );
                    REQUIRE( htonl(words[offset++]) == l );
                    REQUIRE( htonl(words[offset++]) == dSize );
                    REQUIRE( htonl(words[offset++]) == b.flattenSize());
                    REQUIRE( htonl(words[offset++]) == m.flattenSize());
                    char * d_test = ((char *)(words + offset));
                    REQUIRE( d_test == d );
                    ecmdDataBuffer b_test;
                    rc = b_test.unflatten((uint8_t *) (words + offset + (dSize / sizeof(uint32_t))), b.flattenSize());
                    REQUIRE( rc == 0 );
                    REQUIRE( b_test == b );
                    ecmdDataBuffer m_test;
                    rc = m_test.unflatten((uint8_t *) (words + offset + ((dSize + b.flattenSize()) / sizeof(uint32_t))), m.flattenSize());
                    REQUIRE( rc == 0 );
                    REQUIRE( m_test == m );
                }

                WHEN( "unflatten() is called" ) {
                    TestableFSIInstruction i_test;
                    uint32_t rc = i_test.unflatten(flatten_data, size);

                    THEN( "i_test is valid" ) {
                        REQUIRE( rc == 0 );
                        REQUIRE( i_test.getVersion() == i.getVersion() );
                        REQUIRE( i_test.getType() == i.getType() );
                        REQUIRE( i_test.getCommand() == i.getCommand() );
                        REQUIRE( i_test.getFlags() == i.getFlags() );
                        REQUIRE( i_test.getAddress() == a );
                        REQUIRE( i_test.getLength() == l );
                        REQUIRE( i_test.getDeviceString() == d );
                        const ecmdDataBuffer * b_test = i_test.getData();
                        REQUIRE( b_test != NULL );
                        REQUIRE( *b_test == b );
                        const ecmdDataBuffer * m_test = i_test.getMask();
                        REQUIRE( m_test != NULL );
                        REQUIRE( *m_test == m );
                    }
                }
            }
        }

        WHEN( "setup(SCOMOUT, 32) is called on the object" ) {
            std::string d = "empty";
            uint64_t a = 0x08010203;
            uint32_t l = 64;
            uint32_t f = 0;

            uint32_t rc = i.setup(Instruction::SCOMOUT, d, a, l, f, NULL, NULL);

            THEN( "internal data is correct" ) {
                REQUIRE( i.getVersion() == 5 );
                REQUIRE( i.getType() == Instruction::FSI );
                REQUIRE( i.getCommand() == Instruction::SCOMOUT );
                REQUIRE( i.getFlags() == (f | INSTRUCTION_FLAG_DEVSTR) );
            }

            WHEN( "flatten() is called" ) {
                uint32_t size = i.flattenSize();
                uint8_t flatten_data[size];

                uint32_t rc = i.flatten(flatten_data, size);

                THEN( "flatten_data is valid" ) {
                    REQUIRE( rc == 0 );
                    uint32_t * words = (uint32_t * ) flatten_data;
                    uint32_t offset = 0;
                    uint32_t dSize = d.size() + 1;
                    if (dSize % sizeof(uint32_t)) {
                        dSize += (sizeof(uint32_t) - (dSize % sizeof(uint32_t)));
                    }
                    REQUIRE( htonl(words[offset++]) == i.getVersion() );
                    REQUIRE( htonl(words[offset++]) == i.getCommand() );
                    REQUIRE( htonl(words[offset++]) == i.getFlags() );
                    REQUIRE( htonl(words[offset++]) == a );
                    REQUIRE( htonl(words[offset++]) == l );
                    REQUIRE( htonl(words[offset++]) == dSize );
                    char * d_test = ((char *)(words + offset));
                    REQUIRE( d_test == d );
                }

                WHEN( "unflatten() is called" ) {
                    TestableFSIInstruction i_test;
                    uint32_t rc = i_test.unflatten(flatten_data, size);

                    THEN( "i_test is valid" ) {
                        REQUIRE( rc == 0 );
                        REQUIRE( i_test.getVersion() == i.getVersion() );
                        REQUIRE( i_test.getType() == i.getType() );
                        REQUIRE( i_test.getCommand() == i.getCommand() );
                        REQUIRE( i_test.getFlags() == i.getFlags() );
                        REQUIRE( i_test.getAddress() == a );
                        REQUIRE( i_test.getLength() == l );
                        REQUIRE( i_test.getDeviceString() == d );
                        const ecmdDataBuffer * b_test = i_test.getData();
                        REQUIRE( b_test == NULL );
                        const ecmdDataBuffer * m_test = i_test.getMask();
                        REQUIRE( m_test == NULL );
                    }
                }
            }
        }
    }
}

SCENARIO( "I2CInstruction Class", "[Instruction]" ) {

    GIVEN( "A I2CInstruction Object") {
        I2CInstruction i;

        THEN( "object is created correctly" ) {
            REQUIRE( i.getVersion() == 4 );
            REQUIRE( i.getType() == Instruction::I2C );
            REQUIRE( i.getCommand() == Instruction::NOCOMMAND );
            //REQUIRE( i.getFlags() == 0 );
        }

        WHEN( "execute is called with no command" ) {
            ecmdDataBuffer b;
            InstructionStatus is;
            Handle * h = NULL;
            uint32_t rc = i.execute(b, is, &h);

            THEN( "status contains info and return code" ) {
                REQUIRE( is.instructionVersion == i.getVersion() );
                REQUIRE( is.errorMessage.size() == 0 );
                REQUIRE( is.rc == SERVER_COMMAND_NOT_SUPPORTED );
                REQUIRE( rc == SERVER_COMMAND_NOT_SUPPORTED );
            }
        }

        WHEN( "setup(I2CREAD, ofs = 2) is called on the object" ) {
            std::string d = "empty";
            uint32_t e = 412;
            uint32_t p = 45;
            uint32_t sa = 0xAB;
            uint32_t bs = 400;
            uint64_t o = 20;
            uint32_t ofs = 2;
            uint32_t l = 8;
            uint32_t i2cf = 9;
            uint32_t f = 0;
            uint32_t rc = i.setup(Instruction::I2CREAD, d, e, p, sa, bs, o, ofs, l, i2cf, f, NULL);

            THEN( "internal data is correct" ) {
                REQUIRE( i.getVersion() == 4 );
                REQUIRE( i.getType() == Instruction::I2C );
                REQUIRE( i.getCommand() == Instruction::I2CREAD );
                REQUIRE( i.getFlags() == (f | INSTRUCTION_FLAG_DEVSTR) );
            }
            WHEN( "flatten() is called" ) {
                uint32_t size = i.flattenSize();
                uint8_t flatten_data[size];

                uint32_t rc = i.flatten(flatten_data, size);

                THEN( "flatten_data is valid" ) {
                    REQUIRE( rc == 0 );
                    uint32_t * words = (uint32_t * ) flatten_data;
                    uint32_t offset = 0;
                    uint32_t dSize = d.size() + 1;
                    if (dSize % sizeof(uint32_t)) {
                        dSize += (sizeof(uint32_t) - (dSize % sizeof(uint32_t)));
                    }
                    REQUIRE( htonl(words[offset++]) == i.getVersion() );
                    REQUIRE( htonl(words[offset++]) == i.getCommand() );
                    REQUIRE( htonl(words[offset++]) == i.getFlags() );
                    REQUIRE( htonl(words[offset++]) == e );
                    REQUIRE( htonl(words[offset++]) == p );
                    REQUIRE( htonl(words[offset++]) == sa );
                    REQUIRE( htonl(words[offset++]) == bs );
                    REQUIRE( htonl(words[offset++]) == o );
                    REQUIRE( htonl(words[offset++]) == ofs );
                    REQUIRE( htonl(words[offset++]) == l );
                    REQUIRE( htonl(words[offset++]) == i2cf );
                    REQUIRE( htonl(words[offset++]) == dSize );
                    char * d_test = ((char *)(words + offset));
                    REQUIRE( d_test == d );
                }

                WHEN( "unflatten() is called" ) {
                    TestableI2CInstruction i_test;
                    uint32_t rc = i_test.unflatten(flatten_data, size);

                    THEN( "i_test is valid" ) {
                        REQUIRE( rc == 0 );
                        REQUIRE( i_test.getVersion() == i.getVersion() );
                        REQUIRE( i_test.getType() == i.getType() );
                        REQUIRE( i_test.getCommand() == i.getCommand() );
                        REQUIRE( i_test.getFlags() == i.getFlags() );
                        REQUIRE( i_test.getEngineId() == e );
                        REQUIRE( i_test.getPort() == p );
                        REQUIRE( i_test.getSlaveAddress() == sa );
                        REQUIRE( i_test.getBusSpeed() == bs );
                        REQUIRE( i_test.getOffset() == o );
                        REQUIRE( i_test.getOffsetFieldSize() == ofs );
                        REQUIRE( i_test.getI2cFlags() == i2cf );
                        REQUIRE( i_test.getLength() == l );
                        REQUIRE( i_test.getDeviceString() == d );
                        const ecmdDataBuffer * b_test = i_test.getData();
                        REQUIRE( b_test == NULL );
                        const ecmdDataBuffer * m_test = i_test.getMask();
                        REQUIRE( m_test == NULL );
                    }
                }
            }
        }

        WHEN( "setup(I2CREAD, ofs = 8) is called on the object" ) {
            std::string d = "empty";
            uint32_t e = 412;
            uint32_t p = 45;
            uint32_t sa = 0xAB;
            uint32_t bs = 400;
            uint64_t o = 0x0102030405060708;
            uint32_t ofs = 8;
            uint32_t l = 8;
            uint32_t i2cf = 9;
            uint32_t f = 0;
            uint32_t rc = i.setup(Instruction::I2CREAD, d, e, p, sa, bs, o, ofs, l, i2cf, f, NULL);

            THEN( "internal data is correct" ) {
                REQUIRE( i.getVersion() == 6 );
                REQUIRE( i.getType() == Instruction::I2C );
                REQUIRE( i.getCommand() == Instruction::I2CREAD );
                REQUIRE( i.getFlags() == (f | INSTRUCTION_FLAG_DEVSTR) );
            }
            WHEN( "flatten() is called" ) {
                uint32_t size = i.flattenSize();
                uint8_t flatten_data[size];

                uint32_t rc = i.flatten(flatten_data, size);

                THEN( "flatten_data is valid" ) {
                    REQUIRE( rc == 0 );
                    uint32_t * words = (uint32_t * ) flatten_data;
                    uint32_t offset = 0;
                    uint32_t dSize = d.size() + 1;
                    if (dSize % sizeof(uint32_t)) {
                        dSize += (sizeof(uint32_t) - (dSize % sizeof(uint32_t)));
                    }
                    REQUIRE( htonl(words[offset++]) == i.getVersion() );
                    REQUIRE( htonl(words[offset++]) == i.getCommand() );
                    REQUIRE( htonl(words[offset++]) == i.getFlags() );
                    REQUIRE( htonl(words[offset++]) == e );
                    REQUIRE( htonl(words[offset++]) == p );
                    REQUIRE( htonl(words[offset++]) == sa );
                    REQUIRE( htonl(words[offset++]) == bs );
                    offset++;//REQUIRE( htonl(words[offset++]) == o );
                    REQUIRE( htonl(words[offset++]) == ofs );
                    REQUIRE( htonl(words[offset++]) == l );
                    REQUIRE( htonl(words[offset++]) == i2cf );
                    REQUIRE( htonl(words[offset++]) == dSize );
                    REQUIRE( htonl(words[offset++]) == (o >> 32) );
                    REQUIRE( htonl(words[offset++]) == (0xFFFFFFFF & o) );
                    char * d_test = ((char *)(words + offset));
                    REQUIRE( d_test == d );
                }

                WHEN( "unflatten() is called" ) {
                    TestableI2CInstruction i_test;
                    uint32_t rc = i_test.unflatten(flatten_data, size);

                    THEN( "i_test is valid" ) {
                        REQUIRE( rc == 0 );
                        REQUIRE( i_test.getVersion() == i.getVersion() );
                        REQUIRE( i_test.getType() == i.getType() );
                        REQUIRE( i_test.getCommand() == i.getCommand() );
                        REQUIRE( i_test.getFlags() == i.getFlags() );
                        REQUIRE( i_test.getEngineId() == e );
                        REQUIRE( i_test.getPort() == p );
                        REQUIRE( i_test.getSlaveAddress() == sa );
                        REQUIRE( i_test.getBusSpeed() == bs );
                        REQUIRE( i_test.getAddress() == o );
                        REQUIRE( i_test.getOffsetFieldSize() == ofs );
                        REQUIRE( i_test.getI2cFlags() == i2cf );
                        REQUIRE( i_test.getLength() == l );
                        REQUIRE( i_test.getDeviceString() == d );
                        const ecmdDataBuffer * b_test = i_test.getData();
                        REQUIRE( b_test == NULL );
                        const ecmdDataBuffer * m_test = i_test.getMask();
                        REQUIRE( m_test == NULL );
                    }
                }
            }
        }

        WHEN( "setup(I2CWRITE, ofs = 2) is called on the object" ) {
            std::string d = "empty";
            uint32_t e = 412;
            uint32_t p = 45;
            uint32_t sa = 0xAB;
            uint32_t bs = 400;
            uint64_t o = 20;
            uint32_t ofs = 2;
            uint32_t l = 8;
            uint32_t i2cf = 9;
            uint32_t f = 0;
            ecmdDataBuffer b(l);
            b.setByte(0, 0x5A);
            uint32_t rc = i.setup(Instruction::I2CWRITE, d, e, p, sa, bs, o, ofs, l, i2cf, f, &b);

            THEN( "internal data is correct" ) {
                REQUIRE( i.getVersion() == 4 );
                REQUIRE( i.getType() == Instruction::I2C );
                REQUIRE( i.getCommand() == Instruction::I2CWRITE );
                REQUIRE( i.getFlags() == (f | INSTRUCTION_FLAG_DEVSTR) );
            }
            WHEN( "flatten() is called" ) {
                uint32_t size = i.flattenSize();
                uint8_t flatten_data[size];

                uint32_t rc = i.flatten(flatten_data, size);

                THEN( "flatten_data is valid" ) {
                    REQUIRE( rc == 0 );
                    uint32_t * words = (uint32_t * ) flatten_data;
                    uint32_t offset = 0;
                    uint32_t dSize = d.size() + 1;
                    if (dSize % sizeof(uint32_t)) {
                        dSize += (sizeof(uint32_t) - (dSize % sizeof(uint32_t)));
                    }
                    REQUIRE( htonl(words[offset++]) == i.getVersion() );
                    REQUIRE( htonl(words[offset++]) == i.getCommand() );
                    REQUIRE( htonl(words[offset++]) == i.getFlags() );
                    REQUIRE( htonl(words[offset++]) == e );
                    REQUIRE( htonl(words[offset++]) == p );
                    REQUIRE( htonl(words[offset++]) == sa );
                    REQUIRE( htonl(words[offset++]) == bs );
                    REQUIRE( htonl(words[offset++]) == o );
                    REQUIRE( htonl(words[offset++]) == ofs );
                    REQUIRE( htonl(words[offset++]) == l );
                    REQUIRE( htonl(words[offset++]) == i2cf );
                    REQUIRE( htonl(words[offset++]) == dSize );
                    REQUIRE( htonl(words[offset++]) == b.flattenSize());
                    char * d_test = ((char *)(words + offset));
                    REQUIRE( d_test == d );
                    ecmdDataBuffer b_test;
                    rc = b_test.unflatten((uint8_t *) (words + offset + (dSize / sizeof(uint32_t))), b.flattenSize());
                    REQUIRE( rc == 0 );
                    REQUIRE( b_test == b );
                }

                WHEN( "unflatten() is called" ) {
                    TestableI2CInstruction i_test;
                    uint32_t rc = i_test.unflatten(flatten_data, size);

                    THEN( "i_test is valid" ) {
                        REQUIRE( rc == 0 );
                        REQUIRE( i_test.getVersion() == i.getVersion() );
                        REQUIRE( i_test.getType() == i.getType() );
                        REQUIRE( i_test.getCommand() == i.getCommand() );
                        REQUIRE( i_test.getFlags() == i.getFlags() );
                        REQUIRE( i_test.getEngineId() == e );
                        REQUIRE( i_test.getPort() == p );
                        REQUIRE( i_test.getSlaveAddress() == sa );
                        REQUIRE( i_test.getBusSpeed() == bs );
                        REQUIRE( i_test.getOffset() == o );
                        REQUIRE( i_test.getOffsetFieldSize() == ofs );
                        REQUIRE( i_test.getI2cFlags() == i2cf );
                        REQUIRE( i_test.getLength() == l );
                        REQUIRE( i_test.getDeviceString() == d );
                        const ecmdDataBuffer * b_test = i_test.getData();
                        REQUIRE( b_test != NULL );
                        REQUIRE( *b_test == b );
                        const ecmdDataBuffer * m_test = i_test.getMask();
                        REQUIRE( m_test == NULL );
                    }
                }
            }
        }

        WHEN( "setup(I2CWRITE, ofs = 8) is called on the object" ) {
            std::string d = "empty";
            uint32_t e = 412;
            uint32_t p = 45;
            uint32_t sa = 0xAB;
            uint32_t bs = 400;
            uint64_t o = 0x0102030405060708;
            uint32_t ofs = 8;
            uint32_t l = 8;
            uint32_t i2cf = 9;
            uint32_t f = 0;
            ecmdDataBuffer b(l);
            b.setByte(0, 0x5A);
            uint32_t rc = i.setup(Instruction::I2CWRITE, d, e, p, sa, bs, o, ofs, l, i2cf, f, &b);

            THEN( "internal data is correct" ) {
                REQUIRE( i.getVersion() ==  6 );
                REQUIRE( i.getType() == Instruction::I2C );
                REQUIRE( i.getCommand() == Instruction::I2CWRITE );
                REQUIRE( i.getFlags() == (f | INSTRUCTION_FLAG_DEVSTR) );
            }
            WHEN( "flatten() is called" ) {
                uint32_t size = i.flattenSize();
                uint8_t flatten_data[size];

                uint32_t rc = i.flatten(flatten_data, size);

                THEN( "flatten_data is valid" ) {
                    REQUIRE( rc == 0 );
                    uint32_t * words = (uint32_t * ) flatten_data;
                    uint32_t offset = 0;
                    uint32_t dSize = d.size() + 1;
                    if (dSize % sizeof(uint32_t)) {
                        dSize += (sizeof(uint32_t) - (dSize % sizeof(uint32_t)));
                    }
                    REQUIRE( htonl(words[offset++]) == i.getVersion() );
                    REQUIRE( htonl(words[offset++]) == i.getCommand() );
                    REQUIRE( htonl(words[offset++]) == i.getFlags() );
                    REQUIRE( htonl(words[offset++]) == e );
                    REQUIRE( htonl(words[offset++]) == p );
                    REQUIRE( htonl(words[offset++]) == sa );
                    REQUIRE( htonl(words[offset++]) == bs );
                    offset++; //REQUIRE( htonl(words[offset++]) == o );
                    REQUIRE( htonl(words[offset++]) == ofs );
                    REQUIRE( htonl(words[offset++]) == l );
                    REQUIRE( htonl(words[offset++]) == i2cf );
                    REQUIRE( htonl(words[offset++]) == dSize );
                    REQUIRE( htonl(words[offset++]) == (o >> 32) );
                    REQUIRE( htonl(words[offset++]) == (0xFFFFFFFF & o) );
                    REQUIRE( htonl(words[offset++]) == b.flattenSize());
                    char * d_test = ((char *)(words + offset));
                    REQUIRE( d_test == d );
                    ecmdDataBuffer b_test;
                    rc = b_test.unflatten((uint8_t *) (words + offset + (dSize / sizeof(uint32_t))), b.flattenSize());
                    REQUIRE( rc == 0 );
                    REQUIRE( b_test == b );
                }

                WHEN( "unflatten() is called" ) {
                    TestableI2CInstruction i_test;
                    uint32_t rc = i_test.unflatten(flatten_data, size);

                    THEN( "i_test is valid" ) {
                        REQUIRE( rc == 0 );
                        REQUIRE( i_test.getVersion() == i.getVersion() );
                        REQUIRE( i_test.getType() == i.getType() );
                        REQUIRE( i_test.getCommand() == i.getCommand() );
                        REQUIRE( i_test.getFlags() == i.getFlags() );
                        REQUIRE( i_test.getEngineId() == e );
                        REQUIRE( i_test.getPort() == p );
                        REQUIRE( i_test.getSlaveAddress() == sa );
                        REQUIRE( i_test.getBusSpeed() == bs );
                        REQUIRE( i_test.getAddress() == o );
                        REQUIRE( i_test.getOffsetFieldSize() == ofs );
                        REQUIRE( i_test.getI2cFlags() == i2cf );
                        REQUIRE( i_test.getLength() == l );
                        REQUIRE( i_test.getDeviceString() == d );
                        const ecmdDataBuffer * b_test = i_test.getData();
                        REQUIRE( b_test != NULL );
                        REQUIRE( *b_test == b );
                        const ecmdDataBuffer * m_test = i_test.getMask();
                        REQUIRE( m_test == NULL );
                    }
                }
            }
        }

        WHEN( "setup(I2CRESETLIGHT) is called on the object" ) {
            test_i2c_reset(i, Instruction::I2CRESETLIGHT);
        }

        WHEN( "setup(I2CRESETFULL) is called on the object" ) {
            test_i2c_reset(i, Instruction::I2CRESETFULL);
        }
    }
}

void test_i2c_reset(I2CInstruction & i, const Instruction::InstructionCommand c)
{
    std::string d = "empty";
    uint32_t e = 412;
    uint32_t p = 45;
    uint32_t sa = 0x01;
    uint32_t bs = 2;
    uint64_t o = 3;
    uint32_t ofs = 4;
    uint32_t l = 5;
    uint32_t i2cf = 6;
    uint32_t f = 7;
    uint32_t rc = i.setup(c, d, e, p, sa, bs, o, ofs, l, i2cf, f, NULL);

    THEN( "internal data is correct" ) {
        REQUIRE( i.getVersion() == 5 );
        REQUIRE( i.getType() == Instruction::I2C );
        REQUIRE( i.getCommand() == c );
        REQUIRE( i.getFlags() == (f | INSTRUCTION_FLAG_DEVSTR) );
    }
    WHEN( "flatten() is called" ) {
        uint32_t size = i.flattenSize();
        uint8_t flatten_data[size];

        uint32_t rc = i.flatten(flatten_data, size);

        THEN( "flatten_data is valid" ) {
            REQUIRE( rc == 0 );
            uint32_t * words = (uint32_t * ) flatten_data;
            uint32_t offset = 0;
            uint32_t dSize = d.size() + 1;
            if (dSize % sizeof(uint32_t)) {
                dSize += (sizeof(uint32_t) - (dSize % sizeof(uint32_t)));
            }
            REQUIRE( htonl(words[offset++]) == i.getVersion() );
            REQUIRE( htonl(words[offset++]) == i.getCommand() );
            REQUIRE( htonl(words[offset++]) == i.getFlags() );
            REQUIRE( htonl(words[offset++]) == e );
            REQUIRE( htonl(words[offset++]) == p );
            REQUIRE( htonl(words[offset++]) == sa );
            REQUIRE( htonl(words[offset++]) == bs );
            REQUIRE( htonl(words[offset++]) == o );
            REQUIRE( htonl(words[offset++]) == ofs );
            REQUIRE( htonl(words[offset++]) == l );
            REQUIRE( htonl(words[offset++]) == i2cf );
            REQUIRE( htonl(words[offset++]) == dSize );
            char * d_test = ((char *)(words + offset));
            REQUIRE( d_test == d );
        }

        WHEN( "unflatten() is called" ) {
            TestableI2CInstruction i_test;
            uint32_t rc = i_test.unflatten(flatten_data, size);

            THEN( "i_test is valid" ) {
                REQUIRE( rc == 0 );
                REQUIRE( i_test.getVersion() == i.getVersion() );
                REQUIRE( i_test.getType() == i.getType() );
                REQUIRE( i_test.getCommand() == i.getCommand() );
                REQUIRE( i_test.getFlags() == i.getFlags() );
                REQUIRE( i_test.getEngineId() == e );
                REQUIRE( i_test.getPort() == p );
                REQUIRE( i_test.getSlaveAddress() == sa );
                REQUIRE( i_test.getBusSpeed() == bs );
                REQUIRE( i_test.getOffset() == o );
                REQUIRE( i_test.getOffsetFieldSize() == ofs );
                REQUIRE( i_test.getI2cFlags() == i2cf );
                REQUIRE( i_test.getLength() == l );
                REQUIRE( i_test.getDeviceString() == d );
                const ecmdDataBuffer * b_test = i_test.getData();
                REQUIRE( b_test == NULL );
                const ecmdDataBuffer * m_test = i_test.getMask();
                REQUIRE( m_test == NULL );
            }
        }
    }
}

