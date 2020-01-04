#ifndef ECMDISTEP_H
#define ECMDISTEP_H

#include <string>
#include <map>
#include <stdint.h>

class ecmdIPLTable
{
public:

  // The length of istep name is limited to this size.
  // The istep names are known at this time. It has been manually verfied that the 
  // length of all the istep names are below 42. 
  enum { ECMD_FUNC_NAME_SIZE = 42 }; 

  // Starting istep number
  enum { ECMD_FIRST_ISTEP_NUM = 0 };

  // Ending istep number
  // Though istep numbers are from 0 to 26 in the IPL table, istep execution is
  // restricted to 21 as the remaining isteps are meant for debug purposes.
  enum { ECMD_LAST_ISTEP_NUM  = 21 };

  // This istep number is reserved for representing an invalid istep number.
  enum { ECMD_INVALID_ISTEP_NUM = 0xFFFF };

  // This position is used for representing an invalid row number in the 
  // ecmdIPLTable
  enum { ECMD_INVALID_POSITION = 0xFFFF };

  typedef enum {
  ECMD_ISTEP_HOST = 0x0,     // IStep is executed by host
  ECMD_ISTEP_SBE ,           // IStep is executed by Self Boot Engine
  ECMD_ISTEP_SP,             // IStep is executed by Service Processor 
  ECMD_ISTEP_NOOP,           // This istep is NOOP
  ECMD_ISTEP_INVALID_DESTINATION 
  } ecmdIStepDestination_t;

  struct ecmdIStep_t
  {
    uint16_t        majorNum ;          //Major Number
    uint16_t        minorNum ;          //Minor Number        
    const  char     istepName[ECMD_FUNC_NAME_SIZE];  // IPL Step Name
    // Indicates whether this IStep is executed by HOST/SBE/SP
    ecmdIStepDestination_t  destination;      
  };

  /**
    * @brief Constructor.
    * 
    * Right now, no need to initialize anything in the default constructor.
    * 
    */
  ecmdIPLTable() {};

  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Get the istep number
   *
   * @par Description:
   *     This routine gets the istep number for 
   *     the specified istep name.
   *
   * @param[in]   i_istepName   Name of the istep
   * @param[out]  o_majorNum    Major istep number
   * @param[out]  o_minorNum    Minor istep number
   *
   * @return false if the given istep name is an invalid one
   */

  bool getIStepNumber(  const std::string & i_istepName,
                        uint16_t & o_majorNum,
                        uint16_t & o_minorNum );

  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Validate the istep name
   *
   * @par Description:
   *     This routine checks whether the given
   *     the istep name exists.
   *
   * @param[in] i_istepName    Name of the istep
   *
   * @return true if the given istep name exists
   *         false, otherwise.
   */
  bool isValid(const std::string & i_istepName);

  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Validate the istep number (major)
   *
   * @par Description:
   *     This routine checks whether the given
   *     the istep number (major) exists.
   *
   * @param[in] i_majorNum  Major number of the istep
   *
   * @return true if the given istep major number exists
   *         false, otherwise.
   */

  bool isValid(uint16_t i_majorNum);

  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Get the istep name
   *
   * @par Description:
   *     This routine gets the istep name stored at 
   *     the specified array index.
   *
   * @param[in] i_position      Array index
   * @param[out]o_istepName     Name of the istep
   *
   * @return false if the array index is out of range
   */
  bool getIStepNameOf(uint16_t i_position,std::string & o_istepName);
  
  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Get the array index
   *
   * @par Description:
   *     This routine gets the array index of the first minor
   *     number of the given major number.
   *
   * @param[in] i_majorNum    Major Number
   *
   * @return ECMD_INVALID_POSITION if the given major number is not valid
   */
  uint16_t getPosFirstMinorNumber(uint16_t i_majorNum);

  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Get the array index
   *
   * @par Description:
   *     This routine gets the array index of the last minor
   *     number of the given major number.
   *
   * @param[in] i_majorNum     Major Number
   *
   * @return ECMD_INVALID_POSITION if the given major number is not valid
   */
  uint16_t getPosLastMinorNumber(uint16_t i_majorNum);

  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Get the array index
   *
   * @par Description:
   *     This routine gets array index of 
   *     the specified istep name.
   *
   * @param[in] i_istepName    Name of the istep
   *
   * @return ECMD_INVALID_POSITION if the given step name is not valid
   */
  uint16_t getPosition(const std::string & i_istepName);

  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Get the istep number
   *
   * @par Description:
   *     This routine gets the istep number stored
   *     at the specified arry index
   *
   * @param[in] i_position    Array index
   *
   * @return ECMD_INVALID_ISTEP_NUM if the array index is out of range
   */
  uint16_t getIStepNumber(uint16_t i_position);

  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Get the istep Minor number
   *
   * @par Description:
   *     This routine gets the istep number stored
   *     at the specified arry index
   *
   * @param[in] i_position    Array index
   *
   * @return ECMD_INVALID_ISTEP_NUM if the array index is out of range
   */
  uint16_t getIStepMinorNumber(uint16_t i_position);
  
  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Get the destination
   *
   * @par Description:
   *     This routine tells who will perform the
   *     the specified istep number
   *
   * @param[in] i_majorNum    Major istep number
   * @param[in] i_minorNum    Minor istep number
   *
   * @return ECMD_ISTEP_INVALID_DESTINATION if the istep number is not valid
   */
  ecmdIPLTable::ecmdIStepDestination_t getDestination(  uint16_t i_majorNum,
                                                        uint16_t i_minorNum);

  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Get the istep name 
   *
   * @par Description:
   *     This routine gets the istep name for the specified istep
   *     major and minor number
   *
   * @param[in]   o_majorNum    Major istep number
   * @param[in]   o_minorNum    Minor istep number
   * @param[out]  i_istepName   Name of the istep
   *
   * @return false if the given major and minor number is invalid
   */

  bool getIStepName(const uint16_t i_majorNum,
                    const uint16_t i_minorNum,
                    std::string & o_istepName);

  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Get the destination
   *
   * @par Description:
   *     This routine tells who will perform the
   *     istep available at the position i_position
   *
   * @param[in] i_position    Index to the IPL Table
   *
   * @return ECMD_ISTEP_INVALID_DESTINATION if the position is invalid
   */

  ecmdIPLTable::ecmdIStepDestination_t 
                                getDestination(uint16_t i_position);
private:

    // Disabled copy constructor and assignment operator

    ecmdIPLTable(const ecmdIPLTable& right);

    ecmdIPLTable & operator=(const ecmdIPLTable& right);

    static const ecmdIStep_t cv_ecmdIStepTable[];

    // Number of isteps. 
    // This number represents the number of rows in the IPL table
    // The number of isteps in the IPL Table are calculated based
    // on the size of the table divided by the size of a row
    static const uint16_t ECMD_NUMBER_OF_ISTEPS;
};


#endif  // end of define for ecmdIStep_H

