#ifndef NETVARS_HPP
#define NETVARS_HPP
#include <map>
#include <string>
#include "client.hpp"

#define NETVAR(type, name, table, var) \
  type& name() { \
    static int offset = netvars::get_offset(table "->" var); \
    return *(type*)((uintptr_t)this + offset); \
  }


class RecvProp;
class RecvTable {
public:
  RecvProp* m_pProps; int m_nProps; void* m_pDecoder;
  char* m_pNetTableName; bool m_bInitialized; bool m_bInMainList;
};

class RecvProp {

public:
  char* m_pVarName; int m_RecvType; int m_Flags; int m_StringBufferSize;
  bool m_bInsideArray; const void* m_pExtraData; RecvProp* m_pArrayProp;
  void* m_ArrayLengthProxy; void* m_ProxyFn; void* m_DataTableProxyFn;
  RecvTable* m_pDataTable; int m_Offset; int m_ElementStride;
  int m_nElements; const char* m_pParentArrayPropName;
};

class ClientClass {
public:
  void* m_pCreateFn;
  void* m_pCreateEventFn;
  const char* m_pNetworkName;
  RecvTable* m_pRecvTable;
  ClientClass* m_pNext;
  int m_ClassID;
};

namespace netvars {
  inline std::map<std::string, int> offsets;
  void get_props_recursive(RecvTable* table);

  void init(Client* client_interface) {
    for (ClientClass* pClass = client_interface->get_all_classes(); pClass; pClass = pClass->m_pNext) {
      if (pClass->m_pRecvTable) { get_props_recursive(static_cast<RecvTable*>(pClass->m_pRecvTable));}
    }
  }

  void get_props_recursive(RecvTable* table) {
    for (int i = 0; i < table->m_nProps; ++i) {
      RecvProp* prop = &table->m_pProps[i];
      if (prop->m_pDataTable && prop->m_nElements > 0) get_props_recursive(prop->m_pDataTable);
      offsets[std::string(table->m_pNetTableName) + "->" + std::string(prop->m_pVarName)] = prop->m_Offset;
    }
  }

  int get_offset(const std::string& name) { return offsets[name]; }
}

#endif
