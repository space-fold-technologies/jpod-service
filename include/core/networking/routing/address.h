#ifndef __JPOD__ADDRESS__
#define __JPOD__ADDRESS__

#include <net/if.h>
#include <net/pfvar.h>
#include <string>

namespace networking {
  enum class DynamicType {
    NETWORK = PFI_AFLAG_NETWORK,
    BROADCAST = PFI_AFLAG_BROADCAST,
    PEER = PFI_AFLAG_PEER,
    NO_ALIAS = PFI_AFLAG_NOALIAS
  };

  enum class AddressFamily {
    ANY = 0,
    IP_V4 = AF_INET,
    IP_V6 = AF_INET6
  };

  class Address {
    friend class AddressBuilder;

  private:
    Address(pf_addr_wrap address_wrap);
    ~Address();
    pf_addr_wrap _address_wrap;
  };

  class AddressBuilder {
  public:
    auto interface(const std::string &interface) -> AddressBuilder * {
      this->_interface = interface;
      return this;
    }

    auto dynamic_type(DynamicType type) -> AddressBuilder * {
      this->_type = type;
      return this;
    }
    auto table_name(const std::string &name) -> AddressBuilder * {
      this->_table_name = name;
      return this;
    }

    auto ip_address(const std::string &ip_address) -> AddressBuilder * {
      this->_ip_address = ip_address;
      return this;
    }

    auto build() -> Address {
      pf_addr_wrap _address_wrap;
      if (!_table_name.empty()) {
        std::memcpy(_address_wrap.v.tblname, _table_name.c_str(), PF_TABLE_NAME_SIZE);
        _address_wrap.type = PF_ADDR_TABLE;
      } else {
        _address_wrap.type = static_cast<u_int8_t>(_type);
      }
      if (!_interface.empty()) {
        std::memcpy(_address_wrap.v.ifname, _interface.data(), IFNAMSIZ);
      }
      // How to set up the address family and the address mask
      //_address_wrap.v.a.addr
      //_address_wrap.v.a.addr.pfa.addr32
      //_address_wrap.v.a.addr.pfa.addr16
      //_address_wrap.v.a.addr.pfa.addr8
      //_address_wrap.v.a.mask
      /*
       if ipv4 := ipn.IP.To4(); ipv4 != nil {
       		copy(a.wrap.v[0:4], ipv4)
       		copy(a.wrap.v[16:20], ipn.Mask)
       		a.af = C.AF_INET
       	} else {
       		copy(a.wrap.v[0:16], ipn.IP)
       		copy(a.wrap.v[16:32], ipn.Mask)
       		a.af = C.AF_INET6
       	}
       * */
      _address_wrap.type = PF_ADDR_ADDRMASK;
      return Address(_address_wrap);
    }

  private:
    std::string _interface;
    DynamicType _type;
    std::string _table_name;
    std::string _ip_address;
  };
} // namespace networking
#endif // __JPOD__ADDRESS__