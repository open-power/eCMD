//IBM_PROLOG_BEGIN_TAG
/*
 * Copyright 2003,2016 IBM International Business Machines Corp.
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

#ifdef DOCUMENTATION
/**
 * @file ecmdPyApiTypes.H
 * @brief Definitions of the various eCMD Python API types
*/

/**
 @brief How to use a wrapped std::list in Python
 <hr>
 Some eCMD functions return data to Python in the form of a wrapped std::list.  This list can largely be manipulated using standard Python methods<br>

 Functions available on a wrapped Python std::list:
 <table>
 <tr><td>list.size()</td><td>Returns the number of entries in the list</td></tr>
 <tr><td>list.empty()</td><td>Returns True/False as to whether the list is empty</td></tr>
 <tr><td>list.clear()</td><td>Empty the list</td></tr>
 <tr><td>list.reverse()</td><td>Reverses the contents list</td></tr>
 <tr><td>list.resize(number)</td><td>Resizes the list the length specified.  If shrinking the list, entries are dropped from the front of the list.</td></tr>
 <tr><td>list.push_back(entry)</td><td>Pushes an entry onto the back of the list </td></tr>
 <tr><td>list.push_front(entry)</td><td>Pushes an entry onto the front of the list </td></tr>
 <tr><td>entry = list.pop()</td><td>Pop an entry from the back of the list and return it to entry</td></tr>
 <tr><td>list.pop_back()</td><td>Pop an entry from the back of the list.  It is removed from the list, but not returned for use.</td></tr>
 <tr><td>list.pop_front()</td><td>Pop an entry from the front of the list.  It is removed from the list, but not returned for use.</td></tr>
 </table>

 <br>

 Loading data into a list :
 <pre>
 tgts = ecmd.ecmdChipTargetList()
 tgt = ecmd.ecmdChipTarget()
 tgt.chipType="chip 1"
 tgts.push_back(tgt)
 tgt.chipType="chip 2"
 tgts.push_back(tgt)
 </pre>

 Displaying data on a list :
 <pre>
 for tgt in tgts:
     print("name: %s") % (tgt.chipType)
 </pre>
 Alternatively, you can loop by index:
 <pre>
 for i in range(0,tgts.size()):
     print("name: %s") % (tgts[i].chipType)
 </pre>

 You can directly modify an item in a list:
 <pre>
 tgts[1].chipType = "chip 3"
 </pre>

 You can remove an item from the back of list:
 <pre>
 tgt = tgts.pop()
 </pre>

 Or remove an item from the list using python del and giving the slice:
 <pre>
 del(tgts[0:1]) # Same as pop()
 </pre>

 In addition to push_back and push_front, you can also insert anywhere using standard python notation:
 <pre>
 tgt.chipType = "chip 4"
 tgts[1:1] = [tgt]
 </pre>

*/
typedef List List;

/**
 @brief How to use a wraped std::vector in Python
 <hr>
 Some eCMD functions return data to Python in the form of a wrapped std::vector.  This list can largely be manipulated using standard Python methods<br>

 Functions available on a wrapped Python std::vector:
 <table>
 <tr><td>list.size()</td><td>Returns the number of entries in the list</td></tr>
 <tr><td>list.empty()</td><td>Returns True/False as to whether the list is empty</td></tr>
 <tr><td>list.clear()</td><td>Empty the list</td></tr>
 <tr><td>list.reverse()</td><td>Reverses the contents list</td></tr>
 <tr><td>list.resize(number)</td><td>Resizes the list the length specified.  If shrinking the list, entries are dropped from the front of the list.</td></tr>
 <tr><td>list.push_back(entry)</td><td>Pushes an entry onto the back of the list </td></tr>
 <tr><td>entry = list.pop()</td><td>Pop an entry from the back of the list and return it to entry</td></tr>
 <tr><td>list.pop_back()</td><td>Pop an entry from the back of the list.  It is removed from the list, but not returned for use.</td></tr>
 </table>

 Usage of a vector is identical to a list as shown above.
*/
typedef Vector Vector;

/**
 @brief The python version of the ecmdThreadData C API std::list
 @see List
*/
typedef ecmdThreadDataList ecmdThreadDataList;
/**
 @brief The python version of the ecmdChipUnitData C API std::list
 @see List
*/
typedef ecmdChipUnitDataList ecmdChipUnitDataList;
/**
 @brief The python version of the ecmdChipData C API std::list
 @see List
*/
typedef ecmdChipDataList ecmdChipDataList;
/**
 @brief The python version of the ecmdSlotData C API std::list
 @see List
*/
typedef ecmdSlotDataList ecmdSlotDataList;
/**
 @brief The python version of the ecmdNodeData C API std::list
 @see List
*/
typedef ecmdNodeDataList ecmdNodeDataList;
/**
 @brief The python version of the ecmdCageData C API std::list
 @see List
*/
typedef ecmdCageDataList ecmdCageDataList;
/**
 @brief The python version of the string C API std::list
 @see List
*/
typedef stringList stringList;
/**
 @brief The python version of the string C API std::vector
 @see Vector
*/
typedef stringVector stringVector;
/**
 @brief The python version of the ecmdDataBufferBase Vector
 @see Vector
*/
typedef ecmdDataBufferBaseVector ecmdDataBufferBaseVector;
/**
 @brief The python version of the ecmdDataBuffer Vector
 @see Vector
*/
typedef ecmdDataBufferVector ecmdDataBufferVector;
/**
 @brief The python version of the ecmdMemoryEntry C API std::list
 @see List
*/
typedef ecmdMemoryEntryList ecmdMemoryEntryList;
/**
 @brief The python version of the ecmdRingData C API std::list
 @see List
*/
typedef ecmdRingDataList ecmdRingDataList;
/**
 @brief The python version of the ecmdLatchData C API std::list
 @see List
*/
typedef ecmdLatchDataList ecmdLatchDataList;
/**
 @brief The python version of the ecmdLatchEntry C API std::list
 @see List
*/
typedef ecmdLatchEntryList ecmdLatchEntryList;
/**
 @brief The python version of the ecmdSpyGroupData C API std::list
 @see List
*/
typedef ecmdSpyGroupDataList ecmdSpyGroupDataList;
/**
 @brief The python version of the ecmdArrayEntry C API std::list
 @see List
*/
typedef ecmdArrayEntryList ecmdArrayEntryList;
/**
 @brief The python version of the ecmdNameEntry C API std::list
 @see List
*/
typedef ecmdNameEntryList ecmdNameEntryList;
/**
 @brief The python version of the ecmdNameVectorEntry C API std::list
 @see List
*/
typedef ecmdNameVectorEntryList ecmdNameVectorEntryList;
/**
 @brief The python version of the ecmdIndexEntry C API std::list
 @see List
*/
typedef ecmdIndexEntryList ecmdIndexEntryList;
/**
 @brief The python version of the ecmdChipTarget C API std::list
 @see List
*/
typedef ecmdChipTargetList ecmdChipTargetList;
/**
 @brief The python version of the ecmdArrayData C API std::list
 @see List
*/
typedef ecmdArrayDataList ecmdArrayDataList;
/**
 @brief The python version of the ecmdTraceArrayData C API std::list
 @see List
*/
typedef ecmdTraceArrayDataList ecmdTraceArrayDataList;
/**
 @brief The python version of the ecmdSpyData C API std::list
 @see List
*/
typedef ecmdSpyDataList ecmdSpyDataList;
/**
 @brief The python version of the ecmdSpyLatchData C API std::list
 @see List
*/
typedef ecmdSpyLatchDataList ecmdSpyLatchDataList;
/**
 @brief The python version of the ecmdScomData C API std::list
 @see List
*/
typedef ecmdScomDataList ecmdScomDataList;
/**
 @brief The python version of the ecmdI2CCmdEntry C API std::list
 @see List
*/
typedef ecmdI2CCmdEntryList ecmdI2CCmdEntryList;
/**
 @brief The python version of the ecmdConnectionData C API std::list
 @see List
*/
typedef ecmdConnectionDataList ecmdConnectionDataList;
/**
 @brief The python version of the ecmdScomEntry C API std::list
 @see List
*/
typedef ecmdScomEntryList ecmdScomEntryList;
/**
 @brief The python version of the ecmdFileLocation C API std::list
 @see List
*/
typedef ecmdFileLocationList ecmdFileLocationList;
/**
 @brief The python version of the uint32_t C API std::list
 @see List
*/
typedef uint32_tList uint32_tList;

typedef Map Map;
/**
 @brief The python version of the std::map<std::string, ecmdDataBuffer>
*/
typedef string_ecmdDataBufferMap string_ecmdDataBufferMap;
#endif
